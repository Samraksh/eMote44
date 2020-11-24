
/*
This is as straight-forward a rip of the CubeMX USB CDC Device example code as I could make.
Some minor edits for porting.

This does NOT implement the true NETMF USB PAL, only to get usb-serial working for basic comms
My hunch is that the delta to get the real PAL working isn't much, if you know what you are doing.

NPS 2019-11-22
*/

#include <tinyhal.h>
#include <STM32H7_Time/lptim.h>
#include <Samraksh/serial_frame_pal.h>
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usb_serial_ext.h"

#define USB_BUSY_RETRY_INTERVAL_MS 1
#define TX_BUF_SIZE (sizeof(tx_pkt_buf))

#define ALWAYS_QUEUE_FROM_IRQ

// If we are a BASE, leave USB up. If a FENCE, kill it.
#ifndef MKII_BASE_CONFIG
#define KILL_USB
#endif

// Generic Port Stuff

static HAL_CONTINUATION rx_event_contin; // For signaling to CLR that COM2 data is available
uint8_t usb_com2_rx_buf[512] __attribute__ (( section (".ram_d2"), aligned(32) ));
uint32_t usb_com2_rx_buf_sz;

void add_bytes_to_com2(uint8_t *data, unsigned len) {
	if(len + usb_com2_rx_buf_sz > sizeof(usb_com2_rx_buf)) return;
	memcpy(&usb_com2_rx_buf[usb_com2_rx_buf_sz], data, len);
	usb_com2_rx_buf_sz += len;
	rx_event_contin.Enqueue();
}

static inline int usb_serial_generic_WRITE(void* pInstance, const char* Data, size_t size) {
#ifdef MEL_USE_SERIAL_FRAMES
	return send_framed_serial((const uint8_t *)Data, size, TRUE);
#else
	return CPU_USB_write(Data, size);
#endif
}

static inline int usb_serial_generic_WRITE_DATA(void* pInstance, const char* Data, size_t size) {
#ifdef MEL_USE_SERIAL_FRAMES
	return send_framed_serial((const uint8_t *)Data, size, FALSE);
#else
	return CPU_USB_write(Data, size);
#endif
}

static inline BOOL usb_serial_generic_INIT(void* pInstance) {
	return CPU_USB_Initialize(0);
}

static inline int usb_serial_generic_READ_DATA(void* pInstance, char* Data, size_t size) {
	int count;
	if (size == 0) return usb_com2_rx_buf_sz; // Returns available bytes
	GLOBAL_LOCK(irq);
	if (size > usb_com2_rx_buf_sz) count = usb_com2_rx_buf_sz;
	else count = size;
	memcpy(Data, usb_com2_rx_buf, count);
	usb_com2_rx_buf_sz -= count;
	return count;
}

static inline int usb_serial_generic_READ(void* pInstance, char* Data, size_t size) {
#if defined(ALLOW_BKPT) && defined(DEBUG)
	if (Data == NULL && size > 0) __BKPT(); // null pointer check
#endif
	return read_serial_frame_buffer((uint8_t *)Data, size);
}

// WRITES DEBUG FRAMES
static IGenericPort const mel_usb_serial_gport =
{
    // default returns TRUE
    usb_serial_generic_INIT, //BOOL (*Initialize)( void* pInstance );

    // default returns TRUE
    NULL, //BOOL (*Uninitialize)( void* pInstance );

    // default return 0
    usb_serial_generic_WRITE, //int (*Write)( void* pInstance, const char* Data, size_t size );

    // defualt return 0
    usb_serial_generic_READ, //int (*Read)( void* pInstance, char* Data, size_t size );

    // default return TRUE
    NULL, //BOOL (*Flush)( void* pInstance );

    // default do nothing
    NULL, //void (*ProtectPins)( void* pInstance, BOOL On );

    // default return FALSE
    NULL, //BOOL (*IsSslSupported)( void* pInstance );

    // default return FALSE
    NULL, //BOOL (*UpgradeToSsl)( void* pInstance, const UINT8* pCACert, UINT32 caCertLen, const UINT8* pDeviceCert, UINT32 deviceCertLen, LPCSTR szTargetHost );

    // default return FALSE
    NULL, //BOOL (*IsUsingSsl)( void* pInstance );
};

extern const GenericPortTableEntry mel_usb_generic_port =
{
    mel_usb_serial_gport,
    NULL
};

// WRITES DATA FRAMES
static IGenericPort const mel_usb_serial_data_gport =
{
    // default returns TRUE
    usb_serial_generic_INIT, //BOOL (*Initialize)( void* pInstance );

    // default returns TRUE
    NULL, //BOOL (*Uninitialize)( void* pInstance );

    // default return 0
    usb_serial_generic_WRITE_DATA, //int (*Write)( void* pInstance, const char* Data, size_t size );

    // defualt return 0
    usb_serial_generic_READ_DATA, //int (*Read)( void* pInstance, char* Data, size_t size );

    // default return TRUE
    NULL, //BOOL (*Flush)( void* pInstance );

    // default do nothing
    NULL, //void (*ProtectPins)( void* pInstance, BOOL On );

    // default return FALSE
    NULL, //BOOL (*IsSslSupported)( void* pInstance );

    // default return FALSE
    NULL, //BOOL (*UpgradeToSsl)( void* pInstance, const UINT8* pCACert, UINT32 caCertLen, const UINT8* pDeviceCert, UINT32 deviceCertLen, LPCSTR szTargetHost );

    // default return FALSE
    NULL, //BOOL (*IsUsingSsl)( void* pInstance );
};

extern const GenericPortTableEntry mel_usb_generic_data_port =
{
    mel_usb_serial_data_gport,
    NULL
};

// TOTAL_GENERIC_PORTS is from platform_selector.h and should be 2
#if (TOTAL_GENERIC_PORTS != 2)
	#error "TOTAL_GENERIC_PORTS wrong???"
#endif
extern GenericPortTableEntry const* const g_GenericPorts[TOTAL_GENERIC_PORTS] = { &mel_usb_generic_port, &mel_usb_generic_data_port };
// End Generic Port Stuff

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
	usb_lock_mem  =2,
	usb_lock_irq  =3,
} usb_lock_id_t;

extern USBD_HandleTypeDef hUsbDeviceFS; // in usb_device.c
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

static bool USB_initialized = false;
static unsigned last_malloc_size;
static uint8_t rx_pkt_buf[64]; 			// TODO: Assumes packets are handled fast, and should be size 64

static uint8_t tx_pkt_buf[32*1024] __attribute__ (( section (".ram_d2"), aligned(32) )); // MAX SIZE 32 kByte... if you need memory, reduce this first
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
	rx_framed_serial(Buf, *Len);
	usb_cdc_status.RxBytes += *Len;
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, rx_pkt_buf);
	USBD_CDC_ReceivePacket(&hUsbDeviceFS);
	//rx_event_contin.Enqueue();
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

// Yes this is lazy and ugly but I could clean this up forever...
extern void usb_serial_signal_rx_to_clr(void *p); // in Samraksh_Mel_Samraksh_Mel_UsbSerialInterface.cpp

static void reset_status(usb_cdc_status_t *x) {
	memset(&usb_cdc_status, 0, sizeof(usb_cdc_status_t)); // A simple zero'ing
	lptim_task_init(&usb_retry_task);
	usb_retry_task.contin = (void *)&usb_retry_contin;
	usb_retry_task.delay_ms = USB_BUSY_RETRY_INTERVAL_MS;
	usb_retry_contin.InitializeCallback(do_usb_retry, NULL);
	rx_event_contin.InitializeCallback(usb_serial_signal_rx_to_clr, NULL);
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

static unsigned get_tx_buf_used(void) {
	return usb_cdc_status.TxCurrent + usb_cdc_status.TxBytesQueued;
}

#ifdef KILL_USB
static bool do_not_reinit=true; // hack fix me. I don't think needed but just to be sure...
#else
static bool do_not_reinit=false; // hack fix me. I don't think needed but just to be sure...
#endif
HRESULT CPU_USB_Initialize( int Controller ) {
	if (do_not_reinit) return S_OK;
	if (USB_initialized == false){
		MX_USB_DEVICE_Init();
		// Allows USB to work in WFI
		__HAL_RCC_USB2_OTG_FS_CLK_SLEEP_ENABLE();
		__HAL_RCC_USB2_OTG_FS_ULPI_CLK_SLEEP_DISABLE();

		USB_initialized = true;
		usb_com2_rx_buf_sz = 0;
		reset_status(&usb_cdc_status);
#if defined(PAUSE_AFTER_USB_INIT_MS) && PAUSE_AFTER_USB_INIT_MS > 0
		HAL_Delay(PAUSE_AFTER_USB_INIT_MS);
#endif
	}
	return S_OK;
}

// No-op
HRESULT CPU_USB_Uninitialize( int Controller ) {
	uint32_t lock_ret;

	if (USB_initialized == false) return S_OK;

	USB_initialized = false;
	do_not_reinit = true;
	MX_USB_DEVICE_DeInit();

	// We should be IRQ locked so this is kosher
	if (usb_lock != usb_lock_mem)
		usb_lock = 0;

	return S_OK;
}

// Locks USB and returns a pointer directly to buffer for writing
// Must be followed (quickly) by a closing usb_serial_ext_free()
void * usb_serial_ext_malloc(size_t sz) {
	uint32_t lock_ret;
	unsigned used_buf;

	lock_ret = get_lock(&usb_lock, usb_lock_mem);
	if (lock_ret) { usb_cdc_status.lock_fails++; return NULL; } // Failed to get lock, abort

	// Reset buffer high-water mark if empty
	if (!is_usb_tx_not_empty() && usb_cdc_status.TxBytesQueued == 0) {
		usb_cdc_status.TxCurrent = 0;
	}

	used_buf = get_tx_buf_used();
	if (sz > TX_BUF_SIZE - used_buf) {
		return NULL; // Not enough buffer space
	}

	last_malloc_size = sz;

	// USB LOCK IS STILL HELD
	return &tx_pkt_buf[used_buf];
}

// Frees the borrowed memory
// if 'size' is non-zero, will transmit 'size' bytes
int usb_serial_ext_free(unsigned size) {
	// Invalid state
	if (usb_lock != usb_lock_mem) {
		return -1;
	}

	// Trying to TX more bytes than borrowed...
	if (last_malloc_size < size) {
		return -1;
	}

	// Put the extra bytes in the tx queue
	usb_cdc_status.TxBytesQueued += size;
	last_malloc_size = 0;
	free_lock(&usb_lock);

	// Start the send now if not IRQ, else queue it
	if (size && !isInterrupt() ) {
		do_usb_retry(NULL);
	}
	else if (size && isInterrupt() ) {
		usb_retry_contin.Enqueue();
	}

	return 0;
}

// Continuation Callback, EXCEPT from usb_serial_ext_free()
static void do_usb_retry(void *p) {
	int usb_ret;
	uint32_t lock_ret;

	if (!USB_initialized) return;

	lock_ret = get_lock(&usb_lock, usb_lock_tx);
	if (lock_ret) { // Locked out. Wait one schedule cycle and try again.
		usb_cdc_status.lock_fails++;
		usb_retry_contin.Enqueue();
		return;
	}

	if (usb_cdc_status.TxBytesQueued == 0) goto out; // No work to do

	if (is_usb_tx_not_empty()) {
		lptim_add_oneshot(&usb_retry_task); // Retry later
		goto out;
	}

	// Throw buffer away if the link went down
	if (!is_usb_link_up()) {
		goto out_reset;
	}

	// We are free to send
	usb_ret = CDC_Transmit_FS(&tx_pkt_buf[usb_cdc_status.TxCurrent], usb_cdc_status.TxBytesQueued);
	if (usb_ret != USBD_OK) { goto out_reset; }

out_reset:
	usb_cdc_status.TxCurrent += usb_cdc_status.TxBytesQueued;
	usb_cdc_status.TxBytes   += usb_cdc_status.TxBytesQueued;
	usb_cdc_status.TxBytesQueued = 0;

out:
	free_lock(&usb_lock);
	return;
}

static int usb_queue_retry(const char *buf, int size) {
	uint32_t lock_ret;
	int ret;
	unsigned used_buf;

	lock_ret = get_lock(&usb_lock, usb_lock_tx);
	if (lock_ret) { usb_cdc_status.lock_fails++; return 0; } // Failed to get lock, busy

	// Check buffer space
	used_buf = get_tx_buf_used();
	if (size > TX_BUF_SIZE - used_buf) {
		ret = 0; goto out;
	}

	// Add data to buffer and mark bytes as queued
	memcpy(&tx_pkt_buf[used_buf], buf, size);
	usb_cdc_status.TxBytesQueued += size;

	// Attempt to send after interval (typ 1ms)
	lptim_add_oneshot(&usb_retry_task);
	ret = size;

out:
	free_lock(&usb_lock);
	//return ret; // TEMP
	return size; // Never fail
}

int CPU_USB_write(const char *buf, int size) {
	int ret, usb_ret;
	uint32_t lock_ret;
	bool busy_retry = false;

	if (buf == NULL)						return -1;
	if (size == 0)							return 0;		// trivial case
	if (!USB_initialized || !is_usb_link_up())	return size;	// Init but no connection we define as "success"
#ifdef ALWAYS_QUEUE_FROM_IRQ
	if (isInterrupt())						return usb_queue_retry(buf, size); // Always queue request if in interrupt context
#endif
	if (is_usb_tx_not_empty()) 				return usb_queue_retry(buf, size);
	if (is_usb_tx_queued())					return usb_queue_retry(buf, size); // Could service immediately, but throw it on the buffer

	lock_ret = get_lock(&usb_lock, usb_lock_tx);
	if (lock_ret) { usb_cdc_status.lock_fails++; return 0; }// Failed to get lock, busy

	// USB lock acquired

	// Check if busy again after lock acquired
	if (is_usb_tx_not_empty()) { busy_retry = true; goto out; }

	if (size > TX_BUF_SIZE) size = TX_BUF_SIZE;
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
	if (busy_retry)
		return usb_queue_retry(buf, size);
	else
		return ret;
}

extern "C" void OTG_FS_IRQHandler(void) {
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

extern "C" void USB_Error_Handler(void);
void USB_Error_Handler(void) {
#ifdef DEBUG
	//__BKPT();
#endif
}
