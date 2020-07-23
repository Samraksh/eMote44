//-----------------------------------------------------------------------------
//
//    ** DO NOT EDIT THIS FILE! **
//    This file was generated by a tool
//    re-running the tool will overwrite this file.
//
//-----------------------------------------------------------------------------


#ifndef _SAMRAKSH_MEL_H_
#define _SAMRAKSH_MEL_H_

#include <TinyCLR_Interop.h>
struct Library_Samraksh_Mel_Samraksh_Mel_AudioInterface
{
    static const int FIELD_STATIC__EMPTY_FLOAT = 0;

    static const int FIELD__collectUpStream = 5;
    static const int FIELD__collectDownStream = 6;
    static const int FIELD__audio_inference_callback = 7;

    TINYCLR_NATIVE_DECLARE(set_fir_taps_internal___BOOLEAN__U4__SZARRAY_R4);
    TINYCLR_NATIVE_DECLARE(set_model_recording_internal___VOID__BOOLEAN__BOOLEAN);
    TINYCLR_NATIVE_DECLARE(Initialize___BOOLEAN);
    TINYCLR_NATIVE_DECLARE(Uninitialize___BOOLEAN);
    TINYCLR_NATIVE_DECLARE(GetResultData___BOOLEAN__BYREF_R4__SZARRAY_R4__SZARRAY_R4);
    TINYCLR_NATIVE_DECLARE(start_audio_inference___BOOLEAN);
    TINYCLR_NATIVE_DECLARE(stop_audio_inference___VOID);
    TINYCLR_NATIVE_DECLARE(set_ml_duty_cycle___BOOLEAN__U4__U4);
    TINYCLR_NATIVE_DECLARE(set_raw_data_output___BOOLEAN__BOOLEAN);
    TINYCLR_NATIVE_DECLARE(set_dB_thresh___BOOLEAN__R4);
    TINYCLR_NATIVE_DECLARE(set_time_interval___BOOLEAN__U4);

    //--//

};

struct Library_Samraksh_Mel_Samraksh_Mel_UsbSerialInterface
{
    static const int FIELD__m_fDisposed = 1;
    static const int FIELD__m_evtDataEvent = 2;
    static const int FIELD__m_callbacksDataEvent = 3;
    static const int FIELD__ClientDataReceived = 4;
    static const int FIELD___oneCharArray = 5;

    TINYCLR_NATIVE_DECLARE(BytesInBuffer___U4);
    TINYCLR_NATIVE_DECLARE(mel_serial_tx___I4__SZARRAY_U1__U4__I4);
    TINYCLR_NATIVE_DECLARE(mel_serial_rx___I4__SZARRAY_U1__U4);

    //--//

};



extern const CLR_RT_NativeAssemblyData g_CLR_AssemblyNative_Samraksh_Mel;

#endif  //_SAMRAKSH_MEL_H_
