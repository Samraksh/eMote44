//-----------------------------------------------------------------------------
//
//                   ** WARNING! ** 
//    This file was generated automatically by a tool.
//    Re-running the tool will overwrite this file.
//    You should copy this file to a custom location
//    before adding any customization in the copy to
//    prevent loss of your changes when the tool is
//    re-run.
//
//-----------------------------------------------------------------------------

#include <tinyhal.h>
#include "Samraksh_Mel.h"
#include "Samraksh_Mel_Samraksh_Mel_UsbSerialInterface.h"

extern CLR_RT_HeapBlock_NativeEventDispatcher *USB_ne_Context;

using namespace Samraksh_Mel;

#define COM1_PORT_NUM 0

// Signals up to CLR that data is ready
// Unused pointer arg is to make compatible with HAL_CONTINUATION
void usb_serial_signal_rx_to_clr(void *p)
{
	GLOBAL_LOCK(irq);
	if (USB_ne_Context != NULL) // Will be NULL until init by C#
		SaveNativeEventToHALQueue( USB_ne_Context, 0, 0 );
}

UINT32 UsbSerialInterface::BytesInBuffer( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
	return GenericPort_Read(COM1_PORT_NUM, NULL, 0);
}

INT32 UsbSerialInterface::mel_serial_tx( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, UINT32 param1, INT32 param2, HRESULT &hr )
{
	const char *data = (const char *)param0.GetBuffer();
	//uint32_t channel; // NYI
	size_t size = (size_t)param2;
	GenericPort_Write(COM1_PORT_NUM, data, size);
	return 0;
}

INT32 UsbSerialInterface::mel_serial_rx( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, UINT32 param1, HRESULT &hr )
{
	char *data = (char *)param0.GetBuffer();
	size_t size = (size_t)param1;
	return GenericPort_Read(COM1_PORT_NUM, data, size);
}

