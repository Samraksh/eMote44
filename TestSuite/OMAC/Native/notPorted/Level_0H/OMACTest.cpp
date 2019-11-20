/*
 * RadioControlTest.cpp
 */
//#include <Samraksh/HALTimer.h>
#include "OMACTest.h"


#define DEBUG_OMACTest 1

const UINT16 ONESEC_IN_MSEC = 1000;
const UINT16 ONEMSEC_IN_USEC = 1000;

extern OMACTest g_OMACTest;

void Timer_32_Handler(void * arg){
	if(g_OMACTest.my_radio_state) {
		g_OMACTest.my_radio_state = false;
		g_OMACTest.radio_cont.Stop();
	}
	else{
		g_OMACTest.my_radio_state = true;;
		g_OMACTest.radio_cont.StartRx();
	}
}

// Typedef defining the signature of the receive function
//void ReceiveHandler (void *msg, UINT16 Size, UINT16 Src, BOOL Unicast, UINT8 RSSI, UINT8 LinkQuality){
void* OMACTest_ReceiveHandler (void* msg, UINT16 size){
	return msg;
}

void OMACTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status){
	return;
}

BOOL OMACTest_RadioInterruptHandler(RadioInterrupt Interrupt, void* Param){
	return TRUE;
}

BOOL OMACTest::Initialize(){
	VirtualTimerReturnMessage rm;
	DeviceStatus status;
	Config.Network = 138;
	Config.NeighborLivenessDelay = 900000;

	Radio_Event_Handler.RadioInterruptMask = (StartOfTransmission|EndOfTransmission|StartOfReception);
	Radio_Event_Handler.SetRadioInterruptHandler(OMACTest_RadioInterruptHandler);
	Radio_Event_Handler.SetReceiveHandler(OMACTest_ReceiveHandler);
	Radio_Event_Handler.SetSendAckHandler(OMACTest_SendAckHandler);

	radio_cont.Initialize();
	my_radio_state = false;
	//Config.RadioID = RF231RADIOLR;
	if((status = CPU_Radio_Initialize(&Radio_Event_Handler, Config.RadioID, 1, OMAC)) != DS_Success){
		return status;
	}

	VirtTimer_Initialize();
	rm = VirtTimer_SetTimer(32, 0, 1*ONESEC_IN_MSEC*ONEMSEC_IN_USEC, FALSE, FALSE, Timer_32_Handler); //period (3rd argument) is in micro seconds
	ASSERT_SP(rm == TimerSupported);

	return TRUE;
}

BOOL OMACTest::StartTest(){
	VirtTimer_Start(32);
	return TRUE;
}




void OMACTest_Initialize(){
	g_OMACTest.Initialize();
	g_OMACTest.StartTest();
}


