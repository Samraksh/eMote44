/*
 * Samraksh Time Driver
 * Initial Create - Kartik Natarajan
 * 04/20/2011
 * Modified - Mukundan Sridharan; Ananth Muralidharan
 * 07/25/2014
 * Notes - 1) Check ticks to time conversion math
 * 		   2) Implement the Sleep functions, CPU_CPWAIT()
 * 		   3) CounterValue function, maybe need to re-implement in a different way!
 *
 */

#include "eMote_Time.h"
#include <Samraksh/VirtualTimer.h>

#ifndef VIRT_TIMER_SLEEP
#define VIRT_TIMER_SLEEP 6
#endif
//#define DEBUG_TIMER

Time_Driver g_Time_Driver;

//void TimeHandler(void *arg);
void SetCompareHandler(void *arg);


BOOL Time_Driver::Initialize()
{
	BOOL retVal = TRUE;

	CPU_Timer_Initialize_System_time();

	//retVal = retVal && (VirtTimer_SetTimer(VIRT_TIMER_RTC_ONE_SEC, 0, 1000000, FALSE, TRUE, TimeHandlerRTC, LOW_DRIFT_TIMER) == TimerSupported);
	//retVal = retVal && (VirtTimer_Start( VIRT_TIMER_RTC_ONE_SEC ) == TimerSupported);

	retVal = retVal && (VirtTimer_SetTimer(VIRT_TIMER_SLEEP, 0, CPU_Timer_GetMaxTicks(LOW_DRIFT_TIMER), FALSE, TRUE, SetCompareHandler, LOW_DRIFT_TIMER) == TimerSupported);
	retVal = retVal && (VirtTimer_SetTimer(VIRT_TIMER_EVENTS, 0, CPU_Timer_GetMaxTicks(ADVTIMER_32BIT), FALSE, TRUE, SetCompareHandler, ADVTIMER_32BIT) == TimerSupported);

	return retVal;
}


BOOL Time_Driver::Uninitialize()
{
	BOOL retVal = TRUE;

	retVal = retVal && (VirtTimer_Stop( VIRT_TIMER_EVENTS ) == TimerSupported);
	ASSERT(retVal);

	retVal = retVal && (VirtTimer_Stop( VIRT_TIMER_SLEEP ) == TimerSupported);
	//ASSERT(retVal);

	return retVal;
}

UINT64 Time_Driver::CurrentTicks()
{
	UINT64 currentTicks = CPU_Timer_CurrentTicks(SYSTEM_TIME);

	return currentTicks;
}

UINT32 Time_Driver::CounterValue()
{
	return (UINT32)(CPU_Timer_CurrentTicks(SYSTEM_TIME));
}


void SetCompareHandler(void *arg)
{
	HAL_COMPLETION::DequeueAndExec();
}

void Time_Driver::StopTimer()
{
	BOOL success = ( VirtTimer_Stop( VIRT_TIMER_EVENTS ) == TimerSupported );
	ASSERT(success);
}

void Time_Driver::SetCompareValue( UINT64 compareTicks )
{
	UINT32 compareTimeInMicroSecs = 0;

	// for now we are capping compares to a 32-bit max value, but we need to move to 64-bit times and clean this up
	if(compareTicks > 0xFFFFFFFF)
	{
		compareTicks = 0xFFFFFFFF;
	}
	ASSERT(compareTicks < 0xFFFFFFFF); // assert we are not losing time. this is called by Completions.
	compareTimeInMicroSecs = CPU_TicksToMicroseconds((UINT32)compareTicks, ADVTIMER_32BIT);

	if(VirtTimer_Change(VIRT_TIMER_EVENTS, compareTimeInMicroSecs, 0, TRUE, ADVTIMER_32BIT) != TimerSupported)
	{
		if(VirtTimer_SetTimer(VIRT_TIMER_EVENTS, compareTimeInMicroSecs, 0, TRUE, TRUE, SetCompareHandler, ADVTIMER_32BIT) != TimerSupported)
		{
			ASSERT(FALSE);
		}
	}

	VirtTimer_Start( VIRT_TIMER_EVENTS );
}

void Time_Driver::StopTimerSleepClock()
{
	BOOL success = ( VirtTimer_Stop( VIRT_TIMER_SLEEP ) == TimerSupported );
	ASSERT(success);
}

void Time_Driver::SetCompareValueSleepClockMicroSeconds( UINT32 compareTimeInMicroSecs )
{
	if(VirtTimer_Change(VIRT_TIMER_SLEEP, compareTimeInMicroSecs, 0, TRUE, LOW_DRIFT_TIMER) != TimerSupported)
	{
		if(VirtTimer_SetTimer(VIRT_TIMER_SLEEP, compareTimeInMicroSecs, 0, TRUE, TRUE, SetCompareHandler, LOW_DRIFT_TIMER) != TimerSupported)
		{
			ASSERT(FALSE);
		}
	}

	VirtTimer_Start( VIRT_TIMER_SLEEP );
}


INT64 Time_Driver::TicksToTime( UINT64 Ticks )
{
	return (INT64)(CPU_TicksToTime( Ticks, SYSTEM_TIME));
}

INT64 Time_Driver::TicksToMicroseconds( UINT64 Ticks )
{
	return (INT64)(CPU_TicksToMicroseconds(Ticks, SYSTEM_TIME));
}

INT64 Time_Driver::CurrentTime()
{
	INT32 a, b;
	INT64 c;
	CPU_GetDriftParameters(&a, &b, &c);

	UINT64 currentTotalTicks = CurrentTicks();

	GLOBAL_LOCK(irq);
	UINT64 currentTime = 0;

	if (a == 1)
	{
		currentTime = (currentTotalTicks * b + c);
	} else if (a == 2) {
		currentTime = (currentTotalTicks * b + c);
		currentTime = currentTime >> 1;
	} else if (a == 4) {
		currentTime = (currentTotalTicks * b + c);
		currentTime = currentTime >> 2;
	} else if(a != 0) {
		currentTime = (currentTotalTicks * b + c) / a;
	}

	return (INT64)currentTime;
}

INT64 Time_Driver::TimeNow()
{
	return CurrentTime();
}

void Time_Driver::Sleep_uSec( UINT32 uSec )
{	
	HAL_Time_Sleep_MicroSeconds( uSec);
}

// timeToAdd is in 100-nanosecond (ns) increments. This is a Microsoft thing.
void Time_Driver::AddClockTime(UINT64 timeToAdd)
{
	// After waking up, we calculated how much time needs to be added to the system clock
	// TODO: we probably need to add this time to ever timer
	CPU_AddClockTime(ADVTIMER_32BIT, timeToAdd);
	// After updating the timers, we need to set the alarms again.
	VirtTimer_UpdateAlarms();
}