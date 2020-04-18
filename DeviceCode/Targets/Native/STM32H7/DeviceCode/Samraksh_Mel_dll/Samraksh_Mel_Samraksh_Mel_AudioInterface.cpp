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

static int16_t lfsr1(void)
{
    static uint16_t start_state = 0xACE1u;  /* Any nonzero start state will work. */
    uint16_t lfsr = start_state;
    uint16_t bit;                    /* Must be 16-bit to allow bit<<15 later in the code */

    {   /* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) /* & 1 */;
        lfsr = (lfsr >> 1) | (bit << 15);
    }
    start_state = lfsr;
	return lfsr;
}

static float rand_float(void) {
	int16_t x = lfsr1();
	return x/-32768.0;
}

extern CLR_RT_HeapBlock_NativeEventDispatcher *AI_ne_Context;

static float junk_data_delete_me[64][51];
static float ai_output[8];
static float upstream[256];
static float dbSPL = -1.0;
static BOOL is_SONYC_ML_init;
static BOOL keepDown = TRUE;
static BOOL keepUp = FALSE;

void ManagedAICallback(UINT32 arg1, UINT32 arg2)
{
	GLOBAL_LOCK(irq);
	SaveNativeEventToHALQueue( AI_ne_Context, arg1, arg2 );
}

void AudioInterfaceTimerHandler(void *arg){
	float *x = (float *)junk_data_delete_me;
	if (is_SONYC_ML_init == FALSE) {
		ml_printf("%s(): ERROR NOT INIT\r\n", __func__);
		return;
	}
	for(int i=0; i<sizeof(junk_data_delete_me)/sizeof(junk_data_delete_me[0]); i++) {
		x[i] = rand_float();
	}
#ifndef KILL_SONYC_MODEL
	aiRun(junk_data_delete_me, upstream);
	aiRun2(upstream, ai_output);
#endif // #ifndef KILL_SONYC_MODEL
	ManagedAICallback(0,0);
}

INT8 AudioInterface::set_fir_taps_internal( CLR_RT_HeapBlock* pMngObj, INT32 param0, CLR_RT_TypedArray_float param1, HRESULT &hr )
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
	ml_printf("%s()\r\n", __func__);
#ifndef KILL_SONYC_MODEL
	MX_X_CUBE_AI_Init();
#endif // #ifndef KILL_SONYC_MODEL
	VirtTimer_SetTimer(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK, 0, 1000000, FALSE, FALSE, AudioInterfaceTimerHandler);
	is_SONYC_ML_init = TRUE;
	return ML_SUCCESS;
}

INT8 AudioInterface::Uninitialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
	ml_printf("%s()\r\n", __func__);
	if (is_SONYC_ML_init)
		VirtTimer_Stop(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK);
	is_SONYC_ML_init = FALSE;
    return ML_SUCCESS;
}

// static float dbSPL = -1.0;
// static BOOL is_SONYC_ML_init;
// static BOOL keepDown = TRUE;
// static BOOL keepUp = FALSE;
// interop will call this function to populate the float array that will be sent to the C# app callback function
INT8 AudioInterface::GetResultData( CLR_RT_HeapBlock* pMngObj, float * param0, CLR_RT_TypedArray_float param1, CLR_RT_TypedArray_float param2, HRESULT &hr )
{
	INT8 ret = ML_SUCCESS;

	if (is_SONYC_ML_init == FALSE) {
		ml_printf("%s(): ERROR NOT INIT\r\n", __func__);
		return ML_FAIL;
	} else {
		ml_printf("%s()\r\n", __func__);
	}

	// Return dbSPL
	*param0 = dbSPL;

	// Return upstream if on upstream_data
	if (keepUp) {
		// TODO: Check Size? param0.GetSize() if bad return CLR_E_BUFFER_TOO_SMALL ?? Size is bytes or len??? check below
		volatile int debug_check_delete_me1 = param1.GetSize();
		float* data = param1.GetBuffer();
		for (int i = 0; i < ARR_LEN(upstream); i++){
			data[i] = upstream[i];
		}
	}

	// Return downstream if on
	// TODO: This is bogus data
	if (keepDown) {
		volatile int debug_check_delete_me2 = param2.GetSize();
		float* data = param2.GetBuffer();
		for (int i = 0; i < ARR_LEN(ai_output); i++){
			data[i] = ai_output[i];
		}
	}

    return ret;
}

INT8 AudioInterface::start_audio_inference( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
	if (is_SONYC_ML_init == FALSE) {
		ml_printf("%s(): ERROR NOT INIT\r\n", __func__);
		return ML_FAIL;
	} else {
		ml_printf("%s()\r\n", __func__);
	}
	VirtTimer_Start(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK);
    return ML_SUCCESS;
}

void AudioInterface::stop_audio_inference( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
	if (is_SONYC_ML_init == FALSE) return;
	else VirtTimer_Stop(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK);
}

INT8 AudioInterface::set_ml_duty_cycle( CLR_RT_HeapBlock* pMngObj, INT32 param0, INT32 param1, HRESULT &hr )
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

INT8 AudioInterface::set_time_interval( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr )
{
	return ML_FAIL;
}
