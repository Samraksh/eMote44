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


#include "Samraksh_Mel.h"
#include "Samraksh_Mel_Samraksh_Mel_AudioInterface.h"
#include <tinyhal.h>
#include <Samraksh/VirtualTimer.h>

using namespace Samraksh_Mel;

#define ML_SUCCESS 1
#define ML_FAIL 0
#define ARR_LEN(x) (sizeof(x) / sizeof(x[0]))

//#define DEBUG_MEL_INTEROPS
#ifndef DEBUG_MEL_INTEROPS
#define ml_printf(...) (void(0))
#else
#define ml_printf hal_printf
#endif

extern CLR_RT_HeapBlock_NativeEventDispatcher *AI_ne_Context;

static BOOL keepDown = TRUE;
static BOOL keepUp = FALSE;

void ManagedAICallback(UINT32 arg1, UINT32 arg2)
{
	GLOBAL_LOCK(irq);
	SaveNativeEventToHALQueue( AI_ne_Context, arg1, arg2 );
}

INT8 AudioInterface::set_fir_taps_internal( CLR_RT_HeapBlock* pMngObj, UINT32 param0, CLR_RT_TypedArray_float param1, HRESULT &hr )
{
	return ML_FAIL;
}

void AudioInterface::set_model_recording_internal( CLR_RT_HeapBlock* pMngObj, INT8 param0, INT8 param1, HRESULT &hr )
{
	keepUp = param0;
	keepDown = param1;
}

INT8 AudioInterface::Initialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
	return ML_SUCCESS;
}

INT8 AudioInterface::Uninitialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
	return ML_SUCCESS;
}

// FIX ME IN SUPER RUSH
float get_db_spl(void);
float * get_ml_upstream(void);
float * get_ml_downstream(void);

// interop will call this function to populate the float array that will be sent to the C# app callback function
INT8 AudioInterface::GetResultData( CLR_RT_HeapBlock* pMngObj, float * param0, CLR_RT_TypedArray_float param1, CLR_RT_TypedArray_float param2, HRESULT &hr )
{
	// Return dbSPL
	*param0 = get_db_spl();

	// Return upstream if on upstream_data
	if (keepUp) {
		float *my_ret = get_ml_upstream();
		float* data = param1.GetBuffer();
		for (int i = 0; i < 256; i++) {
			data[i] = my_ret[i];
		}
	}

	// Return downstream if on
	if (keepDown) {
		float *my_ret = get_ml_downstream();
		float* data = param2.GetBuffer();
		for (int i = 0; i < 8; i++){
			data[i] = my_ret[i];
		}
	}
    return ML_SUCCESS;
}

void start_microphone(void);
INT8 AudioInterface::start_audio_inference( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
	start_microphone();
	return ML_SUCCESS;
}

void stop_microphone(void);
void AudioInterface::stop_audio_inference( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
	stop_microphone();
}

INT8 AudioInterface::set_ml_duty_cycle( CLR_RT_HeapBlock* pMngObj, UINT32 param0, UINT32 param1, HRESULT &hr )
{
    return ML_FAIL;
}

INT8 AudioInterface::set_raw_data_output( CLR_RT_HeapBlock* pMngObj, INT8 param0, HRESULT &hr )
{
    return ML_FAIL;
}

INT8 AudioInterface::set_dB_thresh( CLR_RT_HeapBlock* pMngObj, float param0, HRESULT &hr )
{
    return ML_FAIL;
}

INT8 AudioInterface::set_time_interval( CLR_RT_HeapBlock* pMngObj, UINT32 param0, HRESULT &hr )
{
	return ML_FAIL;
}
