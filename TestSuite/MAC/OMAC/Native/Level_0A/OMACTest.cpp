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
extern OMACScheduler g_omac_scheduler;
extern OMACType g_OMAC;
extern UINT16 MF_NODE_ID;
//extern Buffer_15_4_t m_receive_buffer;

#define NEIGHBORCLOCKMONITORPIN 31 //2
#define LOCALCLOCKMONITORPIN 22 //25

#define LocalClockMonitor_TIMER 32
#define NeighborClockMonitor_TIMER 33

#define USEONESHOTTIMER TRUE

//NEIGHBORCLOCKMONITORPERIOD in ticks
#define NEIGHBORCLOCKMONITORPERIOD 800000
#define INITIALDELAY 100000

void OMACTest_ReceiveHandler (void* msg, UINT16 NumOfPendingPackets){
}

void OMACTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status){
}

void CMaxTSLocalClockMonitorTimerHandler(void * arg) {

	gOMACTest.IsLocalCLKScheduled = false;
	//Toggle Pin State for monitoring with Logic Analyzer
	if(gOMACTest.LocalClkPINState){
		CPU_GPIO_SetPinState((GPIO_PIN) LOCALCLOCKMONITORPIN, false);
		gOMACTest.LocalClkPINState = false;
	}
	else {
		CPU_GPIO_SetPinState((GPIO_PIN) LOCALCLOCKMONITORPIN, true);
		gOMACTest.LocalClkPINState = true;
	}

	//BOOL rv = gOMACTest.ScheduleNextLocalCLK();
	//VirtualTimerReturnMessage rm;
	//rm = VirtTimer_Start(LocalClockMonitor_TIMER);
}

void CMaxTSNeighborClockMonitorTimerHandler(void * arg) {
	//Toggle Pin State for monitoring with Logic Analyzer
	gOMACTest.IsNeighborCLKScheduled = false;
	if(gOMACTest.NeighborClkPINState){
		CPU_GPIO_SetPinState((GPIO_PIN) NEIGHBORCLOCKMONITORPIN, false);
		gOMACTest.NeighborClkPINState = false;
	}
	else {
		CPU_GPIO_SetPinState((GPIO_PIN) NEIGHBORCLOCKMONITORPIN, true);
		gOMACTest.NeighborClkPINState = true;
	}
	//BOOL rv = gOMACTest.ScheduleNextNeighborCLK();
	//VirtualTimerReturnMessage rm;
	//rm = VirtTimer_Start(NeighborClockMonitor_TIMER);
}


// TIMESYNCTEST

BOOL OMACTest::Initialize(){

	CPU_GPIO_EnableOutputPin((GPIO_PIN) NEIGHBORCLOCKMONITORPIN, TRUE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) LOCALCLOCKMONITORPIN, TRUE);
	LocalClkPINState = true;
	NeighborClkPINState = true;

	IsLocalCLKScheduled = false;
	IsNeighborCLKScheduled = false;

	MyAppID = 3; //pick a number less than MAX_APPS currently 4.
	Config.Network = 138;
	Config.NeighborLivenessDelay = 20000;
	myEventHandler.SetReceiveHandler(OMACTest_ReceiveHandler);
	myEventHandler.SetSendAckHandler(OMACTest_SendAckHandler);
	VirtTimer_Initialize();

	MacId = OMAC;
	Mac_Initialize(&myEventHandler,MacId, MyAppID, Config.RadioID, (void*) &Config);

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER, 0, NEIGHBORCLOCKMONITORPERIOD, USEONESHOTTIMER, FALSE, CMaxTSLocalClockMonitorTimerHandler);
	rm = VirtTimer_SetTimer(NeighborClockMonitor_TIMER, 0, NEIGHBORCLOCKMONITORPERIOD, USEONESHOTTIMER, FALSE, CMaxTSNeighborClockMonitorTimerHandler);


	return TRUE;
}

BOOL OMACTest::StartTest(){
	VirtualTimerReturnMessage rm;

	while(1){
		if(!IsLocalCLKScheduled) {
			ScheduleNextLocalCLK();
		}
		if(!IsNeighborCLKScheduled) {
			ScheduleNextNeighborCLK();
		}
	}
	return TRUE;
}

BOOL OMACTest::ScheduleNextNeighborCLK(){
	UINT16 Nbr2beFollowed = g_OMAC.Neighbor2beFollowed;
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(NeighborClockMonitor_TIMER);
	if (g_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) > 4 ) {//if ( g_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) >= 5 ){
		UINT64 y = HAL_Time_CurrentTicks();
		// TODO: Check if neighbor was registered(at least 2 packets were received)
		UINT64 neighborTime = g_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(Nbr2beFollowed, HAL_Time_CurrentTicks());
		UINT64 NextEventTime = ( ( neighborTime/( (UINT64) NEIGHBORCLOCKMONITORPERIOD) ) + 1 ) * ((UINT64)NEIGHBORCLOCKMONITORPERIOD);
		UINT64 TicksTillNextEvent = g_omac_scheduler.m_TimeSyncHandler.m_globalTime.Neighbor2LocalTime(Nbr2beFollowed, NextEventTime) - y;
		UINT32 MicSTillNextEvent = (UINT32) (HAL_Time_TicksToTime(TicksTillNextEvent));
		UINT32 ProcessingLatency = (UINT32) (HAL_Time_TicksToTime( HAL_Time_CurrentTicks() - y));
		MicSTillNextEvent = MicSTillNextEvent + ProcessingLatency;
		if ( MicSTillNextEvent > 100000){
			MicSTillNextEvent = 40000000;
		}
		else if(MicSTillNextEvent < 10000){
			NextEventTime = NextEventTime + NEIGHBORCLOCKMONITORPERIOD;
			TicksTillNextEvent = g_omac_scheduler.m_TimeSyncHandler.m_globalTime.Neighbor2LocalTime(Nbr2beFollowed, NextEventTime) - y;
			MicSTillNextEvent = (UINT32) (HAL_Time_TicksToTime(TicksTillNextEvent));
			ProcessingLatency = (UINT32) (HAL_Time_TicksToTime( HAL_Time_CurrentTicks() - y));
			MicSTillNextEvent = MicSTillNextEvent + ProcessingLatency;

			if( (MicSTillNextEvent > 100000) ) MicSTillNextEvent = 50000000;
			if( (MicSTillNextEvent < 10000) ) MicSTillNextEvent = 60000000;
		}
		rm = VirtTimer_Change(NeighborClockMonitor_TIMER, 0, MicSTillNextEvent, USEONESHOTTIMER);
		rm = VirtTimer_Start(NeighborClockMonitor_TIMER);
		IsNeighborCLKScheduled = true;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

BOOL OMACTest::ScheduleNextLocalCLK(){
		VirtualTimerReturnMessage rm;
		rm = VirtTimer_Stop(LocalClockMonitor_TIMER);
		UINT64 y = HAL_Time_CurrentTicks();
		UINT64 NextEventTime = ( y/ ((UINT64) NEIGHBORCLOCKMONITORPERIOD)  + 1) * ((UINT64)NEIGHBORCLOCKMONITORPERIOD);
		UINT64 TicksTillNextEvent = NextEventTime - y;
		UINT32 MicSTillNextEvent = (UINT32) (HAL_Time_TicksToTime(TicksTillNextEvent)) ;
		rm = VirtTimer_Change(LocalClockMonitor_TIMER, 0, MicSTillNextEvent, USEONESHOTTIMER);
		rm = VirtTimer_Start(LocalClockMonitor_TIMER);
		IsLocalCLKScheduled = true;
		return TRUE;
}

void OMACTest_Initialize(){
	BOOL ret;
	ret = gOMACTest.Initialize();
	ret = gOMACTest.StartTest();
}


