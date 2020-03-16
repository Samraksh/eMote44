/*
 * OMACTimeSync.cpp
 *
 *  Created on:
 *      Author: Bora Karaoglu
 *
 *  Modified on: Oct 30, 2015
 *  	Authors: Bora Karaoglu; Ananth Muralidharan
 *
 *  Copyright The Samraksh Company
 */

#include <Samraksh/Message.h>
#include <Samraksh/MAC/OMAC/OMAC.h>

#define FUDGEFACTOR 10000		//in 100ns, a value of 1000 =100 microseconds

#define LOCALSKEW 1
extern OMACType g_OMAC;


INT64 GlobalTime::offset =0;
float GlobalTime::skew =0;
UINT16 GlobalTime::leader = RADIO_BROADCAST_ADDRESS;
BOOL GlobalTime::synced=FALSE;

#define MIN_TICKS_DIFF_BTW_TSM 8000000



void PublicTimeSyncCallback(void * param){
	g_OMAC.m_omac_scheduler.m_TimeSyncHandler.PostExecuteEvent();
}

void OMACTimeSync::Initialize(UINT8 radioID, UINT8 macID){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_EnableOutputPin(TIMESYNC_GENERATE_MESSAGEPIN, FALSE);
	CPU_GPIO_SetPinState(TIMESYNC_GENERATE_MESSAGEPIN, FALSE);
	CPU_GPIO_EnableOutputPin(TIMESYNC_RECEIVEPIN, FALSE);
	CPU_GPIO_SetPinState(TIMESYNC_RECEIVEPIN, FALSE);
	CPU_GPIO_EnableOutputPin(OMAC_TIMESYNC_NEXT_EVENT, FALSE);
	CPU_GPIO_SetPinState(OMAC_TIMESYNC_NEXT_EVENT, FALSE);
#endif


	RadioID = radioID;
	MacID = macID;

	m_messagePeriod = g_OMAC.m_Clock.ConvertMicroSecstoTicks(SENDER_CENTRIC_PROACTIVE_TIMESYNC_REQUEST_INMICS) ;//Time period in ticks
	m_globalTime.Init();

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_TIMESYNC, 0, 1 * MILLISECINMICSEC , TRUE, FALSE, PublicTimeSyncCallback, OMACClockSpecifier); //1 sec Timer in micro seconds
	////ASSERT_SP(rm == TimerSupported);

	m_inter_clock_offset = 0;

}

UINT64 OMACTimeSync::NextEvent(){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(OMAC_TIMESYNC_NEXT_EVENT, TRUE);
#endif
	Neighbor_t* sn = NULL;
	UINT64 y;
	UINT16 nextEventsSlot = 0;
	UINT64 nextEventsMicroSec = 0;
	nextEventsSlot = NextEventinSlots();
	while(true){
		sn = g_NeighborTable.GetCritalSyncNeighborWOldestSyncPtr(
				g_OMAC.m_Clock.GetCurrentTimeinTicks()
				, m_messagePeriod
				, g_OMAC.m_Clock.ConvertMicroSecstoTicks(FORCE_REQUESTTIMESYNC_INMICS)
				, g_OMAC.m_Clock.ConvertMicroSecstoTicks(INITIALIZATION_TIMESYNC_INTERVAL_INMICS)
		);
		if(sn != NULL) {
			y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
			Send(sn->MACAddress);
//			if(sn->IsInitializationTimeSamplesNeeded()){ //If the node needs initialization
//				Send(sn->MACAddress);
//			}
//			else if(y - sn->LastTimeSyncSendTime >= m_messagePeriod){
//				Send(sn->MACAddress);
//			}
//			else{
//				break;
//			}
		}
		else{
			break;
		}
	}
	if(sn != NULL) {
		nextEventsMicroSec = g_OMAC.m_Clock.ConvertTickstoMicroSecs ( m_messagePeriod - (y - sn->LastTimeSyncSendTime) );
	}
	/*if(HARDWARE_ACKS){
		nextEventsMicroSec += (1*EXTENDED_MODE_TX_DELAY_MICRO);
	}*/
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(OMAC_TIMESYNC_NEXT_EVENT, FALSE);
#endif
	return(nextEventsMicroSec);
}
/*
 *
 */
UINT16 OMACTimeSync::NextEventinSlots(){
	//	UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	//	UINT64 currentSlotNum = g_OMAC.m_omac_scheduler.GetSlotNumber();
	//	Neighbor_t* sn = g_NeighborTable.GetCritalSyncNeighborWOldestSyncPtr(y, m_messagePeriod,g_OMAC.m_Clock.ConvertMicroSecstoTicks(FORCE_REQUESTTIMESYNC_INMICS) );
	//	if ( sn == NULL ) return ((UINT16) MAX_UINT32);
	//
	//	else if( y - sn->LastTimeSyncSendTime >= m_messagePeriod) { //Already passed the time. schedule send immediately
	//		return 0;
	//	}
	//	else {
	//		UINT64 remslots = (m_messagePeriod - (y - sn->LastTimeSyncSendTime) ) / SLOT_PERIOD;
	//	}
	return 0;
}


/*
 *
 */
void OMACTimeSync::ExecuteEvent(){
	//BK: This will be handled with the non_interrupt timer
	//	Neighbor_t* sn;
	//	while(NextEventinSlots() == 0){
	//		sn = g_NeighborTable.GetCritalSyncNeighborWOldestSyncPtr(g_OMAC.m_Clock.GetCurrentTimeinTicks(),m_messagePeriod,FORCE_REQUESTTIMESYNC_INTICKS);
	//		Send(sn->MacAddress);
	//	}
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(VIRT_TIMER_OMAC_TIMESYNC);
	rm = VirtTimer_Start(VIRT_TIMER_OMAC_TIMESYNC);
	if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
		PostExecuteEvent();
	}
	//PostExecuteEvent();
}

/*
 *
 */
/*UINT8 OMACTimeSync::ExecuteEventDone()
{
	return 0;
}*/

/*
 *
 */
void OMACTimeSync::PostExecuteEvent(){
	g_OMAC.m_omac_scheduler.PostExecution();
}

void OMACTimeSync::FailsafeStop(){
	VirtTimer_Stop(VIRT_TIMER_OMAC_TIMESYNC);
}

/*
 *
 */
//DeviceStatus OMACTimeSync::Send(RadioAddress_t address, Message_15_4_t  * msg, UINT16 size, UINT64 event_time){
BOOL OMACTimeSync::Send(RadioAddress_t address){
	bool request_TimeSync;
	UINT64 lastTimeSyncRecv = g_NeighborTable.GetLastTimeSyncRecv(address);

	TimeSyncRequestMsg * tsreqmsg;
	BOOL rs = false;
	UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();

	//ASSERT_SP( y > lastTimeSyncRecv); //Ensure no rollover

	if( y - lastTimeSyncRecv > g_OMAC.m_Clock.ConvertMicroSecstoTicks(FORCE_REQUESTTIMESYNC_INMICS) ){
		request_TimeSync = true;
	}
	else{
		request_TimeSync = false;
	}

	if (m_globalTime.regressgt2.NumberOfRecordedElements(address) >=2  ){
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( TIMESYNC_GENERATE_MESSAGEPIN, TRUE );
#endif
		IEEE802_15_4_Header_t * header = m_timeSyncMsgBuffer.GetHeader();
		tsreqmsg = (TimeSyncRequestMsg *) m_timeSyncMsgBuffer.GetPayload();
		tsreqmsg->request_TimeSync = request_TimeSync;

		rs = g_OMAC.Send(address, MFM_OMAC_TIMESYNCREQ, tsreqmsg, sizeof(TimeSyncRequestMsg));
		if(!rs){
#ifdef OMAC_DEBUG_PRINTF
			//OMAC_HAL_PRINTF("OMACTimeSync::Send failed. Addr=%d\r\n", address);
#endif
			//	return rs;
		}
#if OMAC_TSYNC_DEBUG_PRINTF_PACKET_ENQUE
		if(rs){
			hal_printf("TSREQ ENQUE SUCCESS d: %d\r\n", address);
		}
		else{
			hal_printf("TSREQ ENQUE FAIL d: %d\r\n", address);
		}
#endif
#ifdef OMAC_DEBUG_PRINTF
		//OMAC_HAL_PRINTF("TS Send: %d, LTime: %lld  \r\n",m_seqNo, y);
		//OMAC_HAL_PRINTF("TS Send: %d, LTime: %s  \r\n",m_seqNo, l2s(y,0));
#endif
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( TIMESYNC_GENERATE_MESSAGEPIN, FALSE );
#endif
	}

	DeviceStatus ds = g_NeighborTable.RecordTimeSyncRequestSent(address, y);
	if(ds != DS_Success){
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("OMACTimeSync::Send RecordTimeSyncRequestSent failure; address: %d; line: %d \r\n", address, __LINE__);
#endif
	}
#if OMAC_TSYNC_DEBUG_PRINTF_PACKET_ENQUE
	if(ds == DS_Success){
		hal_printf("RTSRS SUCCESS d: %d\r\n", address);
	}
	else{
		hal_printf("RTSRS FAIL d: %d\r\n", address);
	}

#endif

	return rs;
}

DeviceStatus OMACTimeSync::ReceiveTSReq(RadioAddress_t msg_src, TimeSyncRequestMsg* rcv_msg){

	//Determine if timesync is requested, schedule sending a message back to the source
	if(rcv_msg->request_TimeSync){
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("OMACTimeSync::ReceiveTSReq. Sending to %d \r\n", msg_src);
#endif
		this->Send(msg_src);
	}
	return DS_Success;
}



void OMACTimeSync::CreateMessage(TimeSyncMsg* timeSyncMsg, UINT64 curticks, bool request_TimeSync) {
	//timeSyncMsg->timesyncIdentifier = 50529027; //0x03030303
	timeSyncMsg->localTime0 = (UINT32) curticks;
	timeSyncMsg->localTime1 = (UINT32) (curticks>>32);
	//timeSyncMsg->request_TimeSync = request_TimeSync;
	//timeSyncMsg->seqNo = m_seqNo++;
}

/*
 *
 */
DeviceStatus OMACTimeSync::Receive(RadioAddress_t msg_src, TimeSyncMsg* rcv_msg, UINT64 SenderDelay, UINT64 ReceiveTS){
	bool TimerReturn;
	DeviceStatus ds = DS_Fail;
	//RadioAddress_t msg_src = msg->GetHeader()->src;
	UINT64 y,neighborscurtime;

	//	if(rcv_msg->timesyncIdentifier != 50529027 ){
	//		y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	//		neighborscurtime = m_globalTime.Neighbor2LocalTime(msg_src,y);
	//		//ASSERT_SP(0);
	//	}
	//UINT64 EventTime = PacketTimeSync_15_4::EventTime(msg,len);
	//TimeSyncMsg* rcv_msg = (TimeSyncMsg *) msg->GetPayload();
	ReceiveTS = ReceiveTS - TIME_BETWEEN_TX_RX_TS_TICKS;
	UINT64 rcv_ltime;
	INT64 l_offset;
	rcv_ltime = (((UINT64)rcv_msg->localTime1) <<32) + rcv_msg->localTime0 + SenderDelay;
	if(ReceiveTS > rcv_ltime){
		l_offset = ReceiveTS - rcv_ltime;
		l_offset = l_offset*-1;
		hal_printf("\r1:ReceiveTS=%s ",l2s(ReceiveTS/48,0));
		hal_printf("rcv=%s ",l2s(rcv_ltime/48,0));
		hal_printf("sdelay=%s ",l2s(SenderDelay/48,0));
		hal_printf("offset=-%s\r\n",l2s((ReceiveTS - rcv_ltime)/48,0));
	}
	else{
		l_offset = rcv_ltime - ReceiveTS;	
		hal_printf("\r2:ReceiveTS=%s ",l2s(ReceiveTS/48,0));
		hal_printf("rcv=%s ",l2s(rcv_ltime/48,0));
		hal_printf("sdelay=%s ",l2s(SenderDelay/48,0));
		hal_printf("offset=%s\r\n",l2s((rcv_ltime - ReceiveTS)/48,0));
	}
	
	//l_offset = 0;
	//	if((m_globalTime.regressgt2.LastRecordedTime(msg_src) >= rcv_ltime)){
	//		return DS_Fail;
	//	}

	bool is_space_available_in_table = false;
	if(m_globalTime.regressgt2.FindNeighbor(msg_src) != c_bad_nbrIndex){
		is_space_available_in_table = true;
	}
	else if(!m_globalTime.regressgt2.IsTableFull()) {
		m_globalTime.regressgt2.InsertNbrID(msg_src);
		is_space_available_in_table = true;
	}
	else {
		UINT8 index = 0;
		//For each time table entry, if the neighbor does not exist in the neighbortable, kick it from both the time table
		for(UINT8 i =0; i < MAX_NBR; ++i){
			if(INVALID_NBR_ID != g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.samples[i].nbrID){
				if( g_NeighborTable.FindIndexEvenDead( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.samples[i].nbrID, &index) != DS_Success){ //If it does not exist in the neighbortable, delete from the time entires from globalTime Table
					g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.Clean(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.samples[i].nbrID);
					if(!m_globalTime.regressgt2.IsTableFull()) {
						is_space_available_in_table = true;
						m_globalTime.regressgt2.InsertNbrID(msg_src);
						break;
					}
				}
			}
		}

		if(!is_space_available_in_table){ //If still space not available look for stale entries in the neighbor table
			//For each time table entry, if the neighbor is not active in neighbor table, kick it from both the neighbor table and the time table
			for(UINT8 i =0; i < MAX_NBR; ++i){
				if(INVALID_NBR_ID != g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.samples[i].nbrID){
					if( g_NeighborTable.FindIndexEvenDead( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.samples[i].nbrID, &index) == DS_Success){
						if(g_NeighborTable.Neighbor[index].neighborStatus != Alive){ //If it exists in the neighbortable but marked as not alive, delete from the time entires from globalTime Table
							g_NeighborTable.ClearNeighborwIndex(index);
							g_NeighborTable.NumberValidNeighbor ++ ;
							//g_NeighborTable.Neighbor[index].Clear();
							g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.Clean(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.samples[i].nbrID);
							if(!m_globalTime.regressgt2.IsTableFull()) {
								is_space_available_in_table = true;
								m_globalTime.regressgt2.InsertNbrID(msg_src);
								break;
							}
						}
					}
				}
			}
		}

	}

	if(is_space_available_in_table){
		m_globalTime.regressgt2.Insert(msg_src, rcv_ltime, l_offset);
		ds = g_NeighborTable.RecordTimeSyncRecv(msg_src,ReceiveTS);
#if OMAC_DEBUG_PRINTF_TS_RX
		hal_printf("TS Receive from = %u \r\n", msg_src);
		//		g_OMAC.PrintNeighborTable();
		g_OMAC.is_print_neigh_table = true;
#endif
	}
	else{
#if OMAC_DEBUG_PRINTF_TS_RX
		hal_printf("No space to record time sample  msg_src = %u \r\n", msg_src);
		g_OMAC.PrintNeighborTable();
		g_OMAC.is_print_neigh_table = true;

#endif
	}

#ifdef OMAC_DEBUG_PRINTF
	if(ds != DS_Success){
		OMAC_HAL_PRINTF("OMACTimeSync::Receive RecordTimeSyncRecv failure; address: %d; line: %d \r\n", msg_src, __LINE__);
	}
#endif

#ifdef def_Neighbor2beFollowed
	if (msg_src == g_OMAC.Neighbor2beFollowed ){
		if (m_globalTime.regressgt2.NumberOfRecordedElements(msg_src) >=2  ){
			//TimerReturn = false;
		}
#ifdef DEBUG_TSYNC_PIN
		CPU_GPIO_SetPinState( TIMESYNC_RECEIVEPIN, TRUE );
#endif
	}
#endif

#ifdef def_Neighbor2beFollowed
	if (msg_src == g_OMAC.Neighbor2beFollowed ){
		if (m_globalTime.regressgt2.NumberOfRecordedElements(msg_src) >=2  ){
			y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
			neighborscurtime = m_globalTime.Neighbor2LocalTime(msg_src,rcv_ltime);
		}
	}
#endif

#ifdef def_Neighbor2beFollowed
	if (msg_src == g_OMAC.Neighbor2beFollowed ){
		if (m_globalTime.regressgt2.NumberOfRecordedElements(msg_src) >= 2  ){

		}
#ifdef DEBUG_TSYNC_PIN
		CPU_GPIO_SetPinState( TIMESYNC_RECEIVEPIN, FALSE );
#endif
	}
#endif
	return DS_Success;
}



//GlobalTime interface implementation
/*async command uint32_t GlobalTime.getLocalTime()
	{
		return call LocalTime.get();
	}

	async command error_t GlobalTime.getGlobalTime(uint32_t *time, am_addr_t nodeID)
	{
 *time = call GlobalTime.getLocalTime();
		if (nodeID == TOS_NODE_ID) {
			return SUCCESS;
		}
		else {
			return call GlobalTime.local2Global(time, nodeID);
		}
	}

	async command error_t GlobalTime.local2Global(uint32_t *time, am_addr_t nodeID)
	{
		error_t result = is_synced(nodeID);
		if (nodeID == TOS_NODE_ID) {
			return result;
		}
		// the input is the local time. If I'm synced, return the global time. Otherwise, do nothing
		if (result == SUCCESS) {
			uint8_t nbrIdx;
			nbrToIdx(nodeID, &nbrIdx);
			if (nbrIdx != INVALID_INDEX) {
				TableItem *ptr = &(m_beaconTable[nbrIdx]);
			//	printf("ofset=%ld lclAvg=%lu lcl=%lu, sk=%ld \r\n",
			//	    ptr->offsetAvg, ptr->localAvg, *time, (int32_t)(ptr->skew * 1000));
 *time += ptr->offsetAvg
					+ (int32_t)(ptr->skew * (int32_t)(*time - ptr->localAvg));
			} else {
				result = FAIL;
				printf("Error in local2Global. should not happen \r\n");
			}
		}

		return result;
	}

	async command error_t GlobalTime.global2Local(uint32_t *time, am_addr_t nodeID)
	{
		if (nodeID == TOS_NODE_ID) {
			printf("global2Local myself \r\n");
			return is_synced(nodeID);
		} else if (call TimeSyncInfo.isSynced(nodeID)) {
			uint8_t nbrIdx;
			nbrToIdx(nodeID, &nbrIdx);
			if (nbrIdx != INVALID_INDEX) {
				TableItem *ptr = &(m_beaconTable[nbrIdx]);
				uint32_t approxLocalTime = *time - ptr->offsetAvg;
 *time = approxLocalTime
					- (int32_t)(ptr->skew * (int32_t)(approxLocalTime - ptr->localAvg));
				return SUCCESS;
			}
			else {
				printf("Error in global2Local. should not happen \r\n");
				return FAIL;
			}
		} else {
			printf("global2Local failed \r\n");
			return FAIL;
		}
	}
 */
