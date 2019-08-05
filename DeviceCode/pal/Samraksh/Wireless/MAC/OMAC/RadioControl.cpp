/*
 * RadioControl.cpp
 *
 *  Created on: Sep 5, 2012
 *      Author: Mukundan
 *
 *  Modified on: Oct 30, 2015
 *  	Authors: Bora Karaoglu; Ananth Muralidharan
 *
 *  Copyright The Samraksh Company
 */

#include <tinyhal.h>
#include <Samraksh/Radio_decl.h>
#include <Samraksh/Message.h>
#include <Samraksh/MAC/OMAC/RadioControl.h>
#include <Samraksh/MAC/OMAC/OMAC.h>

//#include "netmf_pwr_wakelock.h"
#define WLO_OMAC 0x00000010
extern void WakeLock(uint32_t lock);
extern void WakeUnlock(uint32_t lock);

extern OMACType g_OMAC;


#define LOCALSKEW 1
//#define DEBUG_RADIO_STATE 1


/*
 *
 */
DeviceStatus RadioControl_t::Initialize(){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_EnableOutputPin(RADIOCONTROL_SEND_PIN, FALSE);
	CPU_GPIO_SetPinState( RADIOCONTROL_SEND_PIN, FALSE );
	CPU_GPIO_EnableOutputPin(RADIOCONTROL_SENDTS_PIN, FALSE);
	CPU_GPIO_SetPinState( RADIOCONTROL_SENDTS_PIN, FALSE );
	CPU_GPIO_EnableOutputPin(RADIOCONTROL_STATEPIN, FALSE);
	CPU_GPIO_EnableOutputPin(RC_TX_TIMESYNCREQ, FALSE);
	CPU_GPIO_EnableOutputPin(RC_TX_DATA, FALSE);

	CPU_GPIO_EnableOutputPin(OMAC_DRIVING_RADIO_SEND, FALSE);
	CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, FALSE );
	CPU_GPIO_EnableOutputPin(OMAC_DRIVING_RADIO_RECV, TRUE);
	CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_RECV, FALSE );
	CPU_GPIO_EnableOutputPin(OMAC_DRIVING_RADIO_SLEEP, TRUE);
	CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SLEEP, FALSE );

#endif
#if OMAC_DEBUG_PRINTF_RADIOCONTROL_SEND
	packet_seq_num = 0;
#endif
	stayOn = false;
	next_piggybacked_extendedneighborinfo_index = 0;
	return DS_Success;
}

DeviceStatus RadioControl_t::Uninitialize(){
	return Stop();
}

/*
 *
 */
DeviceStatus RadioControl_t::Preload(RadioAddress_t address, Message_15_4_t * msg, UINT16 size){
	static UINT8 seqNumber = 0;
	UINT8 finalSeqNumber = 0;

	IEEE802_15_4_Header_t *header = msg->GetHeader();
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
	header->fcf.fcfWordValue = FCF_WORD_VALUE;
	finalSeqNumber = g_OMAC.GetMyAddress() ^ 0xAA;
	finalSeqNumber += ((g_OMAC.GetMyAddress() >> 8) ^ 0x55);
	finalSeqNumber += seqNumber;
	header->dsn = finalSeqNumber;
	//header->srcpan = SRC_PAN_ID;
	header->destpan = DEST_PAN_ID;
	/*if(g_OMAC.GetMyAddress() == 6846){
		header->dest = 0x0DB1;
	}
	else{
		header->dest = 0x1ABE;
	}*/
	//6846(0x1ABE) is the receiver
	/*if(g_OMAC.GetMyAddress() != 0x1ABE){
		header->dest = 0x1ABE;
	}
	else{
		header->dest = 0x1111;
	}*/
	header->dest = address;
	header->src = g_OMAC.GetMyAddress();
	//header->retryAttempt = 0;
	seqNumber++;

	IEEE802_15_4_Metadata* metadata = msg->GetMetaData();
	//header->SetLength(size + sizeof(IEEE802_15_4_Header_t)+sizeof(IEEE802_15_4_Footer_t)+sizeof(IEEE802_15_4_Metadata));
	header->length = (size + sizeof(IEEE802_15_4_Header_t));

	//msg = (Message_15_4_t *) CPU_Radio_Preload(g_OMAC.radioName, (void *)msg, size+sizeof(IEEE802_15_4_Header_t)+sizeof(IEEE802_15_4_Footer_t)+sizeof(IEEE802_15_4_Metadata));
	msg = (Message_15_4_t *) CPU_Radio_Preload(g_OMAC.radioName, (void *)msg, size+sizeof(IEEE802_15_4_Header_t));
	return DS_Success;
}


/*
 *
 */
DeviceStatus RadioControl_t::Send(RadioAddress_t address, Message_15_4_t* msg, UINT16 size){
#if OMAC_DEBUG_PRINTF_RADIOCONTROL_SEND
	++packet_seq_num;
#endif
	//Check if we can send with timestamping, 4bytes for timestamping + 8 bytes for clock value
	Message_15_4_t* returnMsg;
	if(size == sizeof(softwareACKHeader)){
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, TRUE );
#endif
		returnMsg = (Message_15_4_t *) CPU_Radio_Send(g_OMAC.radioName, msg, size);
		if(returnMsg == msg){
			//OMAC_HAL_PRINTF("Returning success \r\n");
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, FALSE );
#endif
			return DS_Success;
		}
		else{
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, FALSE );
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, TRUE );
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, FALSE );
#endif
			return DS_Fail;
		}

	}
	else{
		PiggybackMessages( msg, size);
		IEEE802_15_4_Header_t *header = msg->GetHeader();
		IEEE802_15_4_Metadata* metadata = msg->GetMetaData();




		header->length = (size);


#ifdef OMAC_DEBUG_GPIO
		if(header->payloadType == MFM_OMAC_TIMESYNCREQ){
			CPU_GPIO_SetPinState( RC_TX_TIMESYNCREQ, TRUE );
		}
		else if(header->payloadType == MFM_DATA){
			CPU_GPIO_SetPinState( RC_TX_DATA, TRUE );
			//OMAC_HAL_PRINTF("RC send; Sending: %d \r\n", (msg->GetPayload())[8]);
		}
#endif

		if( (header->flags & TIMESTAMPED_FLAG) ){
			//Convert TimeStamp to high freq clock
			UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
			UINT64 time_elapsed_since_TS = y - msg->GetMetaData()->GetReceiveTimeStamp();
			UINT64 event_time = HAL_Time_CurrentTicks() - time_elapsed_since_TS;
			//msg->GetMetaData()->SetReceiveTimeStamp((INT64)event_time);
			if((g_OMAC.isSendDone)){//||(g_OMAC.radioName != SI4468_SPI2)){
				//Reset flag just before sending
				g_OMAC.isSendDone = false;
#ifdef OMAC_DEBUG_GPIO
				CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, TRUE ); //Mark 1

				if(OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER != DISABLED_PIN  && g_OMAC.m_omac_scheduler.m_state == I_DATA_SEND_PENDING && g_OMAC.m_omac_scheduler.m_execution_started ){
					CPU_GPIO_SetPinState( OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER, !CPU_GPIO_GetPinState(OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER) );
					CPU_GPIO_SetPinState( OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER, !CPU_GPIO_GetPinState(OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER) );
				}


#endif
				returnMsg = (Message_15_4_t *) CPU_Radio_Send_TimeStamped(g_OMAC.radioName, msg, size, (UINT32)event_time);

#ifdef OMAC_DEBUG_GPIO
				if(OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER != DISABLED_PIN  && g_OMAC.m_omac_scheduler.m_state == I_DATA_SEND_PENDING && g_OMAC.m_omac_scheduler.m_execution_started ){
					CPU_GPIO_SetPinState( OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER, !CPU_GPIO_GetPinState(OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER) );
					CPU_GPIO_SetPinState( OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER, !CPU_GPIO_GetPinState(OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER) );
				}


#endif

			}
			else{
				goto endOfSend;
			}
		}
		else {
			if((g_OMAC.isSendDone)){//||(g_OMAC.radioName != SI4468_SPI2)){
				//Reset flag just before sending
				g_OMAC.isSendDone = false;
#ifdef OMAC_DEBUG_GPIO
				CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, TRUE );
#endif
				returnMsg = (Message_15_4_t *) CPU_Radio_Send(g_OMAC.radioName, msg, size);
			}
			else{
				goto endOfSend;
			}
		}

#ifdef OMAC_DEBUG_GPIO
		if(header->payloadType == MFM_OMAC_TIMESYNCREQ){
			CPU_GPIO_SetPinState( RC_TX_TIMESYNCREQ, FALSE );
		}
		else if(header->payloadType == MFM_DATA){
			CPU_GPIO_SetPinState( RC_TX_DATA, FALSE );
		}
#endif

#if OMAC_DEBUG_PRINTF_RADIOCONTROL_SEND_EXCEPT_DISCO
		if(header->payloadType != MFM_OMAC_DISCOVERY){
			hal_printf("\r\nRadioControl_t::Send sent a packet seq = %u dest = %u type =%u flags = %u\r\n",packet_seq_num,  header->dest, header->payloadType, header->flags);
		}
#endif

		if(returnMsg != NULL){
			//OMAC_HAL_PRINTF("Returning success \r\n");
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, FALSE );
#endif
			return DS_Success;
		}
		else{
			g_OMAC.isSendDone=true;
		}
		endOfSend:

		//MS fix for lockup on send failure.
		//This part means the radio rejected our packet.
		//So senddone is set to true, because we didnt even begin sending

		//OMAC_HAL_PRINTF("Returning DS_Fail \r\n");
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, FALSE );
		CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, TRUE );
		CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SEND, FALSE );
#endif
		return DS_Fail;
	}
}


bool RadioControl_t::PiggybackMessages(Message_15_4_t* msg, UINT16 &size){
	bool rv = false;
	IEEE802_15_4_Header_t *header = msg->GetHeader();
	IEEE802_15_4_Metadata* metadata = msg->GetMetaData();

	if(!(header->flags & MFM_TIMESYNC_FLAG) && (header->payloadType != MFM_TIMESYNC)) {
		rv = PiggybackTimeSyncMessage(msg, size) || rv ;
	}
	if( header->payloadType == MFM_OMAC_TIMESYNCREQ && !(header->flags & MFM_DISCOVERY_FLAG) && (header->payloadType != MFM_OMAC_DISCOVERY)) {
		Neighbor_t* neigh_ptr = g_NeighborTable.GetNeighborPtr(header->dest);
		if(header->dest != 0 && header->dest != RADIO_BROADCAST_ADDRESS && neigh_ptr!= NULL && neigh_ptr->IsSendingMyScheduleNeeded() ){
			rv =  PiggybackDiscoMessage(msg, size) || rv;
		}
	}
#if OMAC_DEBUG_SEND_EXTENDEDMACINfo
	if(header->payloadType == MFM_OMAC_DISCOVERY){
		rv = PiggybackEntendedMACInfoMsg(msg, size) || rv ;
	}
#endif
	return rv;
}

bool RadioControl_t::PiggybackTimeSyncMessage(Message_15_4_t* msg, UINT16 &size){
	const int crc_size = 2;			//used in Radio driver's RF231Radio::Send_TimeStamped
	const int timestamp_size = TIMESTAMP_SIZE;	//used in Radio driver's RF231Radio::Send_TimeStamped
	int additional_overhead = crc_size;

	UINT64 event_time,y;
	UINT32 y_lo, event_time_lo;
	RadioAddress_t dest = 0;
	IEEE802_15_4_Header_t *header = msg->GetHeader();
	IEEE802_15_4_Metadata* metadata = msg->GetMetaData();

	if((header->flags & MFM_TIMESYNC_FLAG) || (header->payloadType == MFM_TIMESYNC)){ //Already embedded
		return false;
	}

	if( (header->flags & TIMESTAMPED_FLAG) ){ //Check if already stamped
		event_time = msg->GetMetaData()->GetReceiveTimeStamp();
	}
	else{ //Otherwise calculate it . Will be added later add it
		additional_overhead += timestamp_size;
		//g_OMAC.m_Clock.CreateSyncPointBetweenClocks();
		event_time = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	}

	if( (size-sizeof(IEEE802_15_4_Header_t)) < IEEE802_15_4_MAX_PAYLOAD - (sizeof(TimeSyncMsg)+additional_overhead) ){
		TimeSyncMsg * tmsg = (TimeSyncMsg *) (msg->GetPayload()+(size-sizeof(IEEE802_15_4_Header_t)));
		// Event time already exists in the packet (either just added or added by the C# application earlier)
		// Adjust the time stamp of the timesync packet accordingly.
		msg->GetMetaData()->SetReceiveTimeStamp((INT64)event_time);
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( RADIOCONTROL_SENDTS_PIN, TRUE );
		CPU_GPIO_SetPinState( RADIOCONTROL_SENDTS_PIN, FALSE );
		CPU_GPIO_SetPinState( RADIOCONTROL_SENDTS_PIN, TRUE );
		CPU_GPIO_SetPinState( RADIOCONTROL_SENDTS_PIN, FALSE );
#endif
		header->flags = ((header->flags | TIMESTAMPED_FLAG));
		y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		y_lo = y & 0xFFFFFFFF;
		event_time_lo = event_time & 0xFFFFFFFF;
		y = y - ( y_lo - event_time_lo );
		g_OMAC.m_omac_scheduler.m_TimeSyncHandler.CreateMessage(tmsg, y);
		dest = header->dest;
		if(dest != 0 && dest != RADIO_BROADCAST_ADDRESS) {
			DeviceStatus ds = g_NeighborTable.RecordTimeSyncSent(dest,y);
			if(ds != DS_Success && dest != RADIO_BROADCAST_ADDRESS){
#ifdef OMAC_DEBUG_PRINTF
				OMAC_HAL_PRINTF("RadioControl_t::PiggybackTimeSyncMessage RecordTimeSyncSent failure; address: %d; line: %d \r\n", dest, __LINE__);
#endif
			}
		}
		msg->GetHeader()->flags = ((UINT8)(msg->GetHeader()->flags | MFM_TIMESYNC_FLAG));
		size += sizeof(TimeSyncMsg);
		return true;
	}
	return false;
}

bool RadioControl_t::PiggybackDiscoMessage(Message_15_4_t* msg, UINT16 &size){
	const int crc_size = 2;			//used in Radio driver's RF231Radio::Send_TimeStamped
	//const int timestamp_size = TIMESTAMP_SIZE;	//used in Radio driver's RF231Radio::Send_TimeStamped
	int additional_overhead = crc_size;
	IEEE802_15_4_Header_t *header = msg->GetHeader();
	IEEE802_15_4_Metadata* metadata = msg->GetMetaData();

	if((header->flags & MFM_DISCOVERY_FLAG) || (header->payloadType == MFM_OMAC_DISCOVERY)){ //Already embedded
		return false;
	}

	if( (size-sizeof(IEEE802_15_4_Header_t)) < IEEE802_15_4_MAX_PAYLOAD - (sizeof(DiscoveryMsg_t)+additional_overhead) ){
		DiscoveryMsg_t * tmsg = (DiscoveryMsg_t *) (msg->GetPayload()+(size-sizeof(IEEE802_15_4_Header_t)));
		g_OMAC.m_omac_scheduler.m_DiscoveryHandler.CreateMessage(tmsg);
		msg->GetHeader()->flags = ((UINT8)(msg->GetHeader()->flags | MFM_DISCOVERY_FLAG));
		size += sizeof(DiscoveryMsg_t);
		return true;
	}
	return false;
}

bool RadioControl_t::PiggybackEntendedMACInfoMsg(Message_15_4_t* msg, UINT16 &size){
	const int crc_size = 2;			//used in Radio driver's RF231Radio::Send_TimeStamped
	int additional_overhead = crc_size;
	IEEE802_15_4_Header_t *header = msg->GetHeader();
	IEEE802_15_4_Metadata* metadata = msg->GetMetaData();

	if((header->flags & MFM_EXTENDED_MAC_INFO_FLAG)){ //Already embedded
		return false;
	}
	if( (size-sizeof(IEEE802_15_4_Header_t)) < IEEE802_15_4_MAX_PAYLOAD - (sizeof(EntendedMACInfoMsgSummary)+additional_overhead) ){

		EntendedMACInfoMsgSummary * tmsg = (EntendedMACInfoMsgSummary *) (msg->GetPayload()+(size-sizeof(IEEE802_15_4_Header_t)));
		tmsg->NumTotalEntries = g_NeighborTable.NumberOfNeighborsTotal();
		tmsg->NNeigh_AFUL = g_NeighborTable.PreviousNumberOfNeighbors();
		tmsg->NumEntriesInMsg = 0;
		msg->GetHeader()->flags = ((UINT8)(msg->GetHeader()->flags | MFM_EXTENDED_MAC_INFO_FLAG));
		size += sizeof(EntendedMACInfoMsgSummary);

		UINT16 remSize = IEEE802_15_4_MAX_PAYLOAD - additional_overhead - (size-sizeof(IEEE802_15_4_Header_t));
		UINT8 numNfits = (UINT8)(remSize/sizeof(MACNeighborInfo));
		if(numNfits > tmsg->NumTotalEntries) numNfits = tmsg->NumTotalEntries;
		if(numNfits > 0 ){
			MACNeighborInfo * macinfo_msg;
			UINT8 starting_next_piggybacked_extendedneighborinfo_index = next_piggybacked_extendedneighborinfo_index;
			for(UINT8 i=0; i < numNfits ; ++i, ++next_piggybacked_extendedneighborinfo_index){
				if(next_piggybacked_extendedneighborinfo_index >= tmsg->NumTotalEntries)  next_piggybacked_extendedneighborinfo_index = 0;
				if( (size-sizeof(IEEE802_15_4_Header_t)) < IEEE802_15_4_MAX_PAYLOAD - (sizeof(MACNeighborInfo)+additional_overhead) ){
					macinfo_msg = (MACNeighborInfo *) (msg->GetPayload()+(size-sizeof(IEEE802_15_4_Header_t)));
					macinfo_msg->MACAddress 						=  g_NeighborTable.Neighbor[next_piggybacked_extendedneighborinfo_index].MACAddress;
					macinfo_msg->neighborStatus 					=  g_NeighborTable.Neighbor[next_piggybacked_extendedneighborinfo_index].neighborStatus;
					macinfo_msg->IsAvailableForUpperLayers 			=  g_NeighborTable.Neighbor[next_piggybacked_extendedneighborinfo_index].IsAvailableForUpperLayers;
					macinfo_msg->IsMyScheduleKnown 				=  g_NeighborTable.Neighbor[next_piggybacked_extendedneighborinfo_index].IsMyScheduleKnown;
					macinfo_msg->NumInitializationMessagesSent 		=  g_NeighborTable.Neighbor[next_piggybacked_extendedneighborinfo_index].NumInitializationMessagesSent;
					macinfo_msg->NumTimeSyncMessagesRecv 			=  g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(g_NeighborTable.Neighbor[next_piggybacked_extendedneighborinfo_index].MACAddress) ;
					size += sizeof(MACNeighborInfo);
					++tmsg->NumEntriesInMsg;
				}
			}
			MACNeighborLinkInfo * macinfo_msg2;
			for(UINT8 i=0; i < numNfits ; ++i, ++starting_next_piggybacked_extendedneighborinfo_index){
				if(starting_next_piggybacked_extendedneighborinfo_index >= tmsg->NumTotalEntries)  starting_next_piggybacked_extendedneighborinfo_index = 0;
				if( (size-sizeof(IEEE802_15_4_Header_t)) < IEEE802_15_4_MAX_PAYLOAD - (sizeof(MACNeighborLinkInfo)+additional_overhead) ){
					macinfo_msg2 = (MACNeighborLinkInfo *) (msg->GetPayload()+(size-sizeof(IEEE802_15_4_Header_t)));
					macinfo_msg2->SendLink 						=  g_NeighborTable.Neighbor[next_piggybacked_extendedneighborinfo_index].SendLink;
					macinfo_msg2->ReceiveLink 						=  g_NeighborTable.Neighbor[next_piggybacked_extendedneighborinfo_index].ReceiveLink;
					size += sizeof(MACNeighborLinkInfo);
				}
			}
		}

		return true;
	}
	return false;
}


//DeviceStatus RadioControl_t::Receive(Message_15_4_t * msg, UINT16 size){
//		return DS_Success;
//}

/*
 *
 */
DeviceStatus RadioControl_t::Stop(){
	if(stayOn) {
		return DS_Success;
	}
	else {
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SLEEP, TRUE );
#endif
		WakeUnlock(WLO_OMAC);
		DeviceStatus returnVal = CPU_Radio_Sleep(g_OMAC.radioName,0);

		if(returnVal == DS_Success) {
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SLEEP, FALSE );
			CPU_GPIO_SetPinState( RADIOCONTROL_STATEPIN, FALSE );
#endif
		}
		else {
			//ASSERT_SP(0);
			//OMAC_HAL_PRINTF("RadioControl_t::Stop Radio did not go to sleep \r\n");
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SLEEP, FALSE );
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SLEEP, TRUE );
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_SLEEP, FALSE );
#endif
		}

		return returnVal;
	}
}

/*
 *
 */
DeviceStatus RadioControl_t::StartRx(){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_RECV, TRUE );
#endif

	//MS: Hack to speedup, why do all this if we are already ON?
	if(stayOn) {
			return DS_Success;
	}
	else {
		WakeLock(WLO_OMAC);
		DeviceStatus returnVal = CPU_Radio_TurnOnRx(g_OMAC.radioName);
		if(returnVal == DS_Success){
	#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState( RADIOCONTROL_STATEPIN, TRUE );
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_RECV, FALSE );
	#endif
		}
		else{
	#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_RECV, FALSE );
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_RECV, TRUE );
			CPU_GPIO_SetPinState( OMAC_DRIVING_RADIO_RECV, FALSE );
	#endif
		}
		return returnVal;
	}
}


