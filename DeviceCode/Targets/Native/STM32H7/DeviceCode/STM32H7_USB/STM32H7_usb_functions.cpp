
/*
This is as straight-forward a rip of the CubeMX USB CDC Device example code as I could make.
Some minor edits for porting.

This does NOT implement the true NETMF USB PAL, only to get usb-serial working for basic comms
My hunch is that the delta to get the real PAL working isn't much, if you know what you are doing.

NPS 2019-11-22
*/

#include <tinyhal.h>
#include "usb_device.h"

volatile int test_delete_me = 0;

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

extern "C" void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

extern "C" uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len); // for debugging

void test_usb(void) {
	USBD_StatusTypeDef ret;
	char s[] = "Hello Nathan!\r\n";
	do {
	ret = (USBD_StatusTypeDef)CDC_Transmit_FS((uint8_t *)s, hal_strlen_s(s));
	} while (ret != USBD_OK);
	__NOP();
}


HRESULT CPU_USB_Initialize( int Controller )
{
	MX_USB_DEVICE_Init();
	if (test_delete_me) test_usb();
	return S_OK;
}

HRESULT CPU_USB_Uninitialize( int Controller )
{
	return S_OK;
}

extern "C" void USB_Error_Handler(void);
void USB_Error_Handler(void) {
#ifdef DEBUG
	__BKPT();
#endif
}
