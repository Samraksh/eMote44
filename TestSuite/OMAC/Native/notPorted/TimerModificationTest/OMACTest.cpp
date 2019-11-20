/*
 * TimerModificationTest.cpp
 */
#include <tinyhal.h>
//#include <Samraksh/HALTimer.h>
#include <Samraksh/VirtualTimer.h>
#include "OMACTest.h"


#define NEIGHBORCLOCKMONITORPIN 31 //2
#define LOCALCLOCKMONITORPIN 22 //25


#define USEONESHOTTIMER FALSE

//NEIGHBORCLOCKMONITORPERIOD in ticks
#define NEIGHBORCLOCKMONITORPERIOD 4000000
#define INITIALDELAY 100000

void CMaxTSLocalClockMonitorTimerHandler(void * arg) {
	//Toggle Pin State for monitoring with Logic Analyzer
	VirtualTimerReturnMessage rm;
	//rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 10000000, USEONESHOTTIMER);
	CPU_GPIO_SetPinState((GPIO_PIN) LOCALCLOCKMONITORPIN, TRUE);
	rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);
	BOOL rv = gOMACTest.ScheduleNextLocalCLK();
	rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);
	CPU_GPIO_SetPinState((GPIO_PIN) LOCALCLOCKMONITORPIN, FALSE);
}



// TIMESYNCTEST

BOOL OMACTest::Initialize(){

	CPU_GPIO_EnableOutputPin((GPIO_PIN) NEIGHBORCLOCKMONITORPIN, TRUE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) LOCALCLOCKMONITORPIN, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) NEIGHBORCLOCKMONITORPIN, FALSE);
	CPU_GPIO_SetPinState((GPIO_PIN) LOCALCLOCKMONITORPIN, FALSE);


	VirtTimer_Initialize();


	m_period1 = 37;
	m_period2 = 191;

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1, 0, NEIGHBORCLOCKMONITORPERIOD, USEONESHOTTIMER, FALSE, CMaxTSLocalClockMonitorTimerHandler);
	ASSERT_SP(rm == TimerSupported);

	return TRUE;
}

BOOL OMACTest::StartTest(){
	VirtualTimerReturnMessage rm;
	UINT64 y = HAL_Time_CurrentTicks();
	UINT64 LocalClockMonitorFrameNum = y /( (UINT64) NEIGHBORCLOCKMONITORPERIOD );
	ScheduleNextLocalCLK();
	rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);
	return TRUE;
}



BOOL OMACTest::ScheduleNextLocalCLK(){
		VirtualTimerReturnMessage rm;
		UINT64 currentSlotNum, period1Remaining, period2Remaining;
		UINT64 y = HAL_Time_CurrentTicks();
		currentSlotNum = y / (16 * 8000);
		period1Remaining = currentSlotNum % m_period1;
		period2Remaining = currentSlotNum % m_period2;
		period1Remaining = ((period1Remaining < period2Remaining) ? (period1Remaining ) : (period2Remaining ));
		UINT32 MicSTillNextEvent = period1Remaining * 16 * 1000;
		if(MicSTillNextEvent > (m_period1*16000)){
			MicSTillNextEvent = 9000000;
		}
		if(MicSTillNextEvent == 0) {
			MicSTillNextEvent = 5000000;
		}
		rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, MicSTillNextEvent, USEONESHOTTIMER);
		ASSERT_SP(rm == TimerSupported);

//		rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
//		ASSERT_SP(rm == TimerSupported);

		return TRUE;

}

void OMACTest_Initialize(){
	BOOL ret;
	ret = gOMACTest.Initialize();
	ret = gOMACTest.StartTest();
}


