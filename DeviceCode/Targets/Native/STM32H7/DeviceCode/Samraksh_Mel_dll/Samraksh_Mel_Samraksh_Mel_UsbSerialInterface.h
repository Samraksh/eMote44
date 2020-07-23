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


#ifndef _SAMRAKSH_MEL_SAMRAKSH_MEL_USBSERIALINTERFACE_H_
#define _SAMRAKSH_MEL_SAMRAKSH_MEL_USBSERIALINTERFACE_H_

namespace Samraksh_Mel
{
    struct UsbSerialInterface
    {
        // Helper Functions to access fields of managed object
        static INT8& Get_m_fDisposed( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_INT8( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_UsbSerialInterface::FIELD__m_fDisposed ); }

        static UNSUPPORTED_TYPE& Get_m_evtDataEvent( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_UNSUPPORTED_TYPE( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_UsbSerialInterface::FIELD__m_evtDataEvent ); }

        static UNSUPPORTED_TYPE& Get_m_callbacksDataEvent( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_UNSUPPORTED_TYPE( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_UsbSerialInterface::FIELD__m_callbacksDataEvent ); }

        static UNSUPPORTED_TYPE& Get_ClientDataReceived( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_UNSUPPORTED_TYPE( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_UsbSerialInterface::FIELD__ClientDataReceived ); }

        static UNSUPPORTED_TYPE& Get__oneCharArray( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_UNSUPPORTED_TYPE( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_UsbSerialInterface::FIELD___oneCharArray ); }

        // Declaration of stubs. These functions are implemented by Interop code developers
        static UINT32 BytesInBuffer( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
        static INT32 mel_serial_tx( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, UINT32 param1, INT32 param2, HRESULT &hr );
        static INT32 mel_serial_rx( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, UINT32 param1, HRESULT &hr );
    };
}
#endif  //_SAMRAKSH_MEL_SAMRAKSH_MEL_USBSERIALINTERFACE_H_
