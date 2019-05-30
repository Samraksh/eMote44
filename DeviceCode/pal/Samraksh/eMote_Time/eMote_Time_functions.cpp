/*
 * Samraksh Time Driver
 * Initial Create - Mukundan Sridharan; Ananth Muralidharan
 * 07/24/2014
 *
 */

#include "eMote_Time.h"

static const UINT64 HAL_Completion_IdleValue = 0x0000FFFFFFFFFFFFull;


BOOL HAL_Time_Initialize()
{
	return g_Time_Driver.Initialize();
}

BOOL HAL_Time_Uninitialize()
{
	return g_Time_Driver.Uninitialize();
}



UINT64 Time_CurrentTicks()
{
	//return g_Time_Driver.CurrentTicks();
	return HAL_Time_CurrentTicks();
}


INT64 HAL_Time_TicksToMicroseconds( UINT64 Ticks )
{
	return g_Time_Driver.TicksToMicroseconds(Ticks);
}

#if defined(DEBUG_EMOTE_TIME)
volatile UINT64 badComparesCount = 0;    //!< number of requests set in the past.
volatile UINT64 badComparesAvg = 11;     //!< average delay of requests set in the past. init to observed value.
volatile UINT64 badComparesMax = 0;      //!< observed worst-case.
#endif

void HAL_Time_SetCompare( UINT64 CompareTicks )
{
	// here we change a 64-bit time from the MF to a 32-bit time
	// we need to move to 64-bit absolute time and clean this portion of code up
	if (CompareTicks == HAL_Completion_IdleValue){
		g_Time_Driver.StopTimer();
	} else {
		UINT64 NowTicks = HAL_Time_CurrentTicks();
		if(CompareTicks > NowTicks) {
			g_Time_Driver.SetCompareValue(CompareTicks - NowTicks );
		}
		else {
#if defined(DEBUG_EMOTE_TIME)
			++badComparesCount;
			if(badComparesMax < (NowTicks - CompareTicks)) { badComparesMax = NowTicks - CompareTicks; }
			badComparesAvg = (badComparesAvg * (badComparesCount - 1) + (NowTicks - CompareTicks)) / badComparesCount;
#endif
			g_Time_Driver.SetCompareValue( 300 );  // assume g_Time_Driver uses virtual timer so compare value cannot miss and therefore any small compare value suffices.
		}
	}
}

void HAL_Time_SetCompare_Sleep_Clock_MicroSeconds( UINT32 compareTimeInMicroSecs )
{
	g_Time_Driver.SetCompareValueSleepClockMicroSeconds(compareTimeInMicroSecs);
}

void HAL_Time_GetDriftParameters  ( INT32* a, INT32* b, INT64* c )
{
	CPU_GetDriftParameters(a, b, c);
}

/*void HAL_Time_Sleep_MicroSeconds( UINT32 uSec )
{
	g_Time_Driver.Sleep_uSec(uSec);
}*/

/*void HAL_Time_Sleep_MicroSeconds_InterruptEnabled( UINT32 uSec )
{
	g_Time_Driver.Sleep_uSec(uSec);
}*/

void HAL_Time_AddClockTime(UINT64 timeToAdd){
	g_Time_Driver.AddClockTime(timeToAdd);
}

