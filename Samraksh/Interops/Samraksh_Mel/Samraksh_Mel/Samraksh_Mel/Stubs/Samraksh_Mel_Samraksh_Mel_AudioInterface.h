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

namespace Samraksh_Mel
{
    struct AudioInterface
    {
        // Helper Functions to access fields of managed object
        static INT8& Get_collectUpStream( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_INT8( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_AudioInterface::FIELD__collectUpStream ); }

        static INT8& Get_collectDownStream( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_INT8( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_AudioInterface::FIELD__collectDownStream ); }

        static UNSUPPORTED_TYPE& Get_audio_inference_callback( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_UNSUPPORTED_TYPE( pMngObj, Library_Samraksh_Mel_Samraksh_Mel_AudioInterface::FIELD__audio_inference_callback ); }

        // Declaration of stubs. These functions are implemented by Interop code developers
        static void mel_get_thresh( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_float param0, HRESULT &hr );
        static INT32 mel_set_thresh( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_float param0, HRESULT &hr );
        static INT8 set_fir_taps_internal( CLR_RT_HeapBlock* pMngObj, UINT32 param0, CLR_RT_TypedArray_float param1, HRESULT &hr );
        static void set_model_recording_internal( CLR_RT_HeapBlock* pMngObj, INT8 param0, INT8 param1, HRESULT &hr );
        static INT8 Initialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
        static INT8 Uninitialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
        static INT8 GetResultData( CLR_RT_HeapBlock* pMngObj, float * param0, CLR_RT_TypedArray_float param1, CLR_RT_TypedArray_float param2, HRESULT &hr );
        static INT8 start_audio_inference( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
        static void stop_audio_inference( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
        static INT8 set_ml_duty_cycle( CLR_RT_HeapBlock* pMngObj, UINT32 param0, UINT32 param1, HRESULT &hr );
        static INT8 set_raw_data_output( CLR_RT_HeapBlock* pMngObj, INT8 param0, HRESULT &hr );
        static INT8 set_dB_thresh( CLR_RT_HeapBlock* pMngObj, float param0, HRESULT &hr );
        static INT8 set_time_interval( CLR_RT_HeapBlock* pMngObj, UINT32 param0, HRESULT &hr );
    };
}
#endif  //_SAMRAKSH_MEL_SAMRAKSH_MEL_AUDIOINTERFACE_H_
