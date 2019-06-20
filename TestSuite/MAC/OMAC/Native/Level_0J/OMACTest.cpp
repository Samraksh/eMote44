/*
 * TimesyncTest.cpp
 */

/*
 * OMACTest.cpp
 *
 *  Created on: Oct 8, 2012
 *      Author: Mukundan Sridharan
 *      A simple Common test for all MACs
 */

//#include <Samraksh/HALTimer.h>
#include "OMACTest.h"

#define DEBUG_OMACTest 1
#define OMACTEST_Timer (GPIO_PIN)120//29
#define OMACTEST_Rx (GPIO_PIN)120//30
#define OMACTEST_Tx (GPIO_PIN)120//24
#define OMACTEST_TxAck (GPIO_PIN)120//31

const UINT16 ONESEC_IN_MSEC = 1000;
const UINT16 ONEMSEC_IN_USEC = 1000;

OMACTest g_OMACTest;
extern NeighborTable g_NeighborTable;
extern OMACType g_OMAC;
extern OMACScheduler g_omac_scheduler;
extern UINT16 MF_NODE_ID;

#define NEIGHBORCLOCKMONITORPIN 31 //2
#define LOCALCLOCKMONITORPIN 22 //25
#define MINEVENTTIM 1000

#define USEONESHOTTIMER FALSE

//NEIGHBORCLOCKMONITORPERIOD in ticks
#define NEIGHBORCLOCKMONITORPERIOD 800000
#define INITIALDELAY 100000


void Timer_32_Handler(void * arg){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Timer, TRUE);
#endif
	g_OMACTest.Send();
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Timer, FALSE);
#endif
}

// Typedef defining the signature of the receive function
//void ReceiveHandler (void *msg, UINT16 Size, UINT16 Src, BOOL Unicast, UINT8 RSSI, UINT8 LinkQuality){
void OMACTest_ReceiveHandler (void* msg, UINT16 size){
	return g_OMACTest.Receive(msg, size);
}

void OMACTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status){
	g_OMACTest.SendAck(msg,size,status);
}

void CMaxTSLocalClockMonitorTimerHandler(void * arg) {
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
	//ASSERT_SP(rm == TimerSupported);

	//Toggle Pin State for monitoring with Logic Analyzer
	if(g_OMACTest.LocalClkPINState){
		CPU_GPIO_SetPinState((GPIO_PIN) LOCALCLOCKMONITORPIN, false);
		g_OMACTest.LocalClkPINState = false;
	}
	else {
		CPU_GPIO_SetPinState((GPIO_PIN) LOCALCLOCKMONITORPIN, true);
		g_OMACTest.LocalClkPINState = true;
	}
	BOOL rv = g_OMACTest.ScheduleNextLocalCLK();

	rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
	//ASSERT_SP(rm == TimerSupported);
}

void CMaxTSNeighborClockMonitorTimerHandler(void * arg) {
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(NeighborClockMonitor_TIMER1);
	//ASSERT_SP(rm == TimerSupported);

	//Toggle Pin State for monitoring with Logic Analyzer
	UINT16 Nbr2beFollowed = g_OMAC.Neighbor2beFollowed;
	if(g_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) > 2 ) {//if ( g_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) >= 5 ))
		if(g_OMACTest.NeighborClkPINState){
			CPU_GPIO_SetPinState((GPIO_PIN) NEIGHBORCLOCKMONITORPIN, false);
			g_OMACTest.NeighborClkPINState = false;
		}
		else {
			CPU_GPIO_SetPinState((GPIO_PIN) NEIGHBORCLOCKMONITORPIN, true);
			g_OMACTest.NeighborClkPINState = true;
		}
	}
	BOOL rv = g_OMACTest.ScheduleNextNeighborCLK();

	rm = VirtTimer_Start(NeighborClockMonitor_TIMER1);
	//ASSERT_SP(rm == TimerSupported);
}

BOOL OMACTest::Initialize(){
	CPU_GPIO_EnableOutputPin((GPIO_PIN) NEIGHBORCLOCKMONITORPIN, TRUE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) LOCALCLOCKMONITORPIN, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) NEIGHBORCLOCKMONITORPIN, FALSE);
	CPU_GPIO_SetPinState((GPIO_PIN) LOCALCLOCKMONITORPIN, FALSE);
	LocalClkPINState = true;
	NeighborClkPINState = true;

	LocalClockMonitorFrameNum = 0;
	NeighborClockMonitorFrameNum = 0;

	MyAppID = 3; //pick a number less than MAX_APPS currently 4.
	Config.Network = 138;
	Config.NeighborLivenessDelay = 900000;
	myEventHandler.SetReceiveHandler(OMACTest_ReceiveHandler);
	myEventHandler.SetSendAckHandler(OMACTest_SendAckHandler);
	MacId = OMAC;

	VirtTimer_Initialize();
#ifdef DEBUG_OMACTest
	CPU_GPIO_EnableOutputPin(OMACTEST_Tx, FALSE);
	CPU_GPIO_EnableOutputPin(OMACTEST_Timer, FALSE);
	CPU_GPIO_EnableOutputPin(OMACTEST_Rx, FALSE);
	CPU_GPIO_EnableOutputPin(OMACTEST_TxAck, FALSE);
#endif
	Mac_Initialize(&myEventHandler, MacId, MyAppID, Config.RadioID, (void*) &Config);

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(32, 0, 10*ONESEC_IN_MSEC*ONEMSEC_IN_USEC, FALSE, FALSE, Timer_32_Handler); //period (3rd argument) is in micro seconds
	ASSERT(rm == TimerSupported);
	rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1, 0, 100*ONESEC_IN_MSEC, USEONESHOTTIMER, FALSE, CMaxTSLocalClockMonitorTimerHandler);
	ASSERT(rm == TimerSupported);
	rm = VirtTimer_SetTimer(NeighborClockMonitor_TIMER1, 0, 100*ONESEC_IN_MSEC, USEONESHOTTIMER, FALSE, CMaxTSNeighborClockMonitorTimerHandler);
	ASSERT(rm == TimerSupported);
	return TRUE;
}

BOOL OMACTest::StartTest(){
	VirtualTimerReturnMessage rm;
	msg.MSGID = 1;
	SendCount = 1;
	RcvCount = 0;

	rm = VirtTimer_Start(32);
	ASSERT(rm == TimerSupported);
	rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
	ASSERT(rm == TimerSupported);
	rm = VirtTimer_Start(NeighborClockMonitor_TIMER1);
	ASSERT(rm == TimerSupported);

	return TRUE;
}

void OMACTest::Receive(void* tmpMsg, UINT16 size){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Rx, TRUE);
	CPU_GPIO_SetPinState(OMACTEST_Rx, FALSE);
#endif
	Message_15_4_t* rcvdMsg = (Message_15_4_t*)tmpMsg;
	hal_printf("start OMACTest::Receive\n");
	hal_printf("OMACTest src is %u\n", rcvdMsg->GetHeader()->src);
	hal_printf("OMACTest dest is %u\n", rcvdMsg->GetHeader()->dest);
	Payload_t* payload = (Payload_t*)rcvdMsg->GetPayload();
	hal_printf("OMACTest msgID is: %u\n", payload->MSGID);
	hal_printf("OMACTest payload is \n");
	for(int i = 1; i <= 5; i++){
		hal_printf(" %d\n", payload->data[i-1]);
	}
	hal_printf("\n");

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Rx, TRUE);
	CPU_GPIO_SetPinState(OMACTEST_Rx, FALSE);
#endif
	hal_printf("end OMACTest::Receive\n");
}


BOOL OMACTest::Send(){
	msg.MSGID = SendCount;
	for(int i = 1; i <= 5; i++){
		msg.data[i-1] = i;
	}

	UINT16 Nbr2beFollowed = g_OMAC.Neighbor2beFollowed;
	if (g_NeighborTable.GetNeighborPtr(Nbr2beFollowed) == NULL) {
		return FALSE;
	}
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Tx, TRUE);
	CPU_GPIO_SetPinState(OMACTEST_Tx, FALSE);
	CPU_GPIO_SetPinState(OMACTEST_Tx, TRUE);
#endif
	//Mac_Send(MacId, MAC_BROADCAST_ADDRESS, MFM_DATA, (void*) &msg.data, sizeof(Payload_t));
	bool ispcktScheduled = Mac_Send(Nbr2beFollowed, MFM_DATA, (void*) &msg, sizeof(Payload_t));
	//if (ispcktScheduled) {SendCount++;}
	SendCount++;
}

void OMACTest::SendAck(void *msg, UINT16 size, NetOpStatus status){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_TxAck, TRUE);
	CPU_GPIO_SetPinState(OMACTEST_TxAck, FALSE);
#endif
	if(status == NO_Success){

	}else {

	}
}

BOOL OMACTest::ScheduleNextNeighborCLK(){
	UINT16 Nbr2beFollowed = g_OMAC.Neighbor2beFollowed;
	VirtualTimerReturnMessage rm;

	if (g_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) > 2 ) {//if ( g_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) >= 5 ){
		UINT64 NextEventTime = 0, neighborTime = 0, y = 0;
		UINT64 LocalFireUpTime = 0;
		UINT64 TicksTillNextEvent;

		//y = HAL_Time_CurrentTicks();
		while (LocalFireUpTime <= y || TicksTillNextEvent <= MINEVENTTIM){
			y = HAL_Time_CurrentTicks();
			neighborTime = g_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(Nbr2beFollowed, y);
			//neighborTime = y;
			while(NextEventTime <= neighborTime){
				NeighborClockMonitorFrameNum = NeighborClockMonitorFrameNum + 1;
				NextEventTime = ( NeighborClockMonitorFrameNum ) * ((UINT64)NEIGHBORCLOCKMONITORPERIOD);
			}
			LocalFireUpTime = g_omac_scheduler.m_TimeSyncHandler.m_globalTime.Neighbor2LocalTime(Nbr2beFollowed, NextEventTime);
			//LocalFireUpTime = NextEventTime;
			TicksTillNextEvent = LocalFireUpTime - y;
		}
		UINT32 MicSTillNextEvent = (UINT32) (HAL_Time_TicksToTime(TicksTillNextEvent));
		UINT32 ProcessingLatency = (UINT32) (HAL_Time_TicksToTime( HAL_Time_CurrentTicks() - y));
		rm = VirtTimer_Change(NeighborClockMonitor_TIMER1, 0, MicSTillNextEvent + ProcessingLatency, USEONESHOTTIMER);
		//ASSERT_SP(rm == TimerSupported);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

BOOL OMACTest::ScheduleNextLocalCLK(){
	VirtualTimerReturnMessage rm;
	UINT64 y = HAL_Time_CurrentTicks();
	UINT64 NextEventTime = 0;
	UINT64 TicksTillNextEvent = 0;
	while(NextEventTime <= y || TicksTillNextEvent <= MINEVENTTIM ){
		LocalClockMonitorFrameNum = LocalClockMonitorFrameNum + 1;
		NextEventTime = LocalClockMonitorFrameNum * ((UINT64)NEIGHBORCLOCKMONITORPERIOD);
		TicksTillNextEvent = NextEventTime - y;
	}

	UINT32 MicSTillNextEvent = (UINT32) (HAL_Time_TicksToTime(TicksTillNextEvent)) ;
	rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, MicSTillNextEvent, USEONESHOTTIMER);
	//ASSERT_SP(rm == TimerSupported);

	return TRUE;
}

void OMACTest_Initialize(){
	g_OMACTest.Initialize();
	g_OMACTest.StartTest();
}


