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

extern CLR_RT_HeapBlock_NativeEventDispatcher *AI_ne_Context;

static int resultDataSize = 0;

void ManagedAICallback(UINT32 arg1, UINT32 arg2)
{
	UINT32 data1, data2;
	data1 = arg1;
	data2 = arg2;

	GLOBAL_LOCK(irq);
	SaveNativeEventToHALQueue( AI_ne_Context, data1, data2 );
}

void AudioInterfaceTimerHandler(void *arg){
	ManagedAICallback(0,0);
}

INT8 AudioInterface::Initialize( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr )
{
    INT8 retVal = 0; 
	hal_printf("ai init\r\n");
	if ((param0 > 0) && (param0 < 1024)) {
		resultDataSize = param0;
	} else {
		resultDataSize = 6;
	}
	VirtTimer_SetTimer(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK, 0, 1000000, FALSE, FALSE, AudioInterfaceTimerHandler);
	VirtTimer_Start(VIRT_TIMER_AUDIO_INTERFACE_CALLBACK);
    return retVal;
}

INT8 AudioInterface::Uninitialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
    INT8 retVal = 0; 
    return retVal;
}

INT8 AudioInterface::GetResultData( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_float param0, HRESULT &hr )
{
	// interop will call this function to populate the float array that will be sent to the C# app callback function

    INT8 retVal = 0; 
	hal_printf("ai get results data\r\n");
	float* data = param0.GetBuffer();

	// populating data with bogus values to test
	// TODO: populate with actual data
	static int testCnt = 1;
	for (int i = 0; i < resultDataSize; i++){
		data[i] = testCnt * i * 1.5;
	}
	testCnt++;

    return retVal;
}

INT8 AudioInterface::start_audio_interference( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr )
{
    INT8 retVal = 0; 
	hal_printf("ai start audio\r\n");
    return retVal;
}

INT8 AudioInterface::stop_audio_interference( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
    INT8 retVal = 0; 
	hal_printf("ai stop audio\r\n");
    return retVal;
}

