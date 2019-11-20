/*
 * TimesyncTest.cpp
 */
#include <tinyhal.h>
//#include <Samraksh/HALTimer.h>
#include <Samraksh/VirtualTimer.h>
#include "OMACTest.h"


//extern HALTimerManager gHalTimerManagerObject;
//extern VirtualTimer gVirtualTimerObject;
//extern VirtualTimerMapper gVirtualTimerMapperObject;
extern OMACTest gOMACTest;
extern OMACType g_OMAC;
extern UINT16 MF_NODE_ID;
//extern Buffer_15_4_t m_receive_buffer;


#define MINEVENTTIM 1000

#define USEONESHOTTIMER FALSE

//NEIGHBORCLOCKMONITORPERIOD in ticks
#define NEIGHBORCLOCKMONITORPERIOD 200000
#define INITIALDELAY 100000

void OMACTest_ReceiveHandler (void* msg, UINT16 NumOfPendingPackets){
}

void OMACTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus){
}

void CMaxTSLocalClockMonitorTimerHandler(void * arg) {
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);


	//Toggle Pin State for monitoring with Logic Analyzer
	if(gOMACTest.LocalClkPINState){
		CPU_GPIO_SetPinState(LOCALCLOCKMONITORPIN, false);
		gOMACTest.LocalClkPINState = false;
	}
	else {
		CPU_GPIO_SetPinState(LOCALCLOCKMONITORPIN, true);
		gOMACTest.LocalClkPINState = true;
	}
	BOOL rv = gOMACTest.ScheduleNextLocalCLK();


	rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);
}

void CMaxTSNeighborClockMonitorTimerHandler(void * arg) {
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(NeighborClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);


	//Toggle Pin State for monitoring with Logic Analyzer
	UINT16 Nbr2beFollowed = g_OMAC.Neighbor2beFollowed;
	if(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) > 2 ) {//if ( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) >= 5 ))
		if(gOMACTest.NeighborClkPINState){
			CPU_GPIO_SetPinState(NEIGHBORCLOCKMONITORPIN, false);
			gOMACTest.NeighborClkPINState = false;
		}
		else {
			CPU_GPIO_SetPinState(NEIGHBORCLOCKMONITORPIN, true);
			gOMACTest.NeighborClkPINState = true;
		}
	}
	BOOL rv = gOMACTest.ScheduleNextNeighborCLK();



	rm = VirtTimer_Start(NeighborClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);

}


// TIMESYNCTEST

BOOL OMACTest::Initialize(){

	CPU_GPIO_EnableOutputPin(NEIGHBORCLOCKMONITORPIN, TRUE);
	CPU_GPIO_EnableOutputPin(LOCALCLOCKMONITORPIN, TRUE);
	CPU_GPIO_SetPinState(NEIGHBORCLOCKMONITORPIN, FALSE);
	CPU_GPIO_SetPinState(LOCALCLOCKMONITORPIN, FALSE);
	LocalClkPINState = true;
	NeighborClkPINState = true;

	LocalClockMonitorFrameNum = 0;
	NeighborClockMonitorFrameNum = 0;

	MyAppID = 3; //pick a number less than MAX_APPS currently 4.
	Config.Network = 138;
	Config.NeighborLivenessDelay = 20000;
	myEventHandler.SetReceiveHandler(OMACTest_ReceiveHandler);
	myEventHandler.SetSendAckHandler(OMACTest_SendAckHandler);
	VirtTimer_Initialize();

	MacId = OMAC;
	Mac_Initialize(&myEventHandler,MacId, MyAppID, Config.RadioID, (void*) &Config);

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1, 0, NEIGHBORCLOCKMONITORPERIOD, USEONESHOTTIMER, FALSE, CMaxTSLocalClockMonitorTimerHandler, 4);
	ASSERT_SP(rm == TimerSupported);
	rm = VirtTimer_SetTimer(NeighborClockMonitor_TIMER1, 0, NEIGHBORCLOCKMONITORPERIOD, USEONESHOTTIMER, FALSE, CMaxTSNeighborClockMonitorTimerHandler, 4);
	assert(rm == TimerSupported);


	return TRUE;
}

BOOL OMACTest::StartTest(){
	VirtualTimerReturnMessage rm;

	UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	UINT64 LocalClockMonitorFrameNum = y /( (UINT64) NEIGHBORCLOCKMONITORPERIOD );

	ScheduleNextLocalCLK();
	ScheduleNextNeighborCLK();

	rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);
	rm = VirtTimer_Start(NeighborClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);

	return TRUE;
}

BOOL OMACTest::ScheduleNextNeighborCLK(){
	UINT16 Nbr2beFollowed = g_OMAC.Neighbor2beFollowed;
	VirtualTimerReturnMessage rm;

	if (g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) > 2 ) {//if ( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) >= 5 ){
		UINT64 NextEventTime = 0, neighborTime = 0, y = 0;
		UINT64 LocalFireUpTime = 0;
		UINT64 TicksTillNextEvent;


		//y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		while (LocalFireUpTime <= y || TicksTillNextEvent <= 40000){
			y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
			neighborTime = g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(Nbr2beFollowed, y);
			//neighborTime = y;
			NeighborClockMonitorFrameNum =  neighborTime / ((UINT64)NEIGHBORCLOCKMONITORPERIOD);
			while(NextEventTime <= neighborTime){
				NeighborClockMonitorFrameNum = NeighborClockMonitorFrameNum + 1;
				NextEventTime = ( NeighborClockMonitorFrameNum ) * ((UINT64)NEIGHBORCLOCKMONITORPERIOD);
			}
			LocalFireUpTime = g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Neighbor2LocalTime(Nbr2beFollowed, NextEventTime);
			//LocalFireUpTime = NextEventTime;
			TicksTillNextEvent = LocalFireUpTime - y;
		}
		UINT32 MicSTillNextEvent = (UINT32) (HAL_Time_TicksToTime(TicksTillNextEvent));
		UINT32 ProcessingLatency = (UINT32) (HAL_Time_TicksToTime( g_OMAC.m_Clock.GetCurrentTimeinTicks() - y));
		rm = VirtTimer_Change(NeighborClockMonitor_TIMER1, 0, MicSTillNextEvent + ProcessingLatency, USEONESHOTTIMER, 4);
		ASSERT_SP(rm == TimerSupported);
		return TRUE;
	}
	else {
		return FALSE;
	}


}

BOOL OMACTest::ScheduleNextLocalCLK(){
		VirtualTimerReturnMessage rm;
		UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		UINT64 NextEventTime = 0;
		UINT64 TicksTillNextEvent = 0;
		while(NextEventTime <= y || TicksTillNextEvent <= MINEVENTTIM ){
			LocalClockMonitorFrameNum = LocalClockMonitorFrameNum + 1;
			NextEventTime = LocalClockMonitorFrameNum * ((UINT64)NEIGHBORCLOCKMONITORPERIOD);
			TicksTillNextEvent = NextEventTime - y;
		}

		UINT32 MicSTillNextEvent = (UINT32) (HAL_Time_TicksToTime(TicksTillNextEvent)) ;
		rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, MicSTillNextEvent, USEONESHOTTIMER, 4);
		ASSERT_SP(rm == TimerSupported);


		return TRUE;
}

void OMACTest_Initialize(){
	BOOL ret;
	ret = gOMACTest.Initialize();
	ret = gOMACTest.StartTest();
}


