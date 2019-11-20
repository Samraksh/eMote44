/*
 * Scheduler.cpp
 *
 *  Created on: Oct 8, 2015
 *      Author: Bora Karaoglu
 *
 *  Copyright The Samraksh Company
 */

#include <Samraksh/MAC/OMAC/Scheduler.h>
#include <Samraksh/MAC/OMAC/OMAC.h>
#include <Samraksh/MAC/OMAC/OMACConstants.h>

#define SOFT_BREAKPOINT() ASSERT(0)

extern OMACType g_OMAC;

void PublicPostExecutionTaskHandler1(void * param){

	g_OMAC.m_omac_scheduler.PostExecution();
}

void PublicSchedulerTaskHandlerFailsafe(void * param){
	g_OMAC.m_omac_scheduler.FailsafeStop();
}


void PublicSchedulerTaskHandlerRadioStop(void * param){
	g_OMAC.m_omac_scheduler.PostExecution();
}

void PublicSchedulerTaskHandler1(void * param){

#if OMAC_SCHEDULER_TIMER_TARGET_TIME_CORRECTION
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(VIRT_TIMER_OMAC_SCHEDULER);
	UINT64 rem_time_micros;
	g_OMAC.m_omac_scheduler.m_curTime_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	if(g_OMAC.m_omac_scheduler.m_scheduledTimer_in_ticks > g_OMAC.m_omac_scheduler.m_curTime_in_ticks){ //Check for early firing from the timer
		g_OMAC.m_omac_scheduler.m_num_rescheduled = g_OMAC.m_omac_scheduler.m_num_rescheduled + 1;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_SCHEDULER);
		if(rm != TimerSupported) {
			SOFT_BREAKPOINT();
		}
		rem_time_micros = g_OMAC.m_Clock.ConvertTickstoMicroSecs( g_OMAC.m_omac_scheduler.m_scheduledTimer_in_ticks - g_OMAC.m_omac_scheduler.m_curTime_in_ticks);
		if(rem_time_micros < OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO) rem_time_micros = OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO;
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_SCHEDULER, 0, rem_time_micros, TRUE, OMACClockSpecifier );
		if(rm != TimerSupported) {
			SOFT_BREAKPOINT();
		}
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_SCHEDULER);
		if(rm != TimerSupported) {
			SOFT_BREAKPOINT();
		}

	}
	else{
#endif
		g_OMAC.m_omac_scheduler.RunEventTask();
#if OMAC_SCHEDULER_TIMER_TARGET_TIME_CORRECTION
	}
#endif
}

void OMACScheduler::Initialize(UINT8 _radioID, UINT8 _macID){

	m_num_FailsafeStop = 0;

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_EnableOutputPin(SCHED_START_STOP_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(SCHED_RX_EXEC_PIN, FALSE);
	CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(SCHED_TX_EXEC_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(SCHED_DISCO_EXEC_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(SCHED_TSREQ_EXEC_PIN, FALSE);
	CPU_GPIO_SetPinState(SCHED_TSREQ_EXEC_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(SCHED_NEXT_EVENT, FALSE);
	CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, FALSE);
	CPU_GPIO_EnableOutputPin(OMAC_SCHED_POST_POST_EXEC, FALSE);
	CPU_GPIO_SetPinState(OMAC_SCHED_POST_POST_EXEC, FALSE);	
	
	CPU_GPIO_EnableOutputPin( RX_RADIO_TURN_ON, TRUE);
	CPU_GPIO_SetPinState( RX_RADIO_TURN_ON, FALSE );
	CPU_GPIO_EnableOutputPin( RX_RADIO_TURN_OFF, TRUE);
	CPU_GPIO_SetPinState( RX_RADIO_TURN_OFF, FALSE );

	
#endif

	radioID = _radioID;
	macID = _macID;

#ifdef PROFILING
	minStartDelay = 300; maxStartDelay = 10;
	minStopDelay = 300; maxStopDelay = 10;
#endif

	m_state = I_IDLE;
	m_execution_started = false;
	m_num_sleep_retry_attempts = 0;
	//Initialize the HAL vitual timer layer

	bool rv = VirtTimer_Initialize();
	//ASSERT_SP(rv);
	VirtualTimerReturnMessage rm;

	rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE, 0, FAILSAFETIME_MICRO, FALSE, FALSE, PublicSchedulerTaskHandlerFailsafe, OMACClockSpecifier);
	rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_SCHEDULER_RADIO_STOP_RETRY, 0, RADIO_STOP_RETRY_PERIOD_IN_MICS, TRUE, FALSE, PublicSchedulerTaskHandlerRadioStop, OMACClockSpecifier);
	rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_SCHEDULER, 0, SLOT_PERIOD_MILLI * MILLISECINMICSEC, TRUE, FALSE, PublicSchedulerTaskHandler1, OMACClockSpecifier);
	//ASSERT_SP(rm == TimerSupported);

	//Initialize Handlers
	m_DiscoveryHandler.Initialize(radioID, macID);
	m_DataReceptionHandler.Initialize(radioID, macID);
	m_DataTransmissionHandler.Initialize();
	m_TimeSyncHandler.Initialize(radioID, macID);

	//m_InitializationTimeinTicks = g_OMAC.m_Clock.GetCurrentTimeinTicks();

	ScheduleNextEvent();

	m_num_FailsafeTimerFiring = 0;
	m_scheduledFailSafeTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( FAILSAFETIME_MICRO);
	rm = VirtTimer_Start(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE);
}

void OMACScheduler::UnInitialize(){
	m_TimeSyncHandler.FailsafeStop();
	m_DataTransmissionHandler.FailsafeStop();
	m_DataReceptionHandler.FailsafeStop();
	m_DiscoveryHandler.FailsafeStop();

	// FIXME: alert other nodes?
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(VIRT_TIMER_OMAC_SCHEDULER);
	rm = VirtTimer_Stop(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE);
}



UINT64 OMACScheduler::GetSlotNumber(){
	UINT64 currentTicks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	UINT64 slotNumber = currentTicks / SLOT_PERIOD_TICKS;
	return slotNumber;
}

UINT64 OMACScheduler::GetSlotNumberfromTicks(const UINT64 &y){
	return ( y / SLOT_PERIOD_TICKS);
}

UINT32 OMACScheduler::GetSlotNumberfromMicroSec(const UINT64 &y){
	return ( y / MILLISECINMICSEC / SLOT_PERIOD_MILLI  );
}

UINT32 OMACScheduler::GetTimeTillTheEndofSlot(){
	UINT64 cur_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	UINT64 ticks_till_end = SLOT_PERIOD_TICKS - ( (cur_ticks + SLOT_PERIOD_TICKS) % SLOT_PERIOD_TICKS);
	UINT32 ms_till_end = ((UINT32) ticks_till_end) / (TICKS_PER_MILLI / MILLISECINMICSEC ) ;
	return ms_till_end;
}

/* ScheduleNextEvent
 *
 */
void OMACScheduler::ScheduleNextEvent(){
	if(g_OMAC.UpdateNeighborTable() > 0 ){//If there are neighbor deleted from the table increase the discovery rate
		//m_DiscoveryHandler.TempIncreaseDiscoRate();
	}
	else if(g_NeighborTable.NumberOfNeighbors() == 0){//If there are no neighbors stay in high disco mode.
		//m_DiscoveryHandler.TempIncreaseDiscoRate();
	}

	VirtualTimerReturnMessage rm;

	UINT64 rxEventOffset = 0, txEventOffset = 0, beaconEventOffset = 0, timeSyncEventOffset=0, rxEventOffsetAdjust, txEventOffsetAdjust;
	UINT64 curticks_rx, curticks_tx, curticks_beacon, curticks_timesync, curticks;
#if OMAC_SCEHDULER_EASY_DEBUG
	nextWakeupTimeInMicSec = MAXSCHEDULERUPDATE;
#else
	UINT64 nextWakeupTimeInMicSec = MAXSCHEDULERUPDATE;
#endif

	timeSyncEventOffset = m_TimeSyncHandler.NextEvent();
	curticks_timesync =  g_OMAC.m_Clock.GetCurrentTimeinTicks();
	if (timeSyncEventOffset < OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO) timeSyncEventOffset = 0xffffffffffffffff;
	
	rxEventOffset = m_DataReceptionHandler.NextEvent();
	curticks_rx =  g_OMAC.m_Clock.GetCurrentTimeinTicks();
	if (rxEventOffset < OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO) rxEventOffset = 0xffffffffffffffff;
	//rxEventOffset = rxEventOffset-1;

	txEventOffset = m_DataTransmissionHandler.NextEvent();
	curticks_tx =  g_OMAC.m_Clock.GetCurrentTimeinTicks();
	if (txEventOffset < OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO) txEventOffset = 0xffffffffffffffff;
	//txEventOffset = txEventOffset-1;


	beaconEventOffset = m_DiscoveryHandler.NextEvent();
	curticks_beacon = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	if (beaconEventOffset < OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO) beaconEventOffset = 0xffffffffffffffff;
	//beaconEventOffset = beaconEventOffset -1;

	//Readjust: No time intensive tasks after this point
	curticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	rxEventOffset = rxEventOffset -  g_OMAC.m_Clock.ConvertTickstoMicroSecs(curticks - curticks_rx);
	txEventOffset = txEventOffset -  g_OMAC.m_Clock.ConvertTickstoMicroSecs(curticks - curticks_tx);

	if(rxEventOffset < MAXSCHEDULERUPDATE && rxEventOffset < nextWakeupTimeInMicSec) {
		nextWakeupTimeInMicSec  = rxEventOffset;
	}
	if(txEventOffset < MAXSCHEDULERUPDATE && txEventOffset < nextWakeupTimeInMicSec) {
		nextWakeupTimeInMicSec  = txEventOffset;
	}
	if(beaconEventOffset < MAXSCHEDULERUPDATE && beaconEventOffset + DISCO_SLOT_GUARD * SLOT_PERIOD_MILLI * MILLISECINMICSEC < nextWakeupTimeInMicSec) {
		beaconEventOffset = beaconEventOffset - g_OMAC.m_Clock.ConvertTickstoMicroSecs(curticks - curticks_beacon);
		nextWakeupTimeInMicSec  = beaconEventOffset;
	}
	//if(timeSyncEventOffset < MAXSCHEDULERUPDATE && timeSyncEventOffset < nextWakeupTimeInMicSec) {
	//	nextWakeupTimeInMicSec  = timeSyncEventOffset;
	//}

	hal_printf("\r rxEvent=%s\r\n", l2s(rxEventOffset,0));
	hal_printf("\r txEvent=%s\r\n", l2s(txEventOffset,0));
	hal_printf("\r beaconent=%s\r\n", l2s(beaconEventOffset,0));
	
	
	if(rxEventOffset == nextWakeupTimeInMicSec) {
		m_state = I_DATA_RCV_PENDING;
		hal_printf("\r RCV \r\n");
		//m_state = I_IDLE;
		//nextWakeupTimeInMicSec = nextWakeupTimeInMicSec - RADIO_TURN_ON_DELAY_MICRO - OMAC_HW_ACK_DELAY; //MMA
		//nextWakeupTimeInMicSec = nextWakeupTimeInMicSec - RADIO_TURN_ON_DELAY_MICRO; //BK: THis calculation is done inside the nextevent in order to prevent a negative value
	}
	else if(txEventOffset == nextWakeupTimeInMicSec) {
		//BK: Transmitter first turns the radio on and hence incurs that delay, then copies the packet on the SPI bus and incurs that delay.
		//TODO: BK: The PROCESSING_DELAY_BEFORE_TX_MICRO should depend on the packet size. We need to experiment and make it better. This will help balance out the guardband and take the bias out of it.
		//nextWakeupTimeInMicSec = nextWakeupTimeInMicSec - PROCESSING_DELAY_BEFORE_TX_MICRO - RADIO_TURN_ON_DELAY_MICRO - OMAC_HW_ACK_DELAY;	//MMA
		//nextWakeupTimeInMicSec = nextWakeupTimeInMicSec + GUARDTIME_MICRO + SWITCHING_DELAY_MICRO - PROCESSING_DELAY_BEFORE_TX_MICRO - RADIO_TURN_ON_DELAY_MICRO;//BK: THis calculation is done inside the nextevent in order to prevent a negative value
		hal_printf("\r SEND \r\n");

		m_state = I_DATA_SEND_PENDING;
		//m_state = I_IDLE;
	}
	else if(beaconEventOffset == nextWakeupTimeInMicSec) {
		//nextWakeupTimeInMicSec = nextWakeupTimeInMicSec - OMAC_HW_ACK_DELAY_MICRO;
		hal_printf("\r DISCO \r\n");
		m_state  = I_DISCO_PENDING;
	}
	else if(timeSyncEventOffset == nextWakeupTimeInMicSec) {
		//nextWakeupTimeInMicSec = nextWakeupTimeInMicSec - OMAC_HW_ACK_DELAY_MICRO;
		hal_printf("\r TimeSync \r\n");
#ifdef OMAC_DEBUG_GPIO		
		CPU_GPIO_SetPinState(SCHED_TSREQ_EXEC_PIN, TRUE);
		CPU_GPIO_SetPinState(SCHED_TSREQ_EXEC_PIN, FALSE);
#endif
		m_state = I_TIMESYNC_PENDING;
	}
	else{
		m_state = I_IDLE;
	}

#ifdef def_Neighbor2beFollowed
#ifdef OMAC_DEBUG_PRINTF
	UINT64 curTicks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
#ifdef OMAC_DEBUG_PRINTF
	hal_printf(" \r\n[LT: %llu - %lu NT: %llu - %lu] OMACScheduler::ScheduleNextEvent() nextWakeupTimeInMicSec= %llu AbsnextWakeupTimeInMicSec= %llu - %lu m_state.GetState() = %d  \r\n"
			, g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks), GetSlotNumberfromTicks(curTicks), m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks), GetSlotNumberfromTicks(m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks)), nextWakeupTimeInMicSec, g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks)+nextWakeupTimeInMicSec, GetSlotNumberfromMicroSec(g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks)+nextWakeupTimeInMicSec), m_state );
#endif
#endif
#endif
	//nextWakeupTimeInMicSec = nextWakeupTimeInMicSec - TIMER_EVENT_DELAY_OFFSET; //BK: There seems to be a constant delay in timers. This is to compansate for it.

	if(nextWakeupTimeInMicSec > MAXSCHEDULERUPDATE || nextWakeupTimeInMicSec < OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO ) {
		nextWakeupTimeInMicSec = MAXSCHEDULERUPDATE;
		m_state = I_IDLE ;
	}
	
    //JH: Currently, virtual timer is fired in time * 2
	rm = VirtTimer_Change(VIRT_TIMER_OMAC_SCHEDULER, 0, nextWakeupTimeInMicSec/2, TRUE, OMACClockSpecifier); //1 sec Timer in micro seconds
	//	
	//ASSERT_SP(rm == TimerSupported);
#if OMAC_SCHEDULER_TIMER_TARGET_TIME_CORRECTION
	m_curTime_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( nextWakeupTimeInMicSec);
	m_num_rescheduled = 0;
#endif
	rm = VirtTimer_Start(VIRT_TIMER_OMAC_SCHEDULER);
	while(rm != TimerSupported){
		hal_printf("CANNOT START VIRT_TIMER_OMAC_SCHEDULER");
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_SCHEDULER);
	}

   
	m_scheduledFailSafeTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( FAILSAFETIME_MICRO);
	SchedulerINUse = true;
	//ASSERT_SP(rm == TimerSupported);


	//
	//	BOOL* completionFlag = (BOOL*)false;
	//	// we assume only 1 can be active, abort previous just in case
	//	OMAC_scheduler_TimerCompletion.Abort();
	//	OMAC_scheduler_TimerCompletion.InitializeForISR(PublicSchedulerTaskHandler, completionFlag);
	//	//Enqueue a task to listen for messages 100 usec from now (almost immediately)
	//	//TODO (Ananth): to check what the right enqueue value should be
	//	OMAC_scheduler_TimerCompletion.EnqueueDelta(nextWakeupTimeInMicSec);
}

bool OMACScheduler::RunEventTask(){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( SCHED_START_STOP_PIN, TRUE );
	CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, TRUE);
#endif
	SchedulerINUse = false;
	if(m_state != I_RADIO_STOP_RETRY) m_num_sleep_retry_attempts = 0;
	VirtualTimerReturnMessage rm;
	//		rm = VirtTimer_Change(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE, 0, FAILSAFETIME_MICRO, TRUE, OMACClockSpecifier); //1 sec Timer in micro seconds
	//		if(rm == TimerSupported) rm = VirtTimer_Start(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE);

	m_scheduledFailSafeTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( FAILSAFETIME_MICRO);

	//g_OMAC.UpdateNeighborTable();
	UINT64 curTicks = g_OMAC.m_Clock.GetCurrentTimeinTicks();

#ifdef def_Neighbor2beFollowed
#ifdef OMAC_DEBUG_PRINTF
	hal_printf(" \r\n[LT: %llu - %lu NT: %llu - %lu] OMACScheduler::RunEventTask()  \r\n"
			, g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks), GetSlotNumber(), g_OMAC.m_Clock.ConvertTickstoMicroSecs(m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks)), GetSlotNumberfromTicks(m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks)) );
#endif
#endif
	m_execution_started = true;
	switch(m_state) {
	case I_DATA_SEND_PENDING:
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(SCHED_TX_EXEC_PIN, TRUE);
		CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, FALSE);
#endif
    

		m_lastHandler = DATA_TX_HANDLER;
		m_DataTransmissionHandler.ExecuteEvent();
		break;
	case I_DATA_RCV_PENDING:
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, TRUE);
		CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, FALSE);
		CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, TRUE);
		CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, FALSE);
#endif

  		m_lastHandler = DATA_RX_HANDLER;
		m_DataReceptionHandler.ExecuteEvent();
		break;
	case I_TIMESYNC_PENDING:
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(SCHED_TSREQ_EXEC_PIN, TRUE);
#endif

		m_lastHandler = TIMESYNC_HANDLER;
		m_TimeSyncHandler.ExecuteEvent();
		break;
	case I_DISCO_PENDING:

	
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, TRUE);
		CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, FALSE);
		CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, TRUE);
		CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, FALSE);
		CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, TRUE);
		CPU_GPIO_SetPinState(SCHED_NEXT_EVENT, FALSE);
#endif	

	
		m_DiscoveryHandler.ExecuteEvent();
		m_lastHandler = CONTROL_BEACON_HANDLER;
		break;
	default: //Case for
#ifdef OMAC_DEBUG_GPIO	
		CPU_GPIO_SetPinState(OMAC_SCHED_POST_POST_EXEC, TRUE);
#endif
		m_TimeSyncHandler.ExecuteEvent();
		//PostExecution();
#ifdef OMAC_DEBUG_GPIO		
		CPU_GPIO_SetPinState(OMAC_SCHED_POST_POST_EXEC, FALSE);
#endif
	}
	return true;
}

void OMACScheduler::PostExecution(){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( SCHED_START_STOP_PIN, FALSE );
	CPU_GPIO_SetPinState( SCHED_START_STOP_PIN, TRUE );
#endif

	m_execution_started = false;
	VirtualTimerReturnMessage rm = TimerNotSupported;
	//	rm = VirtTimer_Stop(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE);
	//		while (rm !=  TimerSupported){
	//			rm = VirtTimer_Stop(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE);
	//		}
	//	if(rm != TimerSupported) {
	//		SOFT_BREAKPOINT();
	//		return; //BK: Failsafe does not stop. let it fire and stop itself.
	//	}
	if(!EnsureStopRadio()){ //BK: Radio does not stop// Reschedule another stopping periof
		++m_num_sleep_retry_attempts;
#ifdef OMAC_DEBUG_PRINTF
		hal_printf(" \r\n OMACScheduler::PostExecution() Radio stop failure! m_num_sleep_retry_attempts = %u  \r\n", m_num_sleep_retry_attempts);
#endif
		if(m_num_sleep_retry_attempts < MAX_SLEEP_RETRY_ATTEMPTS){ //Schedule retrial
			m_state = I_RADIO_STOP_RETRY;


			//			rm = VirtTimer_Change(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE, 0, RADIO_STOP_RETRY_PERIOD_IN_MICS, TRUE, OMACClockSpecifier); //1 sec Timer in micro seconds
			if(rm == TimerSupported) rm = VirtTimer_Stop(VIRT_TIMER_OMAC_SCHEDULER_RADIO_STOP_RETRY);
			if(rm == TimerSupported) rm = VirtTimer_Start(VIRT_TIMER_OMAC_SCHEDULER_RADIO_STOP_RETRY);
			if(rm != TimerSupported){
				PostExecution();
			}
		}
		else{ // Radio does not stop after maximum number of stop trials
			m_state = I_POST_EXECUTE;
			ScheduleNextEvent();
		}

	}
	else{

		m_state = I_IDLE;
		//	rm = VirtTimer_Change(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE, 0, FAILSAFETIME_MICRO, TRUE, OMACClockSpecifier); //1 sec Timer in micro seconds

#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( SCHED_START_STOP_PIN, FALSE );
		CPU_GPIO_SetPinState( SCHED_START_STOP_PIN, TRUE );
#endif

		ScheduleNextEvent();

#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( SCHED_START_STOP_PIN, FALSE );
		CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, FALSE);
		CPU_GPIO_SetPinState(SCHED_TX_EXEC_PIN, FALSE);
		CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, FALSE);
		CPU_GPIO_SetPinState(SCHED_TSREQ_EXEC_PIN, FALSE);
#endif
	}
}

void OMACScheduler::FailsafeStop(){

	VirtualTimerReturnMessage rm;
	if(g_OMAC.m_Clock.GetCurrentTimeinTicks() > m_scheduledFailSafeTimer_in_ticks){
		++m_num_FailsafeTimerFiring;

#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(SCHED_START_STOP_PIN, !CPU_GPIO_GetPinState(SCHED_START_STOP_PIN));
		CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_RX_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_TX_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_TX_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_DISCO_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_TSREQ_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_TSREQ_EXEC_PIN));

		CPU_GPIO_SetPinState(SCHED_START_STOP_PIN, !CPU_GPIO_GetPinState(SCHED_START_STOP_PIN));
		CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_RX_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_TX_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_TX_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_DISCO_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_TSREQ_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_TSREQ_EXEC_PIN));

		CPU_GPIO_SetPinState(SCHED_START_STOP_PIN, !CPU_GPIO_GetPinState(SCHED_START_STOP_PIN));
		CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_RX_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_TX_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_TX_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_DISCO_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_TSREQ_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_TSREQ_EXEC_PIN));


		CPU_GPIO_SetPinState(SCHED_START_STOP_PIN, !CPU_GPIO_GetPinState(SCHED_START_STOP_PIN));
		CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_RX_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_TX_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_TX_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_DISCO_EXEC_PIN));
		CPU_GPIO_SetPinState(SCHED_TSREQ_EXEC_PIN, !CPU_GPIO_GetPinState(SCHED_TSREQ_EXEC_PIN));

#endif


#if OMAC_DEBUG_PRINTF_FAILSAFE_STOP
		hal_printf("FAILSAFE_STOP m_state = %u \r\n", m_state);
#endif
		bool rv = false;
		switch(m_state) {
		case I_DATA_SEND_PENDING:
			m_DataTransmissionHandler.FailsafeStop();
			PostExecution();
			break;
		case I_DATA_RCV_PENDING:
			m_DataReceptionHandler.FailsafeStop();
			PostExecution();
			break;
		case I_TIMESYNC_PENDING:
			m_TimeSyncHandler.FailsafeStop();
			PostExecution();
			break;
		case I_DISCO_PENDING:
			rv = m_DiscoveryHandler.FailsafeStop();
			if (!rv){
				//				rm = VirtTimer_Start(VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE);
				m_scheduledFailSafeTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( FAILSAFETIME_MICRO);

			}
			else{
				PostExecution();
			}
			break;
		case I_RADIO_STOP_RETRY:
			PostExecution();
			break;
		default: //Case for
			PostExecution();
			break;
		}
	}
}


bool OMACScheduler::EnsureStopRadio(){
	DeviceStatus  ds = DS_Success;
	ds = g_OMAC.m_omac_RadioControl.Stop();
	if (ds == DS_Success) {
		return TRUE;
	}
	else {//TODO: BK : This should be implemented in the case of radio not stopping
		return FALSE;
	}
}
