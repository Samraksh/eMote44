
/*
This is as straight-forward a rip of the CubeMX USB CDC Device example code as I could make.
Some minor edits for porting.

This does NOT implement the true NETMF USB PAL, only to get usb-serial working for basic comms
My hunch is that the delta to get the real PAL working isn't much, if you know what you are doing.

NPS 2019-11-22
*/

#include <tinyhal.h>
#include "usb_device.h"
#include "usbd_cdc_if.h"


//#define INBUF_IDX usb_cdc_status.RxQueueBytes // Alias, to confuse people later

// Singleton status structure
typedef struct {
	volatile bool is_connected;
//	int usb_cdc_overrun;
	unsigned TxBytes;
	unsigned RxBytes;
//	unsigned RxQueueBytes;
} usb_cdc_status_t;

typedef enum {
	usb_lock_none =0,
	usb_lock_tx	  =1,
} usb_lock_id_t;

extern USBD_HandleTypeDef hUsbDeviceFS; // in usb_device.c
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

static bool USB_initialized = FALSE;
static uint8_t rx_pkt_buf[64]; 				// TODO: Assumes packets are handled fast
static uint8_t tx_pkt_buf[256]; 			// 4 packets
static usb_cdc_status_t usb_cdc_status;

static uint32_t usb_lock; // For locking TX

// Core USB CDC callbacks
static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

static int CPU_USB_Queue_Rx_Data(char c);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
	CDC_Init_FS,
	CDC_DeInit_FS,
	CDC_Control_FS,
	CDC_Receive_FS
};

// Test if in interrupt context
static inline bool isInterrupt(void) __attribute__ ((unused));
static inline bool isInterrupt() {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

// Returns true if lock aquired
static bool get_lock_inner(volatile uint32_t *Lock_Variable, usb_lock_id_t id) {
	uint32_t status;
	__DSB();

	if (__LDREXW(Lock_Variable) != usb_lock_none) {
		__CLREX(); // AM I NEEDED?
		return false;
	}

	status = __STREXW(id, Lock_Variable);
	return (status == usb_lock_none);
}

// Should try more than once, can legit fail even if lock is free.
// Example, will never succeed if any interrupt hits in between.
// ldrex-strex only guarantees that lock is free when it says so, but NOT the inverse.
static uint32_t get_lock(volatile uint32_t *Lock_Variable, usb_lock_id_t id) {
	int attempts=3;
	do {
		if ( get_lock_inner(Lock_Variable, id) )
			return 0;
	} while (--attempts);
	return *Lock_Variable; // return who we think the blocking owner is. NOT GUARANTEED TO BE RIGHT.
}

static void free_lock(volatile uint32_t *Lock_Variable) {
	__DMB();
	*Lock_Variable = usb_lock_none;
	return;
}

static int8_t CDC_Init_FS(void) {
	usb_cdc_status.is_connected = true;
	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, tx_pkt_buf, 0);
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, rx_pkt_buf);
	return (USBD_OK);
}

static int8_t CDC_DeInit_FS(void) {
	usb_cdc_status.is_connected = false;
	return (USBD_OK);
}

static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length) {
	return (USBD_OK);
}

static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len) {
	for (int i = 0; i < *Len; i++) {
	  CPU_USB_Queue_Rx_Data( (char)Buf[i]); // TODO: Can input more than 1 byte at a time
	}
	usb_cdc_status.RxBytes += *Len;
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, rx_pkt_buf);
	USBD_CDC_ReceivePacket(&hUsbDeviceFS);
	return (USBD_OK);
}

uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len) {
  uint8_t result = USBD_OK;
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  return result;
}

static void reset_status(usb_cdc_status_t *x) {
	memset(x, 0, sizeof(usb_cdc_status_t)); // A simple zero'ing
}

static int is_usb_link_up(void) {
	return usb_cdc_status.is_connected;
}

// On going operation
static int is_usb_tx_not_empty(void) {
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState != 0) return 1;
	else return 0; // empty
}

HRESULT CPU_USB_Initialize( int Controller ) {
	if (USB_initialized == false){
		MX_USB_DEVICE_Init();
		// Allows USB to work in WFI
		__HAL_RCC_USB2_OTG_FS_CLK_SLEEP_ENABLE();
		__HAL_RCC_USB2_OTG_FS_ULPI_CLK_SLEEP_DISABLE();
		USB_initialized = true;
	}
	return S_OK;
}

// No-op
HRESULT CPU_USB_Uninitialize( int Controller ) {
	return S_OK;
}

int CPU_USB_write(const char *buf, int size) {
	int ret, lock_ret, usb_ret;
	const int max_sz = sizeof(tx_pkt_buf);

	if (!USB_initialized)      return -1;	// Never init, hard fail
	if (!is_usb_link_up())     return size;	// Init but no connection we define as "success"
	if (is_usb_tx_not_empty()) return 0;	// Busy

	lock_ret = get_lock(&usb_lock, usb_lock_tx);
	if (lock_ret) return 0; // Failed to get lock, busy

	// Check if busy again after lock acquired
	if (is_usb_tx_not_empty()) { ret = 0; goto out; }

	if (size > max_sz) size = max_sz;
	memcpy(tx_pkt_buf, buf, size);

	usb_ret = CDC_Transmit_FS(tx_pkt_buf, size);

	// Oddball error check
	if (usb_ret != USBD_OK) { ret = -1; goto out; }

	// Adjust accounting, we are locked so IRQ safe
	ret = size;
	usb_cdc_status.TxBytes += size;

out:
	free_lock(&usb_lock);
	return ret;
}

static int CPU_USB_Queue_Rx_Data( char c ){
	// sending back only port number (USB_SERIAL_PORT also contains info that it is a serial interface)
	return USART_AddCharToRxBuffer(ConvertCOM_ComPort(USB_SERIAL_PORT), c);
}

extern "C" void OTG_FS_IRQHandler(void) {
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

extern "C" void USB_Error_Handler(void);
void USB_Error_Handler(void) {
#ifdef DEBUG
	__BKPT();
#endif
}
