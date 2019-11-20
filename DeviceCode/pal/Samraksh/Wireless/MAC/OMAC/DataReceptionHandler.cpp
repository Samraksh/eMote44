/*
 * DataReceptionHandler.cpp
 *
 *  Modified on: Oct 30, 2015
 *  	Authors: Bora Karaoglu; Ananth Muralidharan
 *
 *  Copyright The Samraksh Company
 */

#include <Samraksh/MAC/OMAC/DataReceptionHandler.h>
#include <Samraksh/MAC/OMAC/OMAC.h>

#define OMAC_DRH_DEBUG_UNEXPECTED_POST_EXECUTE 0
extern OMACType g_OMAC;


void PublicDataRxCallback(void * param){
#if OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
	VirtualTimerReturnMessage rm;
	g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
	g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_curTime_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	if(g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_scheduledTimer_in_ticks > g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_curTime_in_ticks){ //Check for early firing from the timer
		//		hal_printf("DTH::EARLY FIRING PEE!! m_scheduledTimer_in_ticks = %llu, m_curTime_in_ticks = %llu",m_scheduledTimer_in_ticks,m_curTime_in_ticks  );
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
		if(rm != TimerSupported) {
			SOFT_BREAKPOINT();
		}
		UINT64 rem_time_micros = g_OMAC.m_Clock.ConvertTickstoMicroSecs( g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_scheduledTimer_in_ticks - g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_curTime_in_ticks);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, rem_time_micros, TRUE, OMACClockSpecifier );
		if(rm != TimerSupported) {
			SOFT_BREAKPOINT();
		}
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
		if(rm != TimerSupported) {
			SOFT_BREAKPOINT();
		}
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = true;
	}
	else{
#endif

#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, FALSE );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, TRUE );
#endif
		if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK){
			if(g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_receptionstate==DRS_RX_END){
				g_OMAC.m_omac_scheduler.m_DataReceptionHandler.SendDataACK();
			}
			else{
				g_OMAC.m_omac_scheduler.m_DataReceptionHandler.PostExecuteEvent();
			}
		}
		else{
			g_OMAC.m_omac_scheduler.m_DataReceptionHandler.PostExecuteEvent();
		}
#if OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
	}
#endif
}
void PublicDataRxSendAckCallback(void * param){
	g_OMAC.m_omac_scheduler.m_DataReceptionHandler.SendDataACK();
}

/*
 *
 */
void DataReceptionHandler::Initialize(UINT8 radioID, UINT8 macID){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_EnableOutputPin(DATARECEPTION_SLOTPIN, TRUE);
	CPU_GPIO_EnableOutputPin(DATATX_NEXTEVENT, TRUE);


	CPU_GPIO_EnableOutputPin(DATARX_TIMING_ERROR_PIN_TOGGLER, TRUE);
	CPU_GPIO_SetPinState( DATARX_TIMING_ERROR_PIN_TOGGLER, FALSE );

	CPU_GPIO_EnableOutputPin(DATARECEPTION_RADIOONPIN, TRUE);
	CPU_GPIO_SetPinState( DATARECEPTION_RADIOONPIN, FALSE );


	CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, FALSE );
	CPU_GPIO_SetPinState( DATATX_NEXTEVENT, FALSE );
	CPU_GPIO_EnableOutputPin(OMAC_TX_DATAACK_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(DATA_RX_INTERRUPT_PIN, FALSE);
	CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, FALSE );
	CPU_GPIO_EnableOutputPin(DATA_RX_END_OF_RECEPTION, FALSE);
	CPU_GPIO_SetPinState( DATA_RX_END_OF_RECEPTION, FALSE );
	CPU_GPIO_EnableOutputPin(DATARX_NEXT_EVENT, TRUE);
	CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, FALSE );
	CPU_GPIO_EnableOutputPin(DATARX_SEND_SW_ACK, TRUE);
	CPU_GPIO_SetPinState( DATARX_SEND_SW_ACK, FALSE );
	CPU_GPIO_EnableOutputPin(DATARX_HANDLE_END_OF_RX, TRUE);
	CPU_GPIO_SetPinState( DATARX_HANDLE_END_OF_RX, FALSE );
	CPU_GPIO_EnableOutputPin(DATARX_EXEC_EVENT, TRUE);
	CPU_GPIO_SetPinState( DATARX_EXEC_EVENT, FALSE );
		
	
#endif
	RadioID = radioID;
	MacID = macID;
	m_nextwakeupSlot = 0;
	m_seedUpdateIntervalinSlots = SEED_UPDATE_INTERVAL_IN_SLOTS;

	m_mask = 137 * 29 * (g_OMAC.GetMyAddress() + 1);
	m_nextSeed = 119 * 119 * (g_OMAC.GetMyAddress() + 1); // The initial seed
	m_nextwakeupSlot = g_OMAC.m_omac_scheduler.m_seedGenerator.RandWithMask(&m_nextSeed, m_mask) % m_seedUpdateIntervalinSlots;
	UpdateSeedandCalculateWakeupSlot(m_nextwakeupSlot, m_nextSeed, m_mask, m_seedUpdateIntervalinSlots, g_OMAC.m_omac_scheduler.GetSlotNumber() );

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_RECEIVER, 0, g_OMAC.LISTEN_PERIOD_FOR_RECEPTION_HANDLER/2 , TRUE, FALSE, PublicDataRxCallback, OMACClockSpecifier); //1 sec Timer in micro seconds
	if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK){
		rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_RECEIVER_ACK, 0, 0 , TRUE, FALSE, PublicDataRxSendAckCallback, OMACClockSpecifier); //1 sec Timer in micro seconds
	}

	//ASSERT_SP(rm == TimerSupported);

	// m_isreceiving = false;
	m_receptionstate = DRS_TX_POST_EXECUTE;

}

UINT64 DataReceptionHandler::NextEvent(){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, TRUE );
#endif
	UINT64 NextEventTimeinTicks;
	UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	UINT64 currentSlotNum = g_OMAC.m_omac_scheduler.GetSlotNumber();
	if ( currentSlotNum >= m_nextwakeupSlot ){ //Check for seed update
		UpdateSeedandCalculateWakeupSlot(m_nextwakeupSlot, m_nextSeed, m_mask, m_seedUpdateIntervalinSlots,  currentSlotNum );
	}
#if OMAC_DRXH_DEBUG_LATEWAKEUP
	m_scheduledRXTime_in_own_clock_ticks = g_OMAC.m_Clock.ConvertSlotstoTicks(m_nextwakeupSlot);
#endif
	NextEventTimeinTicks = g_OMAC.m_Clock.SubstractTicks(g_OMAC.m_Clock.ConvertSlotstoTicks(m_nextwakeupSlot) ,  g_OMAC.m_Clock.ConvertMicroSecstoTicks(g_OMAC.RADIO_TURN_ON_DELAY_RX + GUARDTIME_MICRO + ADDITIONAL_TIMEADVANCE_FOR_RECEPTION));
	/*if(HARDWARE_ACKS){
		NextEventTimeinTicks -= (EXTENDED_MODE_TX_DELAY_MICRO * TICKS_PER_MICRO);
		ASSERT_SP(NextEventTimeinTicks > 0);
	}*/

	while(NextEventTimeinTicks  <= y + OMAC_SCHEDULER_MIN_REACTION_TIME_IN_TICKS){
		currentSlotNum = m_nextwakeupSlot;
		UpdateSeedandCalculateWakeupSlot(m_nextwakeupSlot, m_nextSeed, m_mask, m_seedUpdateIntervalinSlots,  currentSlotNum );
		NextEventTimeinTicks = g_OMAC.m_Clock.SubstractTicks(g_OMAC.m_Clock.ConvertSlotstoTicks(m_nextwakeupSlot) ,  g_OMAC.m_Clock.ConvertMicroSecstoTicks(g_OMAC.RADIO_TURN_ON_DELAY_RX + GUARDTIME_MICRO + ADDITIONAL_TIMEADVANCE_FOR_RECEPTION));
	}
	UINT64 TicksTillNextEvent = NextEventTimeinTicks - g_OMAC.m_Clock.GetCurrentTimeinTicks();
	//ASSERT_SP(NextEventTimeinTicks > y);

	UINT64 nextEventsMicroSec = (g_OMAC.m_Clock.ConvertTickstoMicroSecs(TicksTillNextEvent)) ;
	UINT64 curTicks = g_OMAC.m_Clock.GetCurrentTimeinTicks();

#ifdef def_Neighbor2beFollowed
#ifdef OMAC_DEBUG_PRINTF
	//OMAC_HAL_PRINTF("DataReceptionHandler::NextEvent curTicks: %llu; NextEventTimeinTicks: %llu; m_nextwakeupSlot: %lu; TicksTillNextEvent: %llu; nextEventsMicroSec: %llu\r\n", curTicks, NextEventTimeinTicks, m_nextwakeupSlot, TicksTillNextEvent, nextEventsMicroSec);
	OMAC_HAL_PRINTF("\r\n[LT: %llu - %lu NT: %llu - %lu] DataReceptionHandler::NextEvent() nextWakeupTimeInMicSec = %llu AbsnextWakeupTimeInMicSec= %llu - %lu \r\n", g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks), g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(curTicks), g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks)), g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks)), nextEventsMicroSec, g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks)+nextEventsMicroSec, (g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks)+nextEventsMicroSec)/SLOT_PERIOD_MILLI/MILLISECINMICSEC );
#endif
#endif

//#ifdef OMAC_DEBUG_PRINTF
	//OMAC_HAL_PRINTF("DataReceptionHandler::NextEvent curTicks: %llu; NextEventTimeinTicks: %llu; m_nextwakeupSlot: %lu; TicksTillNextEvent: %llu; nextEventsMicroSec: %llu\r\n", curTicks, NextEventTimeinTicks, m_nextwakeupSlot, TicksTillNextEvent, nextEventsMicroSec);
	//OMAC_HAL_PRINTF("\r\n[LT: %s - %s NT: %s - %s] DataReceptionHandler::NextEvent() nextWakeupTimeInMicSec = %s AbsnextWakeupTimeInMicSec= %s - %s \r\n",g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks), g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(curTicks), g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks)), g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks)), nextEventsMicroSec, g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks)+nextEventsMicroSec, (g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks)+nextEventsMicroSec)/SLOT_PERIOD_MILLI/MILLISECINMICSEC );
//#endif

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, FALSE );
#endif
	return(nextEventsMicroSec);
}

bool DataReceptionHandler::UpdateSeedandCalculateWakeupSlot(UINT64 &wakeupSlot, UINT16 &next_seed, const UINT16 &mask, const UINT32 &seedUpdateIntervalinSlots,  const UINT64 &currentSlotNum ){
	UINT8  iternum = 0;
	if (currentSlotNum >= wakeupSlot){
		UINT16 randVal;
		UINT64 curFrameStart = wakeupSlot - wakeupSlot % seedUpdateIntervalinSlots;
		if(currentSlotNum > curFrameStart &&   currentSlotNum - curFrameStart > MAXUPDATESEEDITERS * seedUpdateIntervalinSlots){ //BK: Prevent infinite loop. We may as well act on this signal and declare the neighbor as dead.
			return false;
		}
		while ( currentSlotNum >= wakeupSlot ){
			//TODO: BK: The following does not seem to work. For now we are bypassing this by having a constant.
			randVal = g_OMAC.m_omac_scheduler.m_seedGenerator.RandWithMask(&next_seed, mask);
			curFrameStart = curFrameStart + seedUpdateIntervalinSlots;
			wakeupSlot = curFrameStart + randVal % seedUpdateIntervalinSlots;
			++iternum;
		}
	}
#if OMAC_DRXH_DEBUG_LATEWAKEUP
	lastwakeupSlotUpdateTimeinTicks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
#endif
	return true;
	//ASSERT_SP(wakeupSlot  > currentSlotNum);
}
/*
 *
 */
void DataReceptionHandler::ExecuteEvent(){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, TRUE );
	CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, TRUE );
	CPU_GPIO_SetPinState( DATARX_EXEC_EVENT, TRUE );
#endif




#if OMAC_DRXH_DEBUG_LATEWAKEUP
	UINT64 expected_y = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks(g_OMAC.RADIO_TURN_ON_DELAY_RX+ GUARDTIME_MICRO + ADDITIONAL_TIMEADVANCE_FOR_RECEPTION);
	if(m_scheduledRXTime_in_own_clock_ticks < expected_y  ){
		if(expected_y  > m_scheduledRXTime_in_own_clock_ticks + OMAC_DRXH_DEBUG_LATEWAKEUP_ALLOWANCE_IN_TICKS){
			//hal_printf("\r\r\n OMAC_DRXH_DEBUG_LATEWAKEUP Before RxON LATE ERROR! scheduledRXTime_ticks = %llu , Cur Ticks = %llu, Error = %llu \r\r\n",m_scheduledRXTime_in_own_clock_ticks, expected_y, g_OMAC.m_Clock.ConvertTickstoMicroSecs( expected_y - m_scheduledRXTime_in_own_clock_ticks ) );
			hal_printf("\r\r\n DRXH Before RxON LATE WAKEUP ERROR! Error = %llu \r\r\n", g_OMAC.m_Clock.ConvertTickstoMicroSecs( expected_y - m_scheduledRXTime_in_own_clock_ticks ) );

		}
	}
	else {
		if( expected_y + OMAC_DRXH_DEBUG_LATEWAKEUP_ALLOWANCE_IN_TICKS < m_scheduledRXTime_in_own_clock_ticks ){
			hal_printf("\r\r\n DRXH Before RxON EARLY WAKEUP ERROR! Error = %llu \r\r\n", g_OMAC.m_Clock.ConvertTickstoMicroSecs( m_scheduledRXTime_in_own_clock_ticks - expected_y ) );
		}
	}
#endif

	VirtualTimerReturnMessage rm;
	// m_isreceiving = false;
	m_receptionstate = DRS_EXECUTE_START;
	//static int failureCount = 0;
	DeviceStatus e = DS_Fail;

	e = g_OMAC.m_omac_RadioControl.StartRx();
	
	if (e == DS_Success){

#if OMAC_WAKEUP_DEBUGGING_FOR_MF
		hal_printf("W\r\n");
#endif

#ifdef OMAC_DEBUG_GPIO

		if(DATARECEPTION_RADIOONPIN != DISABLED_PIN ){
			CPU_GPIO_SetPinState( DATARECEPTION_RADIOONPIN, !CPU_GPIO_GetPinState(DATARECEPTION_RADIOONPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_RADIOONPIN, !CPU_GPIO_GetPinState(DATARECEPTION_RADIOONPIN) );
		}

		CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, FALSE);
		CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, TRUE);


		CPU_GPIO_SetPinState( DATARX_EXEC_EVENT, FALSE );
		CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, TRUE );
		CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, FALSE );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, FALSE );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, TRUE );
		CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, FALSE );
#endif


#if OMAC_DRXH_DEBUG_LATEWAKEUP_PIN_TOGGLING
		if( 	expected_y  > m_scheduledRXTime_in_own_clock_ticks + OMAC_DRXH_DEBUG_LATEWAKEUP_ALLOWANCE_IN_TICKS
				||  expected_y + OMAC_DRXH_DEBUG_LATEWAKEUP_ALLOWANCE_IN_TICKS < m_scheduledRXTime_in_own_clock_ticks
		){
			if(DATARX_TIMING_ERROR_PIN_TOGGLER != DISABLED_PIN){
				CPU_GPIO_SetPinState( DATARX_TIMING_ERROR_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATARX_TIMING_ERROR_PIN_TOGGLER) );
				CPU_GPIO_SetPinState( DATARX_TIMING_ERROR_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATARX_TIMING_ERROR_PIN_TOGGLER) );
			}
		}

#endif



#if OMAC_DRXH_DEBUG_LATEWAKEUP
		expected_y = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( GUARDTIME_MICRO + ADDITIONAL_TIMEADVANCE_FOR_RECEPTION);
		if(m_scheduledRXTime_in_own_clock_ticks < expected_y  ){
			if(expected_y  > m_scheduledRXTime_in_own_clock_ticks + OMAC_DRXH_DEBUG_LATEWAKEUP_ALLOWANCE_IN_TICKS){
				hal_printf("\r\r\n DRXH After RxON LATE ERROR! Error = %llu \r\r\n", g_OMAC.m_Clock.ConvertTickstoMicroSecs( expected_y - m_scheduledRXTime_in_own_clock_ticks) );
			}
		}
		else {
			if( expected_y + OMAC_DRXH_DEBUG_LATEWAKEUP_ALLOWANCE_IN_TICKS < m_scheduledRXTime_in_own_clock_ticks ){
				hal_printf("\r\r\n DRXH After RxON EARLY ERROR! Error = %llu \r\r\n", g_OMAC.m_Clock.ConvertTickstoMicroSecs( m_scheduledRXTime_in_own_clock_ticks - expected_y ));
			}
		}
#endif

		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, g_OMAC.LISTEN_PERIOD_FOR_RECEPTION_HANDLER/2, TRUE, OMACClockSpecifier );
#if	OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( g_OMAC.LISTEN_PERIOD_FOR_RECEPTION_HANDLER);
#endif
#if OMAC_DRXH_DEBUG_LATEWAKEUP
		m_lastScheduledOriginTime = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		m_lastScheduledTargetTime = m_lastScheduledOriginTime + 8 * g_OMAC.LISTEN_PERIOD_FOR_RECEPTION_HANDLER;
#endif
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = true;

	}
	else{
		m_receptionstate = DRS_RADIO_START_FAIl;
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("DataReceptionHandler::ExecuteEvent Could not turn on Rx\r\n");
#endif
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATARX_EXEC_EVENT, FALSE );
		CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, FALSE );
		CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, TRUE);
		CPU_GPIO_SetPinState( DATARX_NEXT_EVENT, FALSE );
#endif
		/*failureCount++;
		if(failureCount > 5){
			//ASSERT_SP(0);
		}*/
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, 100, TRUE,OMACClockSpecifier);
#if	OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 100);
#endif
#if OMAC_DRXH_DEBUG_LATEWAKEUP
		m_lastScheduledOriginTime = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		m_lastScheduledTargetTime = m_lastScheduledOriginTime + 0;
#endif
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = true;

		//PostExecuteEvent();
	}
}

void DataReceptionHandler::HandleRadioInterrupt(){ // This is the beginning of a reception
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, FALSE);
	CPU_GPIO_SetPinState(SCHED_RX_EXEC_PIN, TRUE);
#endif
	VirtualTimerReturnMessage rm;
	// m_isreceiving = true;
	//ASSERT_SP(m_receptionstate == DRS_EXECUTE_START);
	if(m_receptionstate != DRS_EXECUTE_START){
		return;
	}
	m_receptionstate = DRS_RX_START;
	rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
	g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, TRUE );
#endif
	if(rm != TimerSupported){
		/*CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, FALSE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, TRUE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, FALSE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, TRUE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, FALSE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, TRUE );*/
	}
	rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, g_OMAC.MAX_PACKET_RX_DURATION_MICRO/2, TRUE, OMACClockSpecifier );
#if	OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
	m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( g_OMAC.MAX_PACKET_RX_DURATION_MICRO);
#endif

#if OMAC_DRXH_DEBUG_LATEWAKEUP
	m_lastScheduledOriginTime = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	m_lastScheduledTargetTime = m_lastScheduledOriginTime + 8 * g_OMAC.MAX_PACKET_RX_DURATION_MICRO;
#endif

	if(rm != TimerSupported){
		/*CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, FALSE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, TRUE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, FALSE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, TRUE );*/
	}
	rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
	if(rm != TimerSupported){
		/*CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, FALSE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, TRUE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, FALSE );
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, TRUE );*/
	}
	g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = true;

}

void DataReceptionHandler::SendACKHandler(){ // Handler for end of tranmission interupt from the radio
	if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK) { //Only happens in SOFTWARE_ACK case
		VirtualTimerReturnMessage rm;
		// m_isreceiving = false;
		//ASSERT_SP(m_receptionstate == DRS_TX_START);
#ifdef OMAC_DEBUG_GPIO		
		CPU_GPIO_SetPinState( DATATX_SendACKHandler_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_SendACKHandler_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DATATX_SendACKHandler_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_SendACKHandler_PIN_TOGGLER) );
#endif
		if(m_receptionstate != DRS_TX_START){
			return;
		}
		m_receptionstate = DRS_TX_END;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, 0, TRUE, OMACClockSpecifier );
#if	OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
#if OMAC_DRXH_DEBUG_LATEWAKEUP
		m_lastScheduledOriginTime = g_OMAC.m_Clock.GetCurrentTimeinTicks();//HAL_Time_CurrentTicks();
		m_lastScheduledTargetTime = m_lastScheduledOriginTime + 0;
#endif
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = true;
#ifdef OMAC_DEBUG_GPIO		
		CPU_GPIO_SetPinState( DATATX_SendACKHandler_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_SendACKHandler_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DATATX_SendACKHandler_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_SendACKHandler_PIN_TOGGLER) );
#endif
	}
}

void DataReceptionHandler::HandleEndofReception(UINT16 address){
	if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK){
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATARX_HANDLE_END_OF_RX, TRUE );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
#endif

		VirtualTimerReturnMessage rm;
		// m_isreceiving = false;
		//ASSERT_SP(m_receptionstate == 1);
		if(m_receptionstate != DRS_RX_START && m_receptionstate != DRS_EXECUTE_START){
			return;
		}
		m_receptionstate = DRS_RX_END;
		m_lastRXNodeId = address;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
		//rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, 0, TRUE, OMACClockSpecifier );
		//rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);		//This runs in continuation context. Enable this line or next.
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER_ACK);		//This runs in interrupt context
		if(rm != TimerSupported){
			this->SendDataACK();
		}
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATARX_HANDLE_END_OF_RX, FALSE );

		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
#endif
	}
	else if(CPU_Radio_GetRadioAckType() == HARDWARE_ACK) {
		VirtualTimerReturnMessage rm;
		// m_isreceiving = false;
		//ASSERT_SP(m_receptionstate == 1);
		m_receptionstate = DRS_RX_END;
		m_lastRXNodeId = address;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, 0, TRUE, OMACClockSpecifier );
#if	OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
		if(rm != TimerSupported){
			this->SendDataACK();
		}
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = true;

	}
	else if(CPU_Radio_GetRadioAckType() == NO_ACK) {
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATA_RX_INTERRUPT_PIN, FALSE );
		CPU_GPIO_SetPinState( DATA_RX_END_OF_RECEPTION, TRUE );
#endif
		VirtualTimerReturnMessage rm;
		// m_isreceiving = false;
		//ASSERT_SP(m_receptionstate == 1);
		m_receptionstate = DRS_RX_END;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, 0, TRUE, OMACClockSpecifier );
#if	OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
#if OMAC_DRXH_DEBUG_LATEWAKEUP
		m_lastScheduledOriginTime = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		m_lastScheduledTargetTime = m_lastScheduledOriginTime + 0;
#endif
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = true;

#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATA_RX_END_OF_RECEPTION, FALSE );
#endif
	}
}



void DataReceptionHandler::SendDataACK(){ // This prepares a software ACK packet and sends it to the radiocontroller
	VirtualTimerReturnMessage rm;
	//ASSERT_SP(m_receptionstate == DRS_RX_END);
	if(m_receptionstate != DRS_RX_END){
		return;
	}
	m_receptionstate = DRS_TX_START;
	// m_isreceiving = false;

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(OMAC_TX_DATAACK_PIN, TRUE);
	CPU_GPIO_SetPinState(DATARX_SEND_SW_ACK, TRUE);

	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif



	//IEEE802_15_4_Header_t* header = m_ACKmsg.GetHeader();
	/****** Taking the word value of below bits gives FCF_WORD_VALUE *******/
	/*header->fcf->IEEE802_15_4_Header_FCF_BitValue.frameType = FRAME_TYPE_MAC;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.securityEnabled = 0;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.framePending = 0;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.ackRequired = 1;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.intraPAN = 1;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.reserved = 0;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.destAddrMode = 2;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.frameVersion = 1;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.srcAddrMode = 2;*/
	/**************************************************************/
	/*header->fcf.fcfWordValue = FCF_WORD_VALUE;
	header->dsn = 97;
	//header->srcpan = SRC_PAN_ID;
	header->destpan = DEST_PAN_ID;
	header->dest = m_lastRXNodeId;
	header->src = g_OMAC.GetMyAddress();
	//header->network = MyConfig.Network;
	header->macName = g_OMAC.macName;
	header->payloadType = MFM_OMAC_DATA_ACK;
	header->flags = (0);
	//header->retryAttempt = 0;
	m_ACKmsg.GetMetaData()->SetReceiveTimeStamp((UINT32)0);
	UINT8* payload = m_ACKmsg.GetPayload();
	 *payload = 66;
	header->length = sizeof(IEEE802_15_4_Header_t) + 1;
	g_OMAC.m_omac_RadioControl.Send(m_lastRXNodeId, &m_ACKmsg, header->length);*/

	static int i = 0;
	static softwareACKHeader softwareAckHeader;
	if(i == 0){
		softwareAckHeader.src = g_OMAC.GetMyAddress();
		softwareAckHeader.payloadType = MFM_OMAC_DATA_ACK;
		i++;
	}
	softwareAckHeader.dest = m_lastRXNodeId;
	#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("DataReceptionHandler::Send RX\r\n");
	#endif
	DeviceStatus rs = g_OMAC.m_omac_RadioControl.Send(m_lastRXNodeId, (Message_15_4_t*)&softwareAckHeader, sizeof(softwareACKHeader));
	if(rs != DS_Success){
		m_receptionstate = DRS_TX_FAIL;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, 0, TRUE,OMACClockSpecifier );
#if	OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
#if OMAC_DRXH_DEBUG_LATEWAKEUP
		m_lastScheduledOriginTime = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		m_lastScheduledTargetTime = m_lastScheduledOriginTime + g_OMAC.ACK_TX_MAX_DURATION_MICRO * 8;
#endif
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = true;

	}
	else{
		m_receptionstate = DRS_TX_START;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, g_OMAC.ACK_TX_MAX_DURATION_MICRO/2, TRUE,OMACClockSpecifier );
#if	OMAC_DRH_TIMER_TARGET_TIME_CORRECTION
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( g_OMAC.ACK_TX_MAX_DURATION_MICRO);
#endif
#if OMAC_DRXH_DEBUG_LATEWAKEUP
		m_lastScheduledOriginTime = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		m_lastScheduledTargetTime = m_lastScheduledOriginTime + g_OMAC.ACK_TX_MAX_DURATION_MICRO * 8;
#endif
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = true;

	}

#if OMAC_DEBUG_DRH_SEND_ACK
	hal_printf("DRH:SDA dest=%u src=%u rs = %u \r\n ", m_lastRXNodeId, g_OMAC.GetMyAddress(), rs);
#endif

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(OMAC_TX_DATAACK_PIN, FALSE);
	CPU_GPIO_SetPinState(DATARX_SEND_SW_ACK, FALSE);
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif
}

void DataReceptionHandler::PostExecuteEvent(){
	VirtualTimerReturnMessage rm;
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, FALSE );
	CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, TRUE );
#endif
	//m_currtime = g_OMAC.m_Clock.GetCurrentTimeinTicks();

#if OMAC_DRH_DEBUG_UNEXPECTED_POST_EXECUTE
	if(!(m_receptionstate == DRS_EXECUTE_START || m_receptionstate == DRS_TX_END)){
		hal_printf("DRH:UEPE m_receptionstate = %u \r\n", m_receptionstate);

		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		// m_isreceiving = false;
	}
#endif

	m_receptionstate = DRS_TX_POST_EXECUTE;
	//Scheduler's PostExecution stops the radio
	DeviceStatus returnVal = DS_Success;


	
	returnVal = g_OMAC.m_omac_RadioControl.Stop();

#if OMAC_WAKEUP_DEBUGGING_FOR_MF
	hal_printf("Z\r\n");
#endif
	if(returnVal == DS_Success) {
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, FALSE );
#endif
		g_OMAC.m_omac_scheduler.PostExecution();
	}
	else{
		//rm = VirtTimer_Change(VIRT_TIMER_OMAC_RECEIVER, 0, RECEIVER_RADIO_STOP_RECHECK_INTERVAL_MICRO, TRUE, OMACClockSpecifier );
		//rm = VirtTimer_Start(VIRT_TIMER_OMAC_RECEIVER);
		g_OMAC.m_omac_scheduler.PostExecution();
	}
}

void DataReceptionHandler::FailsafeStop(){
	VirtTimer_Stop(VIRT_TIMER_OMAC_RECEIVER);
	g_OMAC.m_omac_scheduler.m_DataReceptionHandler.is_VIRT_TIMER_OMAC_RECEIVER_in_use = false;
	m_receptionstate = DRS_TX_POST_EXECUTE;
}