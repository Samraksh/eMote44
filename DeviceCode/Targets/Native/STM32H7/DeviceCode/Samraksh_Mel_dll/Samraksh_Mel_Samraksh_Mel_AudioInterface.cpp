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

#define DEBUG_MEL_INTEROPS
#ifndef DEBUG_MEL_INTEROPS
#define hal_printf (void(0))
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
static BOOL is_SONYC_ML_init;

void ManagedAICallback(UINT32 arg1, UINT32 arg2)
{
	UINT32 data1, data2;
	data1 = arg1;
	data2 = arg2;

	GLOBAL_LOCK(irq);
	SaveNativeEventToHALQueue( AI_ne_Context, data1, data2 );
}

void AudioInterfaceTimerHandler(void *arg){
	float *x = (float *)junk_data_delete_me;
	if (is_SONYC_ML_init == FALSE) {
		hal_printf("%s(): ERROR NOT INIT\r\n", __func__);
		return;
	}
	for(int i=0; i<sizeof(junk_data_delete_me)/sizeof(junk_data_delete_me[0]); i++) {
		x[i] = rand_float();
	}
	aiRun(junk_data_delete_me, ai_output);
	ManagedAICallback(0,0);
}


INT8 AudioInterface::Initialize( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr )
{
	hal_printf("%s()\r\n", __func__);
	MX_X_CUBE_AI_Init();
	VirtTimer_SetTimer(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK, 0, 1000000, FALSE, FALSE, AudioInterfaceTimerHandler);
	is_SONYC_ML_init = TRUE;
	/*
    INT8 retVal = 0; 

	VirtTimer_SetTimer(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK, 0, 1000000, FALSE, FALSE, AudioInterfaceTimerHandler);
	VirtTimer_Start(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK);
    return retVal;
	*/
}

INT8 AudioInterface::Uninitialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
	hal_printf("%s()\r\n", __func__);
	is_SONYC_ML_init = FALSE;
    INT8 retVal = 0; 
    return retVal;
}

INT8 AudioInterface::GetResultData( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_float param0, HRESULT &hr )
{
	// interop will call this function to populate the float array that will be sent to the C# app callback function

    INT8 retVal = 0; 
	if (is_SONYC_ML_init == FALSE) {
		hal_printf("%s(): ERROR NOT INIT\r\n", __func__);
		return 0;
	} else {
		hal_printf("%s()\r\n", __func__);
	}
	float* data = param0.GetBuffer();

	// populating data with bogus values to test
	// TODO: populate with actual data
	static int testCnt = 1;
	for (int i = 0; i < sizeof(ai_output); i++){
		data[i] = ai_output[i];
	}

    return retVal;
}

INT8 AudioInterface::start_audio_inference( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr )
{
    INT8 retVal = 0; 
	if (is_SONYC_ML_init == FALSE) {
		hal_printf("%s(): ERROR NOT INIT\r\n", __func__);
		return 0;
	} else {
		hal_printf("%s()\r\n", __func__);
	}
	VirtTimer_Start(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK);
    return retVal;
}

INT8 AudioInterface::stop_audio_inference( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
    INT8 retVal = 0; 
	if (is_SONYC_ML_init == FALSE) {
		hal_printf("%s(): ERROR NOT INIT\r\n", __func__);
		return 0;
	} else {
		hal_printf("%s()\r\n", __func__);
	}
	VirtTimer_Stop(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK);
    return retVal;
}

