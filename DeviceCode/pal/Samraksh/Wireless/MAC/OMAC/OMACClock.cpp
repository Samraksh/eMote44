/*
 * OMACClock.cpp
 *
 *  Created on: Sep 5, 2019
 *      Author: mukun
 */

#include <Samraksh/MAC/OMAC/OMACClock.h>
#include <Samraksh/MAC/OMAC/OMACConstants.h>

#define DEBUG_TSYNC_PIN

OMACClock::OMACClock(){
//		m_inter_clock_offset = 0;
		m_first_clock_reading = HAL_Time_CurrentTicks();
}

UINT64 OMACClock::GetCurrentTimeinTicks(){ //This function gets the time ticks required for OMAC
	if(OMACClockSpecifier == LFCLOCKID){
		return VirtTimer_GetTicks(VIRT_TIMER_OMAC_SCHEDULER) * OMACClocktoSystemClockFreqRatio - m_first_clock_reading; //+ m_inter_clock_offset;
		///return VirtTimer_GetTicks(VIRT_TIMER_OMAC_SCHEDULER) - m_first_clock_reading;
	}
	else{
		return HAL_Time_CurrentTicks() - m_first_clock_reading;
	}
};
UINT64 OMACClock::ConvertTickstoMicroSecs(const UINT64& ticks){ //This function gets the time ticks required for OMAC
	///if(OMACClockSpecifier == LFCLOCKID){
	///	//return VirtTimer_GetTicks(VIRT_TIMER_OMAC_SCHEDULER) * OMACClocktoSystemClockFreqRatio;
	///	return ticks*30.517578125; // JH: LSE clock (32.768Khz) => 1000000us / 32768Hz
	///}
	///else {
		return ticks/50; //TICKS_PER_MICRO
	///}
};
UINT64 OMACClock::ConvertMicroSecstoTicks(const UINT64& microsecs){ //This function gets the time ticks required for OMAC
	///if(OMACClockSpecifier == LFCLOCKID){
		//return VirtTimer_GetTicks(VIRT_TIMER_OMAC_SCHEDULER) * OMACClocktoSystemClockFreqRatio;
	///	return ( microsecs * (1<<15))/ 1000000;	
	///}
	///else {
		return microsecs*50; //TICKS_PER_MICRO
	///}
};
UINT64 OMACClock::ConvertSlotstoTicks(const UINT64& slots){ //This function gets the time ticks required for OMAC
	///if(OMACClockSpecifier == LFCLOCKID){
	///	//return VirtTimer_GetTicks(VIRT_TIMER_OMAC_SCHEDULER) * OMACClocktoSystemClockFreqRatio;
	///	return ( slots * 16 * MILLISECINMICSEC * (1<<15))/ 1000000; //JH: SLOT PERIOD is 16 in SLOT_PERIOD_MILLI 	
	///}
	///else {
		return slots*4000000; //SLOT_PERIOD_TICKS
	///}
}

void OMACClock::CreateSyncPointBetweenClocks(){
//		if(0 && OMACClockSpecifier == LFCLOCKID){
//			m_inter_clock_offset = HAL_Time_CurrentTicks() - VirtTimer_GetTicks(VIRT_TIMER_OMAC_SCHEDULER) * OMACClocktoSystemClockFreqRatio ;
//		}
}

UINT64 OMACClock::AddTicks(const UINT64& ticks1, const UINT64& ticks2){
	return (ticks1 + ticks2);
}

UINT64 OMACClock::SubstractTicks(const UINT64& ticks1, const UINT64& ticks2){
	ASSERT(ticks1 >= ticks2);
	return ( (ticks1 > ticks2) ? (ticks1 - ticks2) : 0 );
}

UINT64 OMACClock::SubstractMicroSeconds(const UINT64& ticks1, const UINT64& ticks2){
	ASSERT(ticks1 >= ticks2);
	return ( (ticks1 > ticks2) ? (ticks1 - ticks2) : 0);
}

UINT64 OMACClock::AddMicroSeconds(const UINT64& ticks1, const UINT64& ticks2){
	return (ticks1 + ticks2);
}
