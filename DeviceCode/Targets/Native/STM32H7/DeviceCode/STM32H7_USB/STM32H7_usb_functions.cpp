
/*
This is as straight-forward a rip of the CubeMX USB CDC Device example code as I could make.
Some minor edits for porting.

This does NOT implement the true NETMF USB PAL, only to get usb-serial working for basic comms
My hunch is that the delta to get the real PAL working isn't much, if you know what you are doing.

NPS 2019-11-22
*/

#include <tinyhal.h>
#include <STM32H7_Time/lptim.h>
#include "usb_device.h"
#include "usbd_cdc_if.h"

#define USB_BUSY_RETRY_INTERVAL_MS 10

//#define INBUF_IDX usb_cdc_status.RxQueueBytes // Alias, to confuse people later

// Singleton status structure
typedef struct {
	volatile bool is_connected;
//	int usb_cdc_overrun;
	unsigned lock_fails;
	unsigned TxBytes;
	unsigned TxBytesQueued;
	unsigned TxCurrent; // Bytes in on-going transmission
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
static uint8_t tx_pkt_buf[1024];
static usb_cdc_status_t usb_cdc_status;

static HAL_CONTINUATION usb_retry_contin;
static lptim_task_t usb_retry_task;			// After a request is buffered, try again later (e.g., 25ms)
static void do_usb_retry(void *p);			// The continuation function

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
	if (__LDREXW(Lock_Variable) != usb_lock_none) {
		__CLREX(); // AM I NEEDED?
		return false;
	}
	status = __STREXW(id, Lock_Variable);
	return (status == 0);
}

// Should try more than once, can legit fail even if lock is free.
// Example, will never succeed if any interrupt hits in between.
// ldrex-strex only guarantees that lock is free when it says so, but NOT the inverse.
static uint32_t get_lock(volatile uint32_t *Lock_Variable, usb_lock_id_t id) {
	int attempts=3;
	do {
		if ( get_lock_inner(Lock_Variable, id) )
		{ __DMB(); return 0; }
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
	memset(&usb_cdc_status, 0, sizeof(usb_cdc_status_t)); // A simple zero'ing
	lptim_task_init(&usb_retry_task);
	usb_retry_task.contin = (void *)&usb_retry_contin;
	usb_retry_task.delay_ms = USB_BUSY_RETRY_INTERVAL_MS;
	usb_retry_contin.InitializeCallback(do_usb_retry, NULL);
}

static int is_usb_link_up(void) {
	return usb_cdc_status.is_connected;
}

// On going operation
static bool is_usb_tx_not_empty(void) {
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState != 0) return true;
	else return false; // empty
}

static bool is_usb_tx_queued(void) {
	return (usb_cdc_status.TxBytesQueued > 0);
}

HRESULT CPU_USB_Initialize( int Controller ) {
	if (USB_initialized == false){
		MX_USB_DEVICE_Init();
		// Allows USB to work in WFI
		__HAL_RCC_USB2_OTG_FS_CLK_SLEEP_ENABLE();
		__HAL_RCC_USB2_OTG_FS_ULPI_CLK_SLEEP_DISABLE();

		USB_initialized = true;
		reset_status(&usb_cdc_status);
	}
	return S_OK;
}

// No-op
HRESULT CPU_USB_Uninitialize( int Controller ) {
	return S_OK;
}

// Continuation Callback
// Meaning this is fully synchronous and guaranteed not an ISR
static void do_usb_retry(void *p) {
	int lock_ret, usb_ret;

	lock_ret = get_lock(&usb_lock, usb_lock_tx);
	if (lock_ret) { // Locked out. Wait one schedule cycle and try again.
		usb_cdc_status.lock_fails++;
		usb_retry_contin.Enqueue();
		return;
	}

	if (usb_cdc_status.TxBytesQueued == 0) goto out; // No work to do

	if (is_usb_tx_not_empty()) {
		lptim_add_oneshot(&usb_retry_task); // Wait another 10ms
		goto out;
	}

	// We are free to send
	usb_ret = CDC_Transmit_FS(&tx_pkt_buf[usb_cdc_status.TxCurrent], usb_cdc_status.TxBytesQueued);
	if (usb_ret != USBD_OK) { __BKPT(); goto out_reset; }

out_reset:
	usb_cdc_status.TxCurrent += usb_cdc_status.TxBytesQueued;
	usb_cdc_status.TxBytes   += usb_cdc_status.TxBytesQueued;
	usb_cdc_status.TxBytesQueued = 0;

out:
	free_lock(&usb_lock);
	return;
}

static int usb_queue_retry(const char *buf, int size) {
	int lock_ret;
	int ret;
	const int max_sz = sizeof(tx_pkt_buf);

	lock_ret = get_lock(&usb_lock, usb_lock_tx);
	if (lock_ret) { usb_cdc_status.lock_fails++; return 0; } // Failed to get lock, busy

	// Check buffer space
	if (size > max_sz - usb_cdc_status.TxCurrent - usb_cdc_status.TxBytesQueued) {
		ret = 0; goto out;
	}

	// Add data to buffer and mark bytes as queued
	memcpy(&tx_pkt_buf[usb_cdc_status.TxCurrent + usb_cdc_status.TxBytesQueued], buf, size);
	usb_cdc_status.TxBytesQueued += size;

	// Setup the 10ms timeout
	lptim_add_oneshot(&usb_retry_task);
	ret = size;

out:
	free_lock(&usb_lock);
	return ret; // TEMP
}

int CPU_USB_write(const char *buf, int size) {
	int ret, lock_ret, usb_ret;
	const int max_sz = sizeof(tx_pkt_buf);

	if (!USB_initialized || buf == NULL)	return -1;		// Hard fails
	if (size == 0)							return 0;		// trivial case
	if (!is_usb_link_up())     				return size;	// Init but no connection we define as "success"
	if (is_usb_tx_not_empty()) 				return usb_queue_retry(buf, size);
	if (is_usb_tx_queued())					return usb_queue_retry(buf, size); // Could service immediately, but throw it on the buffer

	lock_ret = get_lock(&usb_lock, usb_lock_tx);
	if (lock_ret) { usb_cdc_status.lock_fails++; return 0; }// Failed to get lock, busy

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
	usb_cdc_status.TxCurrent = size;

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
