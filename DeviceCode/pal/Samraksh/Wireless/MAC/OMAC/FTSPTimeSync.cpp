#include "DiscoveryHandler.h"
#include <Samraksh/Message.h>
#include "RadioControl.h"

extern RadioControl_t g_omac_RadioControl;

void FTSPTimeSync::Initialize(){
	m_skew = 0;
	m_localAverage = 0;
	m_offsetAverage = 0;
}

float  FTSPTimeSync::GetSkew() { return m_skew; }
UINT32 FTSPTimeSync::GetOffset() { return m_offsetAverage; }
UINT32 FTSPTimeSync::GetSyncPoint() { return m_localAverage; }

bool FTSPTimeSync::IsInTransitionPeriod(RadioAddress_t nodeID){
	/*	UINT8 idx;
		if (nbrToIdx(nodeID, &idx) == SUCCESS) {
			return ((m_beaconTable[idx].size < ENTRY_VALID_LIMIT) &&
			m_beaconTable[idx].isInTransition);
		} else {
			return FALSE;
		}
	 */
}

void FTSPTimeSync::SetCounterOffset(UINT16 counterOffset){
	//m_timeSyncBeacon->counterOffset = counterOffset;
}

bool FTSPTimeSync::IsSynced(RadioAddress_t nodeID) {

	//return (IsSynced(nodeID) == SUCCESS);
	return FALSE;
}

void FTSPTimeSync::InsertBeacon(Message_15_4_t *ptr){
	/*Message_15_4_t *bufPtr;

	bufPtr = call MessagePool.get();
	if (bufPtr != NULL) {
	 *bufPtr = *ptr;
		call MessageQueue.enqueue(bufPtr);
	}


#ifdef ORIGINAL_OMAC
		signal TimeSyncInfo.receivedBeaconAck(SUCCESS);
#endif

	m_receivedPiggybackBeacon = TRUE;
	 */
}

bool FTSPTimeSync::IsProccessingBeacon(){
	/*bool retVal;
	retVal = (m_processedMsg != NULL);
	return retVal;*/
	return FALSE;
}

void FTSPTimeSync::SetSeedInfo(UINT16 seed, UINT32 nextFrame) {
	//m_timeSyncBeacon->nextFrame = nextFrame;
	//m_timeSyncBeacon->seed = seed;

}
DeviceStatus FTSPTimeSync::Send(RadioAddress_t address, Message_15_4_t  * msg, UINT16 size, UINT64 event_time){
	IEEE802_15_4_Header_t * header = msg->GetHeader();
	//UINT8 * payload = msg->GetPayload();
	header->dest= address;
	header->type=MFM_TIMESYNC;

	g_omac_RadioControl.Send_TimeStamped(address,msg,sizeof(Message_15_4_t),event_time);
}

void SendAckHandler(Message_15_4_t* msg, UINT8 len, NetOpStatus success){

}

DeviceStatus FTSPTimeSync::Receive(Message_15_4_t* msg, void* payload, UINT8 len){

	return DS_Success;
}

#ifdef ORIGINAL_OMAC
async command error_t TimeSyncInfo.sendBeaconAck(am_addr_t dest, uint8_t dsn) {
	cc2420_header_t* hdrPtr = call CC2420PacketBody.getHeader( &m_timeSyncBeaconBuffer );
	hdrPtr->dsn = dsn;
	hdrPtr->dest = dest;
	return beacon(dest, &m_timeSyncBeaconBuffer);
}
#endif

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
			//	printf("ofset=%ld lclAvg=%lu lcl=%lu, sk=%ld\n",
			//	    ptr->offsetAvg, ptr->localAvg, *time, (int32_t)(ptr->skew * 1000));
 *time += ptr->offsetAvg
					+ (int32_t)(ptr->skew * (int32_t)(*time - ptr->localAvg));
			} else {
				result = FAIL;
				printf("Error in local2Global. should not happen\n");
			}
		}

		return result;
	}

	async command error_t GlobalTime.global2Local(uint32_t *time, am_addr_t nodeID)
	{
		if (nodeID == TOS_NODE_ID) {
			printf("global2Local myself\n");
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
				printf("Error in global2Local. should not happen\n");
				return FAIL;
			}
		} else {
			printf("global2Local failed\n");
			return FAIL;
		}
	}
 */
