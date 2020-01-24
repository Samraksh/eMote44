
/*
This is as straight-forward a rip of the CubeMX USB CDC Device example code as I could make.
Some minor edits for porting.

This does NOT implement the true NETMF USB PAL, only to get usb-serial working for basic comms
My hunch is that the delta to get the real PAL working isn't much, if you know what you are doing.

NPS 2019-11-22
*/

#include <tinyhal.h>
#include "usb_device.h"

#ifndef STRING_BUF_SIZE
#define STRING_BUF_SIZE 128
#endif

// Note buffers are large to accomodate testing

#ifndef USB_IN_BUF_SIZE
#define USB_IN_BUF_SIZE (256)
#endif

#define INBUF_IDX usb_cdc_status.RxQueueBytes // Alias, to confuse people later

static bool USB_initialized = FALSE;

static uint8_t inbuf[USB_IN_BUF_SIZE];		// USB buffer
											// Using libc for outbuf
static uint8_t rx_pkt_buf[STRING_BUF_SIZE]; // Max FS packet size is 64 bytes
static uint8_t tx_pkt_buf[STRING_BUF_SIZE]; // Max FS packet size is 64 bytes

static usb_cdc_status_t usb_cdc_status;

static void reset_status(usb_cdc_status_t *x) {
	memset(x, 0, sizeof(usb_cdc_status_t)); // A simple zero'ing
}

static int is_usb_link_up(void) {
	return usb_cdc_status.is_connected;
}

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

extern "C" void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

extern "C" uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len); // for debugging


HRESULT CPU_USB_Initialize( int Controller )
{
	if (USB_initialized == false){
		MX_USB_DEVICE_Init();
		USB_initialized = true;
	}
	return S_OK;
}

HRESULT CPU_USB_Uninitialize( int Controller )
{
	USB_initialized = false;
	return S_OK;
}

// Kind of sloppy to be locked for most of it, but it is clean and not worth further investment
/*int CPU_USB_read(void *buf, int size) {
	int ret=0;
	// Don't care about CDC not connected case, simply means nothing to read
	if (INBUF_IDX == 0) return ret;
	__disable_irq();
	if (size >= INBUF_IDX) { // Return the whole input buffer
		memcpy(buf, inbuf, INBUF_IDX);
		ret = INBUF_IDX;
		INBUF_IDX = 0;
	}
	else { // Return a piece of the input buffer, shift reminder down
		memcpy(buf, inbuf, size);
		memcpy(inbuf, &inbuf[size], sizeof(inbuf)-size); // Does not actually overlap so no need memmove()
		INBUF_IDX -= size;
		ret = size;
	}
	usb_cdc_status.RxBytes += ret;
	__enable_irq();
	return ret;
}
*/
int CPU_USB_write(const char *buf, int size) {
	int ret;
	//if (!is_usb_link_up()) return -1; // CDC not connected
	memcpy(tx_pkt_buf, buf, size);
	do { // TODO: ADD TIMEOUT
		ret = CDC_Transmit_FS(tx_pkt_buf, size);
	} while(ret == USBD_BUSY);
	if (ret != USBD_OK) __BKPT();
	__disable_irq();
	usb_cdc_status.TxBytes += size;
	__enable_irq();
	return size;
}

// This function is called from the usb driver c code
extern "C" int CPU_USB_Queue_Rx_Data(  char c );
int CPU_USB_Queue_Rx_Data(  char c ){
	return USART_AddCharToRxBuffer(USB_SERIAL_PORT, c);
}

extern "C" void USB_Error_Handler(void);
void USB_Error_Handler(void) {
#ifdef DEBUG
	__BKPT();
#endif
}
