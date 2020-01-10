/*
 * DataTransmissionHandler.cpp
 *
 *  Created on: Sep 5, 2012
 *      Author: Mukundan Sridharan
 *
 *  Modified on: Oct 30, 2015
 *  	Authors: Bora Karaoglu; Ananth Muralidharan
 *
 *  Copyright The Samraksh Company
 */

//#include <Samraksh/Neighbors.h>
#include <Samraksh/MAC/OMAC/DataTransmissionHandler.h>
#include <Samraksh/MAC/OMAC/OMAC.h>


extern OMACType g_OMAC;

#if defined(OMAC_DEBUG_PRINTF) || defined(DEBUG_OMAC_UPDATENEIGHBORSWAKEUPSLOT)
#define DEBUG_OMAC_UNWUS_PRINTF(...) hal_printf( __VA_ARGS__ )
#else
#define DEBUG_OMAC_UNWUS_PRINTF(...) (void)0
#endif

#define OMAC_DTH_DEBUG_ReceiveDATAACK 0
#define OMAC_DTH_DEBUG_SendACKHandler 0

void PublicDataTxCallback(void * param){
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION
	VirtualTimerReturnMessage rm;
	g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.m_curTime_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	//	m_TimeDiff_in_micros = g_OMAC.m_Clock.ConvertTickstoMicroSecs(m_curTime_in_ticks - m_scheduledTimer_in_ticks);
	if(g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.m_scheduledTimer_in_ticks > g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.m_curTime_in_ticks){ //Check for early firing from the timer
		//		hal_printf("DTH::EARLY FIRING PEE!! m_scheduledTimer_in_ticks = %llu, m_curTime_in_ticks = %llu",m_scheduledTimer_in_ticks,m_curTime_in_ticks  );
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER);
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}
		UINT64 rem_time_micros = g_OMAC.m_Clock.ConvertTickstoMicroSecs( g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.m_scheduledTimer_in_ticks - g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.m_curTime_in_ticks);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, rem_time_micros, TRUE, OMACClockSpecifier );
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER);
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}
	}
	else{
#endif
		if(	FAST_RECOVERY) {
			g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.SendRetry();
		}
		else{
			g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.PostExecuteEvent();
		}
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION
	}
#endif
}

/*void PublicDataTxPostExecCallback(void * param){
	g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.PostExecuteEvent();
}*/

void PublicFastRecoveryCallback(void* param){
	g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.SendRetry();
}



/*
 *
 */
void DataTransmissionHandler::Initialize(){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_EnableOutputPin(DATATX_PIN, TRUE);
	CPU_GPIO_EnableOutputPin(DATATX_DATA_PIN, TRUE);
	CPU_GPIO_SetPinState( DATATX_DATA_PIN, FALSE );
	CPU_GPIO_EnableOutputPin(DATARX_NEXTEVENT, TRUE);
	CPU_GPIO_SetPinState( DATATX_PIN, FALSE );

	CPU_GPIO_EnableOutputPin(DATATX_TIMING_ERROR_PIN_TOGGLER, TRUE);
	CPU_GPIO_SetPinState( DATATX_TIMING_ERROR_PIN_TOGGLER, FALSE );

	CPU_GPIO_EnableOutputPin(DATATX_CCA_PIN_TOGGLER, TRUE);
	CPU_GPIO_SetPinState( DATATX_CCA_PIN_TOGGLER, FALSE );


	CPU_GPIO_EnableOutputPin(FAST_RECOVERY_SEND, TRUE);
	CPU_GPIO_SetPinState( FAST_RECOVERY_SEND, FALSE );
	CPU_GPIO_EnableOutputPin(DATATX_SEND_ACK_HANDLER, TRUE);
	CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
	//CPU_GPIO_EnableOutputPin(HW_ACK_PIN, TRUE);
	//CPU_GPIO_SetPinState( HW_ACK_PIN, FALSE );
	CPU_GPIO_SetPinState( DATARX_NEXTEVENT, FALSE );
	CPU_GPIO_EnableOutputPin(DATATX_POSTEXEC, TRUE);
	CPU_GPIO_SetPinState( DATATX_POSTEXEC, FALSE );
	CPU_GPIO_EnableOutputPin(DATATX_RECV_HW_ACK, TRUE);
	CPU_GPIO_SetPinState( DATATX_RECV_HW_ACK, FALSE );
	CPU_GPIO_EnableOutputPin(DATATX_SCHED_DATA_PKT, TRUE);
	CPU_GPIO_SetPinState( DATATX_SCHED_DATA_PKT, FALSE );

	CPU_GPIO_EnableOutputPin(DATATX_NEXT_EVENT, TRUE);
	CPU_GPIO_SetPinState( DATATX_NEXT_EVENT, FALSE );

	CPU_GPIO_EnableOutputPin(DATATX_RECV_SW_ACK, TRUE);
	CPU_GPIO_SetPinState( DATATX_RECV_SW_ACK, FALSE );

	CPU_GPIO_EnableOutputPin(OMAC_RX_DATAACK_PIN, FALSE);

	CPU_GPIO_EnableOutputPin(DTH_STATE_PIN_TOGGLER, TRUE);
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, FALSE );

	CPU_GPIO_EnableOutputPin(DATATX_SendACKHandler_PIN_TOGGLER, TRUE);
	CPU_GPIO_SetPinState( DATATX_SendACKHandler_PIN_TOGGLER, FALSE );
	

#endif
	m_curTime_in_ticks = 0;
	isDataPacketScheduled = false;
	m_currentSlotRetryAttempt = 0;
	m_RANDOM_BACKOFF = 0;
	m_backoff_mask = 137 * 29 * (g_OMAC.GetMyAddress() + 1);
	m_backoff_seed = 119 * 119 * (g_OMAC.GetMyAddress() + 1); // The initial seed
	//m_TXMsg = (DataMsg_t*)m_TXMsgBuffer.GetPayload() ;

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_TRANSMITTER, 0, g_OMAC.MAX_PACKET_TX_DURATION_MICRO, TRUE, FALSE, PublicDataTxCallback, OMACClockSpecifier); //1 sec Timer in micro seconds
	ASSERT_SP(rm == TimerSupported);
	//rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_TRANSMITTER_POST_EXEC, 0, ACK_RX_MAX_DURATION_MICRO, TRUE, FALSE, PublicDataTxPostExecCallback, OMACClockSpecifier);
	ASSERT_SP(rm == TimerSupported);
}

UINT64 DataTransmissionHandler::CalculateNextTxMicro(UINT16 dest){
	UINT64 nextTXmicro = MAX_UINT64;
	Neighbor_t* neigh_ptr = g_NeighborTable.GetNeighborPtr(dest);
	if(neigh_ptr == NULL) {
		return nextTXmicro;
	}
//#if OMAC_DTH_DEBUG_LATEWAKEUP
	UINT64 m_scheduledTXTime_in_neigh_clock_ticks = neigh_ptr->nextwakeupSlot * SLOT_PERIOD_TICKS;
	UINT64 m_scheduledTXTime_in_own_clock_ticks = g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Neighbor2LocalTime(dest, m_scheduledTXTime_in_neigh_clock_ticks);
	//UINT64 nextTXmicro = g_OMAC.m_Clock.ConvertTickstoMicroSecs(nextTXTicks) - PROCESSING_DELAY_BEFORE_TX_MICRO - RADIO_TURN_ON_DELAY_MICRO;
//#endif
	nextTXmicro = g_OMAC.m_Clock.SubstractMicroSeconds( g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Neighbor2LocalTime(dest, neigh_ptr->nextwakeupSlot * SLOT_PERIOD_TICKS)) , (g_OMAC.RADIO_TURN_ON_DELAY_TX+g_OMAC.DELAY_FROM_OMAC_TX_TO_RADIO_DRIVER_TX));
	//nextTXmicro = g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Neighbor2LocalTime(dest, neigh_ptr->nextwakeupSlot * SLOT_PERIOD_TICKS));
	if(EXECUTE_WITH_CCA){
		if(nextTXmicro > g_OMAC.CCA_PERIOD_ACTUAL) {
			nextTXmicro -= g_OMAC.CCA_PERIOD_ACTUAL ;
		}
	}
	if(FAST_RECOVERY){
		if(nextTXmicro > GUARDTIME_MICRO) {
			nextTXmicro -= GUARDTIME_MICRO;
		}
	}
	if(FAST_RECOVERY2){
	}
#if OMAC_DTH_DEBUG_LATEWAKEUP
	m_scheduledFUTime_in_own_clock_micro = nextTXmicro;
#endif
	return nextTXmicro;
}

UINT64 DataTransmissionHandler::CalculateNextRXOpp(UINT16 dest){
	if(UpdateNeighborsWakeUpSlot(dest, 0)) {


		UINT64 nextTXmicro = CalculateNextTxMicro(dest);
		UINT64 curmicro =  g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_Clock.GetCurrentTimeinTicks());

		while(nextTXmicro  <= curmicro + OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO) {
			if(UpdateNeighborsWakeUpSlot(dest, 1)){
				nextTXmicro = CalculateNextTxMicro(dest);
				curmicro =  g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_Clock.GetCurrentTimeinTicks());
			}
			else{
				nextTXmicro = MAX_UINT64;
				break;
			}
		}
		ASSERT_SP(nextTXmicro > curmicro);

		UINT64 remMicroSecnextTX = nextTXmicro - curmicro;
		//Wake up the transmitter a little early
		//remMicroSecnextTX -= GUARDTIME_MICRO;


#ifdef OMAC_DEBUG_PRINTF
		//OMAC_HAL_PRINTF("DataTransmissionHandler::NextEvent curmicro: %llu; nextTXmicro: %llu; remMicroSecnextTX: %llu \r\n", curmicro, nextTXmicro, remMicroSecnextTX);
		//OMAC_HAL_PRINTF("DataTransmissionHandler::NextEvent curmicro: %s; nextTXmicro: %s; remMicroSecnextTX: %s \r\n", l2s(curmicro,0), l2s(nextTXmicro,0), l2s(remMicroSecnextTX,0));

#ifdef def_Neighbor2beFollowed
		//OMAC_HAL_PRINTF("\r\n[LT: %llu - %lu NT: %llu - %lu] DataTransmissionHandler::NextEvent() remMicroSecnextTX= %llu AbsnextWakeupTimeInMicSec= %llu - %lu m_neighborNextEventTimeinMicSec = %llu - %lu\r\n"
		//		, g_OMAC.m_Clock.ConvertTickstoMicroSecs(curTicks), g_OMAC.m_omac_scheduler.GetSlotNumber(), g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks)), g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, curTicks))
		//		, remMicroSecnextTX, g_OMAC.m_Clock.ConvertTickstoMicroSecs(nextTXTicks), g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(nextTXTicks), g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_NeighborTable.GetNeighborPtr(_m_outgoingEntryPtr->GetHeader()->dest)->nextwakeupSlot * SLOT_PERIOD_TICKS), g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(g_NeighborTable.GetNeighborPtr(_m_outgoingEntryPtr->GetHeader()->dest)->nextwakeupSlot * SLOT_PERIOD_TICKS) );
#endif
#endif

		/*UINT64 neighborSlot = g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(dest, g_OMAC.m_Clock.GetCurrentTimeinTicks()));
		Neighbor_t* neigh_ptr = g_NeighborTable.GetNeighborPtr(dest);
		UINT64 neighborwakeUpSlot = neigh_ptr->nextwakeupSlot;
		if(neighborwakeUpSlot - neighborSlot < 20 ){
			neighborwakeUpSlot = neighborwakeUpSlot+1;
		}*/
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATARX_NEXTEVENT, FALSE );
#endif
		return remMicroSecnextTX;
	}
	else {
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATARX_NEXTEVENT, FALSE );
#endif
		//Either Dont have packet to send or missing timing for the destination
		return MAX_UINT64;
	}

}

void DataTransmissionHandler::SendACKToUpperLayers(Message_15_4_t* msg, UINT16 Size, NetOpStatus status, UINT8 radioAckStatus){
	SendAckFuncPtrType m_txAckHandler = NULL;
	MACEventHandler* mac_event_handler_ptr = NULL;

	if (msg != NULL && msg->GetHeader()){
		if(msg->GetHeader()->payloadType != MFM_OMAC_TIMESYNCREQ){
			if(IsValidNativeAppIdOffset(msg->GetHeader()->payloadType) ){
				mac_event_handler_ptr = g_OMAC.GetNativeAppHandler(msg->GetHeader()->payloadType);
				if(mac_event_handler_ptr != NULL) {
					m_txAckHandler = mac_event_handler_ptr->GetSendAckHandler();
				}
			}
			if(m_txAckHandler == NULL){
				m_txAckHandler = g_OMAC.m_txAckHandler;
			}
			if(m_txAckHandler != NULL){
				(m_txAckHandler)(msg, Size, status, radioAckStatus);
			}
		}
		else{
			if(status == NetworkOperations_Fail || status == NetworkOperations_Success){
				g_NeighborTable.DeletePacket(msg);
			}
		}
	}
}

/*
 * This function returns the number of ticks until the transmission time
 */
UINT64 DataTransmissionHandler::NextEvent(){
	//in case the task delay is large and we are already pass
	//tx time, tx immediately
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATARX_NEXTEVENT, TRUE );
	CPU_GPIO_SetPinState( DATATX_NEXT_EVENT, TRUE );
#endif

	Message_15_4_t* msg_carrier = g_NeighborTable.FindStalePacketWithRetryAttemptsGreaterThan(FRAMERETRYMAXATTEMPT, BO_OMAC);
	while(msg_carrier){
		SendACKToUpperLayers(msg_carrier, sizeof(Message_15_4_t), NetworkOperations_Fail, TRAC_STATUS_FAIL_TO_SEND);
		msg_carrier = g_NeighborTable.FindStalePacketWithRetryAttemptsGreaterThan(FRAMERETRYMAXATTEMPT,  BO_OMAC);
	}


//	g_NeighborTable.FindAndRemoveStalePackets(FRAMERETRYMAXATTEMPT);

	//Check all Neighbors that have a packet in the queue
	isDataPacketScheduled = false;
	m_outgoingEntryPtr_dest = 0;
	m_outgoingEntryPtr = NULL;
	UINT64 cur_remMicroSecnextTX, remMicroSecnextTX = MAX_UINT64;
	for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		if(g_NeighborTable.Neighbor[i].neighborStatus != Dead){
			if( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.IsNeighborTimeAvailable(g_NeighborTable.Neighbor[i].MACAddress)){
				// Readjust the neighbors queues and drop packets that have failed FRAMERETRYMAXATTEMPT
				if(g_NeighborTable.IsThereAPacketWithDest(g_NeighborTable.Neighbor[i].MACAddress)){
					cur_remMicroSecnextTX = CalculateNextRXOpp(g_NeighborTable.Neighbor[i].MACAddress);
					if(cur_remMicroSecnextTX < remMicroSecnextTX){
						remMicroSecnextTX = cur_remMicroSecnextTX;
						m_outgoingEntryPtr_dest = g_NeighborTable.Neighbor[i].MACAddress;
						isDataPacketScheduled = true;
					}
				}
			}
		}
	}

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATATX_NEXT_EVENT, FALSE );
#endif
	//At this point we have decided to send the packet to m_outgoingEntryPtr_dest
	if(isDataPacketScheduled) {
		return CalculateNextRXOpp(m_outgoingEntryPtr_dest);
	}
	else {
		return MAX_UINT64;
	}
}

void DataTransmissionHandler::DropPacket(){
	//Packet will have to be dropped if retried max attempts
	//However this function is only used for successfull packet drops
#if OMAC_DTH_PRINT_NEIGHBOR_TABLE_AFTER_SEND
			g_OMAC.is_print_neigh_table = true;
#endif
#ifdef OMAC_DEBUG_PRINTF
	OMAC_HAL_PRINTF("dropping packet\r\n");
#endif
#if OMAC_DEBUG_PRINTF_PACKETDROP_SUCESS
#endif


	Neighbor_t* neigh_ptr = g_NeighborTable.GetNeighborPtr(m_outgoingEntryPtr_dest);
	if(neigh_ptr == NULL) {
		ASSERT_SP(0);
	}
	else if(m_outgoingEntryPtr == NULL) {
//		hal_printf("Packet is not assigned");
//		ASSERT_SP(0);
		return;
	}
	else {
		if(m_outgoingEntryPtr->GetHeader()->flags & MFM_DISCOVERY_FLAG){
			neigh_ptr->RecordMyScheduleSent();
		}
		if(m_outgoingEntryPtr->GetHeader()->flags & MFM_TIMESYNC_FLAG){
			neigh_ptr->IncrementNumInitMessagesSent();
		}

		if(neigh_ptr->IsInitializationTimeSamplesNeeded() && g_NeighborTable.IsThereATSRPacketWithDest(m_outgoingEntryPtr_dest) && m_outgoingEntryPtr == g_NeighborTable.FindTSRPacketForNeighbor(m_outgoingEntryPtr_dest)  ) {
#if OMAC_DEBUG_PRINTF_PACKETDROP_SUCESS
			hal_printf("Dropping TSR Packet SUCCESS NumTimeSyncMessagesSent = %u < INITIAL_RETRY_BACKOFF_WINDOW_SIZE dest= %u payloadType= %u, flags = %u, Retry Attempts = %u \r\n"
					, neigh_ptr->NumInitializationMessagesSent
					, m_outgoingEntryPtr->GetHeader()->dest
					, m_outgoingEntryPtr->GetHeader()->payloadType
					, m_outgoingEntryPtr->GetHeader()->flags
					, m_outgoingEntryPtr->GetMetaData()->GetRetryAttempts());
#endif
			//			ClearMsgContents(m_outgoingEntryPtr);
			g_NeighborTable.DeletePacket(m_outgoingEntryPtr);
			//			neigh_ptr->tsr_send_buffer.DropOldest(1);
			neigh_ptr->SendLink.RecordPacketSuccess(true);
		}
		else if(g_NeighborTable.IsThereAPacketWithDest(m_outgoingEntryPtr_dest) && m_outgoingEntryPtr == g_NeighborTable.FindDataPacketForNeighbor(m_outgoingEntryPtr_dest) ) {
#if OMAC_DEBUG_PRINTF_PACKETDROP_SUCESS
			hal_printf("Dropping Data Packet SUCCESS dest= %u payloadType= %u, flags = %u, Retry Attempts = %u \r\n"
					, m_outgoingEntryPtr->GetHeader()->dest
					, m_outgoingEntryPtr->GetHeader()->payloadType
					, m_outgoingEntryPtr->GetHeader()->flags
					, m_outgoingEntryPtr->GetMetaData()->GetRetryAttempts());
#endif

			//			ClearMsgContents(m_outgoingEntryPtr);
			//			neigh_ptr->send_buffer.DropOldest(1);
			//g_NeighborTable.DeletePacket(m_outgoingEntryPtr);  //BK: Rmoving this case relying on the ACK mechanism to remove the packet
			neigh_ptr->SendLink.RecordPacketSuccess(true);

			if( (m_outgoingEntryPtr->GetHeader()->flags & MFM_TIMESYNC_FLAG)
			){ //This is flushing the time sync message queue if the previous message was successful
				while((g_NeighborTable.IsThereATSRPacketWithDest(m_outgoingEntryPtr_dest))){
					g_NeighborTable.DeletePacket(g_NeighborTable.FindTSRPacketForNeighbor(m_outgoingEntryPtr_dest));
				}

			}
			if(true){
#if OMAC_DTH_DEBUG_ReceiveDATAACK_PRINTOUT
				hal_printf("DropPacket:NetworkOperations_Success dest = %u \r\r\n", m_outgoingEntryPtr->GetHeader()->dest);
#endif
				SendACKToUpperLayers(m_outgoingEntryPtr, sizeof(Message_15_4_t), NetworkOperations_Success, TRAC_STATUS_SUCCESS);
			}


		}
		else if(g_NeighborTable.IsThereATSRPacketWithDest(m_outgoingEntryPtr_dest) && m_outgoingEntryPtr == g_NeighborTable.FindTSRPacketForNeighbor(m_outgoingEntryPtr_dest) ){
#if OMAC_DEBUG_PRINTF_PACKETDROP_SUCESS
			hal_printf("Dropping TSR Packet SUCCESS DestID dest= %u payloadType= %u, flags = %u, Retry Attempts = %u \r\n"
					, m_outgoingEntryPtr->GetHeader()->dest
					, m_outgoingEntryPtr->GetHeader()->payloadType
					, neigh_ptr->tsr_send_buffer.Getm_outgoingEntryPtreigh_ptr->tsr_send_buffer.GetOldestwithoutRemoval()->GetMetaData()->GetRetryAttempts());
#endif
			//			ClearMsgContents(neigh_ptr->tsr_send_buffer.GetOldestwithoutRemoval());
			//			neigh_ptr->tsr_send_buffer.DropOldest(1);

			g_NeighborTable.DeletePacket(m_outgoingEntryPtr);
			neigh_ptr->SendLink.RecordPacketSuccess(true);
			//neigh_ptr->tsr_send_buffer.ClearBuffer();
		}
		else{ // The packet is gone
			//ASSERT_SP(0);
		}
	}

	isDataPacketScheduled = false;

	m_currentSlotRetryAttempt = 0;
}

void DataTransmissionHandler::SendRetry(){ // BK: This function is called to retry in the case of FAST_RECOVERY
	VirtualTimerReturnMessage rm;
	m_currentSlotRetryAttempt++;

	if(FAST_RECOVERY && txhandler_state == DTS_WAITING_FOR_ACKS && m_currentSlotRetryAttempt < SLOTRETRYMAXATTEMPT){
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( FAST_RECOVERY_SEND, TRUE );
		CPU_GPIO_SetPinState( FAST_RECOVERY_SEND, FALSE );
#endif
		//Enable m_RANDOM_BACKOFF for retries
		m_RANDOM_BACKOFF = true;
		ExecuteEventHelper();
	}
	else{
		//VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER_POST_EXEC);
		PostExecuteEvent();
	}
}

void DataTransmissionHandler::ExecuteEventHelper() { // BK: This function starts sending routine for a packet
#ifdef OMAC_DEBUG_GPIO //Mark 3 ExecuteEventHelper Start
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif

	bool canISend = true;
	UINT64 y;
	DeviceStatus DS = DS_Success;
	VirtualTimerReturnMessage rm;

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATATX_PIN, FALSE );
	CPU_GPIO_SetPinState( DATATX_PIN, TRUE );
#endif

	//The number 500 was chosen arbitrarily. In reality it should be the sum of backoff period + CCA period + guard band.
	//For GUARDTIME_MICRO period check the channel before transmitting
	//140 usec is the time taken for CCA to return a result
	if(EXECUTE_WITH_CCA) y = g_OMAC.m_Clock.GetCurrentTimeinTicks();

	while(EXECUTE_WITH_CCA){
		//If retrying, don't do CCA, but perform random backoff and transmit
		if(m_currentSlotRetryAttempt > 0){
			break;
		}
		//Check CCA only for DATA packets

#if OMAC_DTH_DEBUG_CCA
		if(DATATX_CCA_PIN_TOGGLER != DISABLED_PIN){
			CPU_GPIO_SetPinState( DATATX_CCA_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_CCA_PIN_TOGGLER) );
			CPU_GPIO_SetPinState( DATATX_CCA_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_CCA_PIN_TOGGLER) );
		}
#endif
		DS = CPU_Radio_ClearChannelAssesment(g_OMAC.radioName);

#if OMAC_DTH_DEBUG_CCA
		if(DATATX_CCA_PIN_TOGGLER != DISABLED_PIN){
			CPU_GPIO_SetPinState( DATATX_CCA_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_CCA_PIN_TOGGLER) );
			CPU_GPIO_SetPinState( DATATX_CCA_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_CCA_PIN_TOGGLER) );
		}
#endif


		if(DS != DS_Success){
#ifdef OMAC_DEBUG_PRINTF
			OMAC_HAL_PRINTF("transmission detected!\r\n");
#endif
			//i = GUARDTIME_MICRO/140;
			canISend = false;
			break;
		}
		else { 
#ifdef OMAC_DEBUG_PRINTF
			OMAC_HAL_PRINTF("transmission CAD not detected!\r\n");
#endif
		} 
		canISend = true;

		if(m_currentSlotRetryAttempt == 0){
			if( g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_Clock.GetCurrentTimeinTicks() - y) > CCA_PERIOD_MICRO){
				break;
			}
		}
		else{
			if( g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_Clock.GetCurrentTimeinTicks() - y) > CCA_PERIOD_FRAME_RETRY_MICRO){
				break;
			}
		}
	}

#ifdef OMAC_DEBUG_GPIO //Mark 4 After CCA
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif

	//Perform CCA for random backoff period (only for retries)
	if(m_RANDOM_BACKOFF){
		UINT16 randVal = g_OMAC.m_omac_scheduler.m_seedGenerator.RandWithMask(&m_backoff_seed, m_backoff_mask);
		m_backoff_seed = randVal;
		int i = 0;
		int finalBackoffValue = randVal % g_OMAC.RANDOM_BACKOFF_COUNT_MAX;
		//OMAC_HAL_PRINTF("rand value is %d\r\n", (randVal % RANDOM_BACKOFF_COUNT));
		while(i <= (randVal % g_OMAC.RANDOM_BACKOFF_COUNT_MAX)){
			++i;
			DS = CPU_Radio_ClearChannelAssesment(g_OMAC.radioName);
			if(DS != DS_Success){
				OMAC_HAL_PRINTF("transmission detected (inside backoff)!\r\n");
				canISend = false;
				break;
			}
		}
	}

#ifdef OMAC_DEBUG_GPIO //Mark 5 After Random Backoff
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif


#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATATX_PIN, FALSE );
	CPU_GPIO_SetPinState( DATATX_PIN, TRUE );
#endif
	//Transmit
	if(canISend){
		txhandler_state = DTS_CCA_CLEAR;
		//resendSuccessful = false;

#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATATX_DATA_PIN, TRUE );
		CPU_GPIO_SetPinState( DATATX_DATA_PIN, FALSE );
#endif





		bool rv = Send();

		if(rv) {

#if OMAC_DEBUG_PRINTF_TXATTEMPT_SUCCESS
			if(m_outgoingEntryPtr != NULL){
				hal_printf("TXATTEMPT_SUCCESSL dest= %u payloadType= %u, flags = %u, Retry Attempts = %u \r\n"
						, m_outgoingEntryPtr->GetHeader()->dest
						, m_outgoingEntryPtr->GetHeader()->payloadType
						, m_outgoingEntryPtr->GetHeader()->flags
						, m_outgoingEntryPtr->GetMetaData()->GetRetryAttempts());
			}
			else{
				hal_printf("TXATTEMPT_SUCCESS NO m_outgoingEntryPtr \r\n");
			}
#endif


			txhandler_state = DTS_SEND_INITIATION_SUCCESS;
			if(CPU_Radio_GetRadioAckType() == NO_ACK){
				DropPacket();
			}
		}
		else{
			txhandler_state = DTS_SEND_INITIATION_FAIL;
#ifdef OMAC_DEBUG_GPIO
			//OMAC_HAL_PRINTF("DataTransmissionHandler::ExecuteEventHelper Toggling\r\n");
			CPU_GPIO_SetPinState( DATATX_PIN, FALSE );
			CPU_GPIO_SetPinState( DATATX_PIN, TRUE );
			CPU_GPIO_SetPinState( DATATX_PIN, FALSE );
			CPU_GPIO_SetPinState( DATATX_PIN, TRUE );
#endif
		}
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER); 
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, g_OMAC.MAX_PACKET_TX_DURATION_MICRO, TRUE, OMACClockSpecifier );
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION||OMAC_DTH_DEBUG_UNEXPECTED_POST_EX
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( g_OMAC.MAX_PACKET_TX_DURATION_MICRO);
		m_scheduledTimer_in_ticks2 = m_scheduledTimer_in_ticks;
#endif
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		//rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, FAST_RECOVERY_WAIT_PERIOD_MICRO, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER); 
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}


		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
	}
	else{ //I cannot send
#if OMAC_DEBUG_PRINTF_CCA_TXFAIL
		hal_printf("CCA_TXFAIL dest= %u \r\n",m_outgoingEntryPtr_dest);
#endif

		txhandler_state = DTS_CCA_BUSY;



		//Increment number of retries for the current packet
		m_outgoingEntryPtr = SelectPacketForDest(m_outgoingEntryPtr_dest);
		if(m_outgoingEntryPtr != NULL && isDataPacketScheduled){
			IEEE802_15_4_Metadata* metadata = m_outgoingEntryPtr->GetMetaData();
			metadata->SetRetryAttempts(metadata->GetRetryAttempts()+1);
		}

		//		Neighbor_t* neigh_ptr = g_NeighborTable.GetNeighborPtr(m_outgoingEntryPtr_dest);
		//		if(neigh_ptr != NULL){
		//			m_outgoingEntryPtr = NULL;
		//			if(neigh_ptr->send_buffer.GetNumberMessagesInBuffer() > 0 ) {
		//				m_outgoingEntryPtr = neigh_ptr->send_buffer.GetOldestwithoutRemoval();
		//			}
		//			else if(neigh_ptr->tsr_send_buffer.GetNumberMessagesInBuffer() > 0 ){
		//				m_outgoingEntryPtr = neigh_ptr->tsr_send_buffer.GetOldestwithoutRemoval();
		//			}
		//			else{
		//				//Commenting out, as sometimes there may not be anything to be sent,
		//				//	in which case Send returns a false.
		//				//ASSERT_SP(0);
		//			}
		//			if(m_outgoingEntryPtr != NULL && isDataPacketScheduled){
		//				IEEE802_15_4_Metadata* metadata = m_outgoingEntryPtr->GetMetaData();
		//				metadata->SetRetryAttempts(metadata->GetRetryAttempts()+1);
		//			}
		//		}

		//Stop execution
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER); 
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, 0, TRUE, OMACClockSpecifier );
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION||OMAC_DTH_DEBUG_UNEXPECTED_POST_EX
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER); 
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
	}
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATATX_PIN, FALSE );
#endif
}

/*
 *
 */
void DataTransmissionHandler::ExecuteEvent(){
#ifdef OMAC_DEBUG_PRINTF
#ifdef def_Neighbor2beFollowed
	//OMAC_HAL_PRINTF("\r\n[LT: %llu - %lu NT: %llu - %lu] DataTransmissionHandler:ExecuteEvent \r\n"
	//		, g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_Clock.GetCurrentTimeinTicks()), g_OMAC.m_omac_scheduler.GetSlotNumber(), g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, g_OMAC.m_Clock.GetCurrentTimeinTicks())), g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, g_OMAC.m_Clock.GetCurrentTimeinTicks())) );
	OMAC_HAL_PRINTF("\r\n[LT: %s - %s NT: %s - %s] DataTransmissionHandler:ExecuteEvent \r\n"
			, l2s(g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_Clock.GetCurrentTimeinTicks()),0), l2s(g_OMAC.m_omac_scheduler.GetSlotNumber(),0), l2s(g_OMAC.m_Clock.ConvertTickstoMicroSecs(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, g_OMAC.m_Clock.GetCurrentTimeinTicks())),0), l2s(g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(g_OMAC.Neighbor2beFollowed, g_OMAC.m_Clock.GetCurrentTimeinTicks())),0) );

#endif
#endif

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATATX_PIN, TRUE );
	CPU_GPIO_SetPinState( DATATX_NEXT_EVENT, TRUE );
#endif

#ifdef OMAC_DEBUG_GPIO //Mark 1 Start
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif


	VirtualTimerReturnMessage rm;
	//When starting a new send, reset attempt to 0
	m_currentSlotRetryAttempt = 0;
	m_RANDOM_BACKOFF = false;
	txhandler_state = DTS_EXECUTE_START;
	DeviceStatus e = DS_Fail;

	Neighbor_t* neigh_ptr = g_NeighborTable.GetNeighborPtr(m_outgoingEntryPtr_dest);
	if(neigh_ptr == NULL){
		e = DS_Fail;
		txhandler_state = DTS_DEST_NOT_AVAILABLE;
	}
	else if(neigh_ptr->random_back_off_slot != 0){
		--(neigh_ptr->random_back_off_slot);
		e = DS_Fail;
		txhandler_state = DTS_BACKOFF;
	}
	else{
		e = g_OMAC.m_omac_RadioControl.StartRx();
		if(e == DS_Success){
			txhandler_state = DTS_RADIO_START_SUCCESS;
		}
		else{
			txhandler_state = DTS_RADIO_START_FAILED;
		}
	}



#ifdef OMAC_DEBUG_GPIO //Mark 2 After Radio Start
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
	CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif

	if(e == DS_Success){
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATATX_NEXT_EVENT, TRUE );
		CPU_GPIO_SetPinState( DATATX_NEXT_EVENT, FALSE );
#endif
		this->ExecuteEventHelper();
	}
	else{
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATATX_NEXT_EVENT, FALSE );
		CPU_GPIO_SetPinState( DATATX_NEXT_EVENT, TRUE );
		CPU_GPIO_SetPinState( DATATX_NEXT_EVENT, FALSE );
#endif
		//OMAC_HAL_PRINTF("DataTransmissionHandler::ExecuteEvent Radio not in RX state\r\n");
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER); 
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, 0, TRUE, OMACClockSpecifier );
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION||OMAC_DTH_DEBUG_UNEXPECTED_POST_EX
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER);
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
	}
}

void DataTransmissionHandler::SelectRetrySlotNumForNeighborBackOff(){
	Neighbor_t* neigh_ptr = g_NeighborTable.GetNeighborPtr(m_outgoingEntryPtr_dest);
	if(neigh_ptr != NULL){
		if(neigh_ptr->random_back_off_window_size >= INITIAL_RETRY_BACKOFF_WINDOW_SIZE && neigh_ptr->random_back_off_window_size <= MAX_RETRY_BACKOFF_WINDOW_SIZE){
			neigh_ptr->random_back_off_window_size = neigh_ptr->random_back_off_window_size * 2;
		}
		else {
			neigh_ptr->random_back_off_window_size = INITIAL_RETRY_BACKOFF_WINDOW_SIZE;
		}
		if(neigh_ptr->random_back_off_window_size > MAX_RETRY_BACKOFF_WINDOW_SIZE){
			neigh_ptr->random_back_off_window_size = MAX_RETRY_BACKOFF_WINDOW_SIZE;
		}

		UINT16 randVal = g_OMAC.m_omac_scheduler.m_seedGenerator.RandWithMask(&m_backoff_seed, m_backoff_mask);
		m_backoff_seed = randVal;
		neigh_ptr->random_back_off_slot = randVal % neigh_ptr->random_back_off_window_size;
	}
}

void DataTransmissionHandler::SendACKHandler(Message_15_4_t* rcv_msg, UINT8 radioAckStatus){


#if OMAC_DTH_DEBUG_SendACKHandler //Mark 7
	if(DATATX_SendACKHandler_PIN_TOGGLER != DISABLED_PIN){
		CPU_GPIO_SetPinState( DATATX_SendACKHandler_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_SendACKHandler_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DATATX_SendACKHandler_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_SendACKHandler_PIN_TOGGLER) );
	}
#endif
	txhandler_state = DTS_SEND_FINISHED;
	RadioAddress_t dest = m_outgoingEntryPtr_dest;
	RadioAddress_t myID = g_OMAC.GetMyAddress();
	VirtualTimerReturnMessage rm;

	if(CPU_Radio_GetRadioAckType() == HARDWARE_ACK){
		//resendSuccessful = true;
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RX_DATAACK_PIN, TRUE);
		//CPU_GPIO_SetPinState( HW_ACK_PIN, TRUE );
#endif
		if(radioAckStatus == TRAC_STATUS_SUCCESS || radioAckStatus == TRAC_STATUS_SUCCESS_DATA_PENDING){
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, TRUE );
			CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
#endif

			if(true){ //if(g_OMAC.m_txAckHandler != NULL){
				if(m_outgoingEntryPtr->GetHeader()->payloadType != MFM_OMAC_TIMESYNCREQ){
					SendACKToUpperLayers(rcv_msg, sizeof(Message_15_4_t), NetworkOperations_SendACKed, radioAckStatus);
#if OMAC_DTH_DEBUG_ReceiveDATAACK_PRINTOUT
					hal_printf("DataTransmissionHandler:HARDWARE_ACKSendACK:NetworkOperations_SendACKed dest = %u \r\r\n", rcv_msg->GetHeader()->dest);
#endif

				}
			}

			//Drop data packets only if send was successful
			DropPacket();
			//set flag to false after packet has been sent and ack received

			m_currentSlotRetryAttempt = 0;
			//m_currentFrameRetryAttempt = 0;
			//dest = rcv_msg->GetHeader()->dest;
			if(dest != myID){
				DeviceStatus ds = g_NeighborTable.RecordLastHeardTime(dest, g_OMAC.m_Clock.GetCurrentTimeinTicks());
				if(ds != DS_Success){
#ifdef OMAC_DEBUG_PRINTF
					OMAC_HAL_PRINTF("DataTransmissionHandler::SendACKHandler RecordLastHeardTime failure; address: %d; line: %d\r\n", dest, __LINE__);
#endif
				}
			}

			txhandler_state = DTS_RECEIVEDDATAACK;
			rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER); 
			if(rm != TimerSupported) {
				//SOFT_BREAKPOINT();
			}

			rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, 0, TRUE, OMACClockSpecifier );
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION||OMAC_DTH_DEBUG_UNEXPECTED_POST_EX
			m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
			if(rm != TimerSupported) {
				//SOFT_BREAKPOINT();
			}

			rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER); 
			if(rm != TimerSupported) {
				//SOFT_BREAKPOINT();
			}

#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(OMAC_RX_DATAACK_PIN, FALSE);
			//CPU_GPIO_SetPinState( HW_ACK_PIN, TRUE );
#endif
			if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
				PostExecuteEvent();
			}
		}
		else if(radioAckStatus == TRAC_STATUS_NO_ACK || radioAckStatus == TRAC_STATUS_FAIL_TO_SEND){
			//Drop timesync packets irrespective of whether send was successful or not.
			//Don't retry a TS packet (for now)
			/*if(rcv_msg->GetHeader()->payloadType == MFM_TIMESYNCREQ){
				m_currentSlotRetryAttempt = 0;
				m_currentFrameRetryAttempt = 0;
				//isDataPacketScheduled = false;
				//m_outgoingEntryPtr = NULL;
				g_send_buffer.DropOldest(1);
			}
			else if(m_outgoingEntryPtr->GetHeader()->payloadType == MFM_DATA){
				m_currentSlotRetryAttempt++;
			}*/

			if(radioAckStatus == TRAC_STATUS_NO_ACK){

				if(true){//if(g_OMAC.m_txAckHandler != NULL){
					if(m_outgoingEntryPtr->GetHeader()->payloadType != MFM_OMAC_TIMESYNCREQ){
						SendACKToUpperLayers(rcv_msg, sizeof(Message_15_4_t), NetworkOperations_SendNACKed, radioAckStatus);
#if OMAC_DTH_DEBUG_ReceiveDATAACK_PRINTOUT
						hal_printf("DataTransmissionHandler:HARDWARE_ACKSendACK:NetworkOperations_SendNACKed dest = %u \r\r\n", rcv_msg->GetHeader()->dest);
#endif
					}
				}

				if(FAST_RECOVERY2){
					g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.UnsuccessfulTransmission(m_outgoingEntryPtr_dest);
				}
#ifdef OMAC_DEBUG_GPIO
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, TRUE );
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, TRUE );
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
#endif
			}
			else if(radioAckStatus == TRAC_STATUS_FAIL_TO_SEND){
#ifdef OMAC_DEBUG_GPIO
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, TRUE );
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, TRUE );
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, TRUE );
				CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
#endif
			}

			if(true){//if(g_OMAC.m_txAckHandler != NULL){
				if(m_outgoingEntryPtr->GetHeader()->payloadType != MFM_OMAC_TIMESYNCREQ){
					SendACKToUpperLayers(rcv_msg, sizeof(Message_15_4_t), NetworkOperations_Busy, radioAckStatus);
#if OMAC_DTH_DEBUG_ReceiveDATAACK_PRINTOUT
					hal_printf("DataTransmissionHandler:HARDWARE_ACKSendACK:NetworkOperations_Busy dest = %u \r\r\n", rcv_msg->GetHeader()->dest);
#endif
				}
			}

			txhandler_state = DTS_WAITING_FOR_ACKS;
			rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER); 
			if(rm != TimerSupported) {
				//SOFT_BREAKPOINT();
			}

			rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, 0, TRUE, OMACClockSpecifier );
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION||OMAC_DTH_DEBUG_UNEXPECTED_POST_EX
			m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
			if(rm != TimerSupported) {
				//SOFT_BREAKPOINT();
			}

			rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER);
			//if(rm == TimerSupported) txhandler_state = DTS_WAITING_FOR_ACKS;
			if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
				PostExecuteEvent();
			}
		}
		/*else if(radioAckStatus == TRAC_STATUS_FAIL_TO_SEND){
			CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, TRUE );
			CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
			CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, TRUE );
			CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
			CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, TRUE );
			CPU_GPIO_SetPinState( DATATX_SEND_ACK_HANDLER, FALSE );
			txhandler_state = DTS_WAITING_FOR_ACKS;
			rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER);
			rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, 2000, TRUE, OMACClockSpecifier );
			rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER);
			//if(rm == TimerSupported) txhandler_state = DTS_WAITING_FOR_ACKS;
			if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
				PostExecuteEvent();
			}
		}*/
		else{
#ifdef OMAC_DEBUG_PRINTF
			OMAC_HAL_PRINTF("radioAckStatus: %d\r\n", radioAckStatus);
#endif
			ASSERT_SP(0);
		}
	}
	else if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK){
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER); 
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}
#ifdef OMAC_DEBUG_PRINTF
			OMAC_HAL_PRINTF("Waiting RX ACK \r\n");
#endif
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, g_OMAC.MAX_PACKET_RX_DURATION_MICRO + g_OMAC.ACK_RX_MAX_DURATION_MICRO, TRUE, OMACClockSpecifier );
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION||OMAC_DTH_DEBUG_UNEXPECTED_POST_EX
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( g_OMAC.ACK_RX_MAX_DURATION_MICRO);
#endif
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}
		//Set up a timer with 1 microsecond delay (that is ideally 0 but would not make a difference)
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER); 
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		if(rm == TimerSupported) txhandler_state = DTS_WAITING_FOR_ACKS;
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
		if(true){ //if(g_OMAC.m_txAckHandler != NULL){
			if(m_outgoingEntryPtr->GetHeader()->payloadType != MFM_OMAC_TIMESYNCREQ){
				SendACKToUpperLayers(rcv_msg, sizeof(Message_15_4_t), NetworkOperations_SendInitiated, radioAckStatus);
#if OMAC_DTH_DEBUG_ReceiveDATAACK_PRINTOUT
				hal_printf("DataTransmissionHandler:SOFTWARE_ACKSendACK:NetworkOperations_SendInitiated dest = %u \r\r\n", rcv_msg->GetHeader()->dest);
#endif
			}
		}
	}
	else {
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, 0, TRUE, OMACClockSpecifier );
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION||OMAC_DTH_DEBUG_UNEXPECTED_POST_EX
		m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}
		//Set up a timer with 0 microsecond delay (that is ideally 0 but would not make a difference)
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER); 
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
	}
}

void DataTransmissionHandler::ReceiveDATAACK(UINT16 sourceaddress){ //Mark 8

	//2) Check if DataTransmissionHandler is active
	//1) SOFTWARE_ACKs are used
	//3) If the sourceID is equal to the destination of the original message
	//
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
	CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
#endif

	if( true
			&& 	CPU_Radio_GetRadioAckType() == SOFTWARE_ACK
			//&&	g_OMAC.m_omac_scheduler.m_state == (I_DATA_SEND_PENDING) //MS: commented this if the scheduler moved on
			&& g_OMAC.m_omac_scheduler.m_execution_started
			&& 	sourceaddress == m_outgoingEntryPtr_dest
	){
		if(txhandler_state != DTS_WAITING_FOR_ACKS) {
			return;
		}

#if OMAC_DTH_DEBUG_ReceiveDATAACK
		if(DATATX_ReceiveDATAACK_PIN_TOGGLER != DISABLED_PIN){ //Mark 8
			CPU_GPIO_SetPinState( DATATX_ReceiveDATAACK_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_ReceiveDATAACK_PIN_TOGGLER) );
			CPU_GPIO_SetPinState( DATATX_ReceiveDATAACK_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_ReceiveDATAACK_PIN_TOGGLER) );
		}
#endif


#if OMAC_SEND_DEBUGGING_FOR_MF
		hal_printf("A\r\n");
#endif


		txhandler_state = DTS_RECEIVEDDATAACK;




		if(m_outgoingEntryPtr != NULL){ //if(g_OMAC.m_txAckHandler != NULL && m_outgoingEntryPtr != NULL){
			if(m_outgoingEntryPtr->GetHeader()->payloadType != MFM_OMAC_TIMESYNCREQ){
				SendACKToUpperLayers(m_outgoingEntryPtr, sizeof(Message_15_4_t), NetworkOperations_SendACKed, TRAC_STATUS_SUCCESS);
#if OMAC_DTH_DEBUG_ReceiveDATAACK_PRINTOUT
				hal_printf("ReceiveDATAACK:NetworkOperations_SendACKed dest = %u \r\r\n", m_outgoingEntryPtr->GetHeader()->dest);
#endif
			}
		}


#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RX_DATAACK_PIN, TRUE);
		//CPU_GPIO_SetPinState( HW_ACK_PIN, TRUE );
		CPU_GPIO_SetPinState( DATATX_RECV_SW_ACK, TRUE );
#endif
		VirtualTimerReturnMessage rm;

#if	OMAC_DEBUG_PRINTF_PACKET_ACK_RX_SUCCESS
		if(m_outgoingEntryPtr && m_outgoingEntryPtr_dest != 0){
			hal_printf("ACK_RX_SUCCESS dest= %u payloadType= %u, flags = %u, Retry Attempts = %u \r\n"
					, m_outgoingEntryPtr->GetHeader()->dest
					, m_outgoingEntryPtr->GetHeader()->payloadType
					, m_outgoingEntryPtr->GetHeader()->flags
					, m_outgoingEntryPtr->GetMetaData()->GetRetryAttempts());
		}
		else{
			hal_printf("ACK_RX_SUCCESS NO m_outgoingEntryPtr \r\n");
		}
#endif

		g_NeighborTable.RecordLastHeardTime(sourceaddress,g_OMAC.m_Clock.GetCurrentTimeinTicks());
		DropPacket(); // The decision for dropping the packet depends on the outcome of the data reception


		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER); 
		if(rm != TimerSupported) {
			//SOFT_BREAKPOINT();
		}

		if(rm == TimerSupported){
			rm = VirtTimer_Change(VIRT_TIMER_OMAC_TRANSMITTER, 0, 0, TRUE, OMACClockSpecifier );
#if OMAC_DTH_TIMER_TARGET_TIME_CORRECTION||OMAC_DTH_DEBUG_UNEXPECTED_POST_EX
			m_scheduledTimer_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + g_OMAC.m_Clock.ConvertMicroSecstoTicks( 0);
#endif
			if(rm != TimerSupported) {
				//SOFT_BREAKPOINT();
			}
			//Set up a timer with 1 microsecond delay (that is ideally 0 but would not make a difference)
			rm = VirtTimer_Start(VIRT_TIMER_OMAC_TRANSMITTER); 
			if(rm != TimerSupported) {
				//SOFT_BREAKPOINT();
			}

			if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
				PostExecuteEvent();
			}
		}
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RX_DATAACK_PIN, FALSE);
		//CPU_GPIO_SetPinState( HW_ACK_PIN, FALSE );
		CPU_GPIO_SetPinState( DATATX_RECV_SW_ACK, FALSE );
#endif
	}
	else {
#if OMAC_DTH_DEBUG_ReceiveDATAACK_PRINTOUT
		hal_printf("ReceiveDATAACK: REJECTED: Got an Ack from dest = %u \r\r\n", sourceaddress);
#endif
	}
}


/*
 *
 */
void DataTransmissionHandler::PostExecuteEvent(){
#if OMAC_DTH_DEBUG_UNEXPECTED_POST_EX
	if(txhandler_state != DTS_RECEIVEDDATAACK && txhandler_state != DTS_RECEIVEDDATAACK){
#ifdef OMAC_DEBUG_GPIO //Mark 8
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif
		m_curTime_in_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		m_TimeDiff_in_micros = g_OMAC.m_Clock.ConvertTickstoMicroSecs(m_curTime_in_ticks - m_scheduledTimer_in_ticks);

		if(m_scheduledTimer_in_ticks > m_curTime_in_ticks){
			//hal_printf("DTH::EARLY FIRING PEE!! m_scheduledTimer_in_ticks = %llu, m_curTime_in_ticks = %llu",m_scheduledTimer_in_ticks,m_curTime_in_ticks  );
			hal_printf("DTH::EARLY FIRING PEE!! m_scheduledTimer_in_ticks = %s, m_curTime_in_ticks = %s",l2s(m_scheduledTimer_in_ticks,0),l2s(m_curTime_in_ticks,0)  );
	
		}

		if(m_outgoingEntryPtr && m_outgoingEntryPtr_dest != 0){
			hal_printf("DTH:UEPE dest= %u payloadType= %u, flags = %u, Retry Attempts = %u, txhandler_state= %u \r\n"
					, m_outgoingEntryPtr->GetHeader()->dest
					, m_outgoingEntryPtr->GetHeader()->payloadType
					, m_outgoingEntryPtr->GetHeader()->flags
					, m_outgoingEntryPtr->GetMetaData()->GetRetryAttempts()
					,txhandler_state
			);
		}
		else{
			hal_printf("DTH:UEPE NO m_outgoingEntryPtr, txhandler_state= %u \r\n", txhandler_state);
		}
	}
#endif
	if(txhandler_state == DTS_WAITING_FOR_ACKS && m_outgoingEntryPtr && m_outgoingEntryPtr_dest != 0){
#if OMAC_DEBUG_PRINTF_PACKET_ACK_RX_FAIL
#ifdef OMAC_DEBUG_GPIO //Mark 9
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif
		if(m_outgoingEntryPtr && m_outgoingEntryPtr_dest != 0){
			hal_printf("ACK RX FAIL dest= %u payloadType= %u, flags = %u, Retry Attempts = %u \r\n"
					, m_outgoingEntryPtr->GetHeader()->dest
					, m_outgoingEntryPtr->GetHeader()->payloadType
					, m_outgoingEntryPtr->GetHeader()->flags
					, m_outgoingEntryPtr->GetMetaData()->GetRetryAttempts());
#if OMAC_DTH_PRINT_NEIGHBOR_TABLE_AFTER_SEND
			g_OMAC.is_print_neigh_table = true;
#endif
		}
		else{
			hal_printf("ACK RX FAIL NO m_outgoingEntryPtr \r\n");
		}

#endif
		SelectRetrySlotNumForNeighborBackOff();
		if(m_outgoingEntryPtr->GetHeader()->payloadType != MFM_OMAC_TIMESYNCREQ){
			SendACKToUpperLayers(m_outgoingEntryPtr, sizeof(Message_15_4_t), NetworkOperations_SendNACKed, 0);
#if OMAC_DTH_DEBUG_ReceiveDATAACK_PRINTOUT
			hal_printf("DataTransmissionHandler:SOFTWARE_ACKSendACK:NetworkOperations_SendNACKed dest = %u \r\r\n", m_outgoingEntryPtr->GetHeader()->dest);
#endif
		}
	}
#if OMAC_DEBUG_PRINTF_PACKET_ACK_RX_FAIL
	else if(txhandler_state == DTS_RECEIVEDDATAACK){
		hal_printf("ACK RX SUCCESS! \r\n");
	}
#endif
	txhandler_state = DTS_POSTEXECUTION;
	//Scheduler's PostExecution stops the radio
	//g_OMAC.m_omac_RadioControl.Stop();
#ifdef OMAC_DEBUG_GPIO
	//CPU_GPIO_SetPinState( DATATX_POSTEXEC, TRUE );
	//CPU_GPIO_SetPinState( DATATX_POSTEXEC, FALSE );
	CPU_GPIO_SetPinState( DATATX_DATA_PIN, TRUE );
	CPU_GPIO_SetPinState( DATATX_DATA_PIN, FALSE );
#endif



	g_OMAC.m_omac_scheduler.PostExecution();
}

void DataTransmissionHandler::FailsafeStop(){
	VirtTimer_Stop(VIRT_TIMER_OMAC_TRANSMITTER);
}

Message_15_4_t* DataTransmissionHandler::SelectPacketForDest(UINT16 m_outgoingEntryPtr_dest_){

	Message_15_4_t* m_outgoingEntryPtr_local = NULL;

	Neighbor_t* neigh_ptr = g_NeighborTable.GetNeighborPtr(m_outgoingEntryPtr_dest_);
	if(neigh_ptr == NULL){
		return NULL;
	}

	m_outgoingEntryPtr_local = NULL;
	if(neigh_ptr->IsInitializationTimeSamplesNeeded() && g_NeighborTable.IsThereATSRPacketWithDest(m_outgoingEntryPtr_dest_) ) {
		m_outgoingEntryPtr_local = g_NeighborTable.FindTSRPacketForNeighbor(m_outgoingEntryPtr_dest_);
		txhandler_state = DTS_PACKET_SELECTED;
	}
	else if(g_NeighborTable.IsThereADataPacketWithDest(m_outgoingEntryPtr_dest_) ) {
		m_outgoingEntryPtr_local = g_NeighborTable.FindDataPacketForNeighbor(m_outgoingEntryPtr_dest_);
		txhandler_state = DTS_PACKET_SELECTED;
	}
	else if(g_NeighborTable.IsThereATSRPacketWithDest(m_outgoingEntryPtr_dest_) ){
		m_outgoingEntryPtr_local = g_NeighborTable.FindTSRPacketForNeighbor(m_outgoingEntryPtr_dest_);
		txhandler_state = DTS_PACKET_SELECTED;
	}
	else{
		txhandler_state = DTS_PACKET_NOTSELECTED;
		//Commenting out, as sometimes there may not be anything to be sent,
		//	in which case Send returns a false.
		//ASSERT_SP(0);
	}
	return m_outgoingEntryPtr_local;
}

/*
 *
 */
bool DataTransmissionHandler::Send(){
	//ASSERT_SP(m_outgoingEntryPtr != NULL);

	DeviceStatus rs;

	m_outgoingEntryPtr = SelectPacketForDest(m_outgoingEntryPtr_dest);

	//	Neighbor_t* neigh_ptr = g_NeighborTable.GetNeighborPtr(m_outgoingEntryPtr_dest);
	//	if(neigh_ptr == NULL){
	//		return false;
	//	}
	//
	//	m_outgoingEntryPtr = NULL;
	//	if(neigh_ptr->IsInitializationTimeSamplesNeeded() && neigh_ptr->tsr_send_bum_outgoingEntryPtrgoingEntryPtr = neigh_ptr->tsr_send_buffer.GetOldestwithoutRemoval();
	//	}
	//	else if(neigh_ptr->send_buffer.GetNumberMessagesInBuffer() > 0 ) {
	//		m_outgoingEntryPtr = neigh_ptr->send_buffer.GetOldestwithoutRemoval();
	//	}
	//	else if(neigh_ptr->tsr_send_buffer.GetNumberMessagesInBuffer() > 0 ){
	//		m_outgoingEntryPtr = neigh_ptr->tsr_send_buffer.GetOldestwithoutRemoval();
	//	}
	//	else{
	//		//Commenting out, as sometimes there may not be anything to be sent,
	//		//	in which case Send returns a false.
	//		//ASSERT_SP(0);
	//	}

	//Send only when packet has been scheduled
	if(m_outgoingEntryPtr != NULL && isDataPacketScheduled){
		UINT16 dest = m_outgoingEntryPtr->GetHeader()->dest;
		IEEE802_15_4_Header_t* header = m_outgoingEntryPtr->GetHeader();
		IEEE802_15_4_Metadata* metadata = m_outgoingEntryPtr->GetMetaData();
		metadata->SetRetryAttempts(metadata->GetRetryAttempts()+1);
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATATX_DATA_PIN, TRUE );
		CPU_GPIO_SetPinState( DATATX_PIN, FALSE );
		CPU_GPIO_SetPinState( DATATX_PIN, TRUE );
#endif

#if OMAC_DTH_DEBUG_LATEWAKEUP
		UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks() + OMAC_DTH_DELAY_FROM_SEND_TO_RADIO_DRIVER_SEND_IN_TICKS;
		UINT8 print_OMAC_DTH_DEBUG_LATEWAKEUP_error = 0;
		if(m_scheduledTXTime_in_own_clock_ticks < y  ){
			if(y  > m_scheduledTXTime_in_own_clock_ticks + OMAC_DTH_DEBUG_LATEWAKEUP_ALLOWANCE_IN_TICKS){
				print_OMAC_DTH_DEBUG_LATEWAKEUP_error = 1;
			}
		}
		else {
			if(y + OMAC_DTH_DEBUG_LATEWAKEUP_ALLOWANCE_IN_TICKS < m_scheduledTXTime_in_own_clock_ticks ){
				print_OMAC_DTH_DEBUG_LATEWAKEUP_error = 2;
			}
		}
#endif

#ifdef OMAC_DEBUG_GPIO //Mark 6 Before  Initiating Send from DTH
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
		CPU_GPIO_SetPinState( DTH_STATE_PIN_TOGGLER, !CPU_GPIO_GetPinState(DTH_STATE_PIN_TOGGLER) );
#endif

		txhandler_state = DTS_SEND_INITIATED;
		rs = g_OMAC.m_omac_RadioControl.Send(dest, m_outgoingEntryPtr, header->length);

#if OMAC_SEND_DEBUGGING_FOR_MF
		hal_printf("S\r\n");
#endif

#if OMAC_DTH_DEBUG_PRINTF_PACKET_SEND
		hal_printf("DTH:Send() dest= %u payloadType= %u, flags = %u, Retry Attempts = %u, rs = %u \r\n"
				, m_outgoingEntryPtr->GetHeader()->dest
				, m_outgoingEntryPtr->GetHeader()->payloadType
				, m_outgoingEntryPtr->GetHeader()->flags
				, m_outgoingEntryPtr->GetMetaData()->GetRetryAttempts(), rs);
#endif


#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( DATATX_DATA_PIN, FALSE );
#endif


#if OMAC_DTH_DEBUG_LATEWAKEUP_PIN_TOGGLING
		if(print_OMAC_DTH_DEBUG_LATEWAKEUP_error == 2) {
			if(DATATX_TIMING_ERROR_PIN_TOGGLER != DISABLED_PIN){
				CPU_GPIO_SetPinState( DATATX_TIMING_ERROR_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_TIMING_ERROR_PIN_TOGGLER) );
				CPU_GPIO_SetPinState( DATATX_TIMING_ERROR_PIN_TOGGLER, !CPU_GPIO_GetPinState(DATATX_TIMING_ERROR_PIN_TOGGLER) );
			}
		}
#endif

#if OMAC_DTH_DEBUG_LATEWAKEUP

		if(print_OMAC_DTH_DEBUG_LATEWAKEUP_error == 2) {
			//hal_printf("\r\r\n DTH LATE WAKEUP ERROR! Error in MicroSec= %llu \r\r\n"
			//		,g_OMAC.m_Clock.ConvertTickstoMicroSecs( m_scheduledTXTime_in_own_clock_ticks - y) );
			hal_printf("\r\r\n DTH LATE WAKEUP ERROR! Error in MicroSec= %s \r\r\n"
					,l2s(g_OMAC.m_Clock.ConvertTickstoMicroSecs( m_scheduledTXTime_in_own_clock_ticks - y),0) );
			
			}
		else if(print_OMAC_DTH_DEBUG_LATEWAKEUP_error == 1){
			//hal_printf("\r\r\n OMAC_DTH_DEBUG_LATEWAKEUP EARLY ERROR! scheduledTXTime_ticks = %llu , Cur Ticks = %llu scheduledTXTime_ticks_neigh = %llu Error in MicroSec= %llu\r\r\n"
			//	,m_scheduledTXTime_in_own_clock_ticks, y, m_scheduledTXTime_in_neigh_clock_ticks, y - m_scheduledTXTime_in_own_clock_ticks );
			//hal_printf("\r\r\n DTH EARLY WAKEUP ERROR! Error in MicroSec= %llu\r\r\n"
			//		,g_OMAC.m_Clock.ConvertTickstoMicroSecs( y - m_scheduledTXTime_in_own_clock_ticks) );
			hal_printf("\r\r\n DTH EARLY WAKEUP ERROR! Error in MicroSec= %s\r\r\n"
					,l2s(g_OMAC.m_Clock.ConvertTickstoMicroSecs( y - m_scheduledTXTime_in_own_clock_ticks),0) );
		}

#endif

		//m_outgoingEntryPtr = NULL;
		if(rs != DS_Success){
			return false;
		}
		else{
			return true;
		}
	}
	else{
		txhandler_state = DTS_SEND_PACKET_NOT_READY;
		return false;
	}
}

/* Schedule a data packet only if a neighbor is found or there are msgs in the buffer

BOOL DataTransmissionHandler::ScheduleDataPacket(UINT8 _skipperiods, UINT16 dest)
{
	CPU_GPIO_SetPinState( DATATX_SCHED_DATA_PKT, TRUE );
	CPU_GPIO_SetPinState( DATATX_SCHED_DATA_PKT, FALSE );
	// do not schedule a packet if
	// 1) Case for no data packets in line
	// 2) Case : destination does not exist in the neighbor table
	//	3) Case: No timing info is available for the destination


}
 */


BOOL DataTransmissionHandler::UpdateNeighborsWakeUpSlot(UINT16 dest, UINT8 _skipperiods){
	bool rv;
	Neighbor_t* neighborEntry =  g_NeighborTable.GetNeighborPtr(dest);
	if (neighborEntry != NULL) {
		if (neighborEntry->MACAddress != dest) {
			DEBUG_OMAC_UNWUS_PRINTF("DataTransmissionHandler::ScheduleDataPacket() incorrect neighbor returned\r\n");
			return FALSE;
		}
		UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		UINT64 neighborTimeinTicks = g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(dest, g_OMAC.m_Clock.GetCurrentTimeinTicks());
		if (neighborTimeinTicks == 0){ //Case: No timing info is available for the destination
			//Keep the packet but do not schedule the data packet
			DEBUG_OMAC_UNWUS_PRINTF("DataTransmissionHandler::ScheduleDataPacket() neighbor time is not known!!!\r\n");
			return FALSE;
		}
		UINT64 neighborSlot = g_OMAC.m_omac_scheduler.GetSlotNumberfromTicks(neighborTimeinTicks);
		if(neighborSlot >= neighborEntry->nextwakeupSlot) {
			rv = g_OMAC.m_omac_scheduler.m_DataReceptionHandler.UpdateSeedandCalculateWakeupSlot(neighborEntry->nextwakeupSlot, neighborEntry->nextSeed, neighborEntry->mask, neighborEntry->seedUpdateIntervalinSlots, neighborSlot );
			if (!rv) { // rv update failed something is wrong with the neighbor, refrain from sending to the neighbor
				return FALSE;
			}
		}
		while(_skipperiods > 0) {
			--_skipperiods;
			neighborSlot = neighborEntry->nextwakeupSlot;
			g_OMAC.m_omac_scheduler.m_DataReceptionHandler.UpdateSeedandCalculateWakeupSlot(neighborEntry->nextwakeupSlot, neighborEntry->nextSeed, neighborEntry->mask, neighborEntry->seedUpdateIntervalinSlots, neighborSlot );
		}
		return TRUE;
	}
	else { //Case : destination does not exist in the neighbor table
		DEBUG_OMAC_UNWUS_PRINTF("Cannot find nbr %u\r\n", dest);
		return FALSE;
	}
}


