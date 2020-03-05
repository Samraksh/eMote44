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


#ifndef _SAMRAKSH_MEL_SAMRAKSH_MEL_AUDIOINTERFACE_H_
#define _SAMRAKSH_MEL_SAMRAKSH_MEL_AUDIOINTERFACE_H_

// from libSONYC_ML.a
extern "C" {
void MX_X_CUBE_AI_Init(void);
int aiRun(const void *in_data, void *out_data);
}

namespace Samraksh_Mel
{
    struct AudioInterface
    {
        // Helper Functions to access fields of managed object
        static INT32& Get_callbackDataSize( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_INT32( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_AudioInterface::FIELD__callbackDataSize ); }

        static UNSUPPORTED_TYPE& Get_audio_inference_callback( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_UNSUPPORTED_TYPE( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_AudioInterface::FIELD__audio_inference_callback ); }

        // Declaration of stubs. These functions are implemented by Interop code developers
        static INT8 Initialize( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr );
        static INT8 Uninitialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
        static INT8 GetResultData( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_float param0, HRESULT &hr );
        static INT8 start_audio_inference( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr );
        static INT8 stop_audio_inference( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
    };
}
#endif  //_SAMRAKSH_MEL_SAMRAKSH_MEL_AUDIOINTERFACE_H_
