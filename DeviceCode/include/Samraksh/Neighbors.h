/*
 * Neighbors.h : Defines and implements the MAC/Link layer Neighbor Table
 *
 *Created on: Oct 25, 2012
 *Author: Mukundan Sridharan
 *
 */

#ifndef NEIGHBORS_H_
#define NEIGHBORS_H_

#include <Samraksh/Mac_decl.h>
#include <Samraksh/Buffer.h>

#define NeighborChanged 1
#define Received 0

//#include "TinyCLR_Runtime.h"

#define Data_Send_Buffer_15_4_t_SIZE		2
#define TimeSync_Send_Buffer_15_4_t_SIZE 	1

extern UINT8 MacName;
#define MAX_NEIGHBORS 10
#define INVALID_NEIGHBOR_INDEX 255
#define INVALID_MACADDRESS 0
#define INVALID_MACADDRESS2 65535
#define ISMAC_VALID(x) ( (x==INVALID_MACADDRESS2 || x==INVALID_MACADDRESS) ? false : true)

#define Total_Buffer_SIZE 18


#define NUM_ENFORCED_TSR_PCKTS_BEFORE_DATA_PCKTS 2


#define NEIGHBORS_DEBUG_QUEUE_INSERT 0
//extern void  ManagedCallback(UINT16 arg1, UINT16 arg2);
//#define DEBUG_NEIGHBORTABLE

#if defined(DEBUG_NEIGHBORTABLE)
#define ENABLE_PIN_NB(x,y) CPU_GPIO_EnableOutputPin(x,y)
#define SET_PIN_NB(x,y) CPU_GPIO_SetPinState(x,y)
#define DEBUG_PRINTF_NB(...) hal_printf(__VA_ARGS__)
#else
#define ENABLE_PIN_NB(x,y)
#define SET_PIN_NB(x,y)
#define DEBUG_PRINTF_NB(...) (void)0
#endif

struct PACK Link_t {
	/* AvgRSSI
	 * Exponentially smoothed (with lambda of 0.2) average of   SINR code as reported by the radio.
	 * 		For RF231 has conversion (-91 + AvgRSSI) provides the true value.
	 * 		For SI radio the conversion is already done at the driver and is cast into uint.
	 *	  	  Hence the conversion to get signed value is -1*( (0xFF - AvgRSSI)+1 ).
	 */
	UINT8 AvgRSSI;
	/*LinkQuality:
	 * As reported by the radio.
	 * For RF231, represent a measure of the BER for the corresponding SINR.
	 * For SI radio, not available. Hence a value of 0 is reported.
	 * Exponentially smoothed with lambda of 0.2.
	 */
	//UINT8 LinkQuality;
	/* AveDelay
	 * : Has units of 65.535 (=10^3/(2^19/2^8)) ms. Exponentially smoothed
	 */
	UINT8 AveDelay;

	UINT8 Link_reliability_bitmap;

public:
	Link_t() : AvgRSSI(0), AveDelay(0), Link_reliability_bitmap(0xFF)  {}
	void Initialize(){
		AvgRSSI = 0;
		//LinkQuality = 0;
		AveDelay = 0;
		Link_reliability_bitmap = 0x00;
	}
	void RecordPacketSuccess(bool s){
		Link_reliability_bitmap = Link_reliability_bitmap>>1;
		if(s){
			Link_reliability_bitmap = Link_reliability_bitmap | 0x80;
		}
		else{
			Link_reliability_bitmap = Link_reliability_bitmap & 0x7F;
		}
	}
//	bool IsReliable(){
//		if(Link_reliability_bitmap <= 0x0F ) return false;
//		else return true;
//	}
};

enum NeighborStatus {
	Alive = 0,
	Dead = 1,
	//Suspect = 2,
	NbrStatusError = 0xF
};

typedef struct {
	UINT16 MACAddress;
	UINT8 NumInitializationMessagesSent;	//Count of timesync+disco packets sent per neighbor
	bool IsMyScheduleKnown; //Boolean indicating whether the neighbor has information about the current node's schedule
	bool IsInitializationTimeSamplesNeeded(){
		if(IsSendingMyScheduleNeeded() || NumInitializationMessagesSent < NUM_ENFORCED_TSR_PCKTS_BEFORE_DATA_PCKTS) return true;
		else return false;
	}
	void IncrementNumInitMessagesSent(){
		if( IsInitializationTimeSamplesNeeded() ) {
			++(NumInitializationMessagesSent);
			if(NumInitializationMessagesSent == NUM_ENFORCED_TSR_PCKTS_BEFORE_DATA_PCKTS){
				++(NumInitializationMessagesSent);
			}
		}

	}
	inline bool IsSendingMyScheduleNeeded(){
		return !IsMyScheduleKnown;
	}
	void RecordMyScheduleSent(){
		IsMyScheduleKnown = true;
	}
	//Send (formerly forward) link details between current and neighbor node
	Link_t SendLink;
	//Receive (formerly reverse) link details between current and neighbor node
	Link_t ReceiveLink;
	NeighborStatus neighborStatus;
	bool IsAvailableForUpperLayers;
	UINT16 CountOfPacketsReceived;
	UINT64 LastHeardTime;
	UINT8 ReceiveDutyCycle; //percentage
	UINT16 FrameLength;

	UINT16  nextSeed; //the seed we have from in the control message last received
	UINT16 mask;
	UINT64  nextwakeupSlot;
	UINT32  seedUpdateIntervalinSlots;

	UINT64  LastTimeSyncRecvTime;			// Lasst time a time sync message is received
	UINT64  LastTimeSyncRequestTime;	// Last time instant a time sync request is sent
	UINT64  LastTimeSyncSendTime;	// Last time instant a time sync is sent(piggbacked)

	UINT8 random_back_off_slot;
	UINT8 random_back_off_window_size;
	//UINT8   numErrors;
	//UINT8   size;
	//BOOL    isInTransition;
	//UINT32  localAvg;
	//INT32   offsetAvg;
	//float   skew;
	//TODO: BK: DELETE THESE NOT USED BUT KEPT FOR THE TIME BEIGN
	UINT16  radioStartDelay;
	UINT16  counterOffset;

	//	Buffer_15_4<Data_Send_Buffer_15_4_t_SIZE> send_buffer;
	//	Buffer_15_4<TimeSync_Send_Buffer_15_4_t_SIZE> tsr_send_buffer;

	//	Buffer_15_4_T<Data_Send_Buffer_15_4_t_SIZE, Message_15_4_t*> send_buffer;
	//	Buffer_15_4_T<TimeSync_Send_Buffer_15_4_t_SIZE, Message_15_4_t*> tsr_send_buffer;

	void MarkAsDead(){
		neighborStatus = Dead;
		IsAvailableForUpperLayers = false;
		IsMyScheduleKnown = false;
		NumInitializationMessagesSent = 0;

	}
	void Clear(){
		//		send_buffer.Initialize();
		//		tsr_send_buffer.Initialize();

		MarkAsDead();

		MACAddress = INVALID_MACADDRESS;
		NumInitializationMessagesSent = 0;
		IsMyScheduleKnown = false;
		//		SendLink.AvgRSSI = 0;
		//		SendLink.LinkQuality = 0;
		//		SendLink.AveDelay = 0;
		//		ReceiveLink.AvgRSSI = 0;
		//		ReceiveLink.LinkQuality = 0;
		//		ReceiveLink.AveDelay = 0;
		SendLink.Initialize();
		ReceiveLink.Initialize();

		neighborStatus = Dead;
		IsAvailableForUpperLayers = false;
		CountOfPacketsReceived = 0;
		LastHeardTime = 0;
		ReceiveDutyCycle = 0;
		FrameLength = 0;

		nextSeed = 0;
		mask = 0;
		nextwakeupSlot = 0;
		seedUpdateIntervalinSlots = 0;

		LastTimeSyncRecvTime = 0;			// Lasst time a time sync message is received
		LastTimeSyncRequestTime = 0;	// Last time instant a time sync request is sent
		LastTimeSyncSendTime = 0;	// Last time instant a time sync is sent(piggbacked)

		UINT8 random_back_off_slot = 0;
		UINT8 random_back_off_window_size = 1;
		//UINT8   numErrors;
		//UINT8   size;
		//BOOL    isInTransition;
		//UINT32  localAvg;
		//INT32   offsetAvg;
		//float   skew;
		//		//TODO: BK: DELETE THESE NOT USED BUT KEPT FOR THE TIME BEIGN
		//		UINT16  radioStartDelay;
		//		UINT16  counterOffset;



	}
}Neighbor_t;

class NeighborTableCommonParameters_One_t{
public:
	UINT16 MACAddress;
	NeighborStatus status;
	UINT64 lastHeardTime;
	Link_t linkQualityMetrics;
	bool availableForUpperLayers;
	NeighborTableCommonParameters_One_t(){
		MACAddress = 0;
		status = NbrStatusError;
		lastHeardTime = 0;
		//linkQualityMetrics.LinkQuality = 0;
		linkQualityMetrics.AvgRSSI = 0;
		linkQualityMetrics.AveDelay = 0;
		availableForUpperLayers=0;
	}
};

class NeighborTableCommonParameters_Two_t{
public:
	UINT16  nextSeed; //the seed we have from in the control message last received
	UINT16 	mask;
	UINT64  nextwakeupSlot;
	UINT32  seedUpdateIntervalinSlots;
	NeighborTableCommonParameters_Two_t(){
		nextSeed = 0;
		mask = 0;
		nextwakeupSlot = 0;
		seedUpdateIntervalinSlots = 0;
	}
};

class NeighborTable {
public:
	UINT8 NumberValidNeighbor;
	UINT8 PreviousNumberValidNeighbor;
	Neighbor_t Neighbor[MAX_NEIGHBORS];
	ArrayBuffer<Total_Buffer_SIZE> send_buffer;


	// neighbor table util functions
	DeviceStatus GetFreeIdx(UINT8* index);
	DeviceStatus ClearNeighbor(UINT16 MACAddress);
	DeviceStatus ClearNeighborwIndex(UINT8 tableIndex);
	DeviceStatus FindIndexEvenDead(const UINT16 MACAddress, UINT8* index);
	DeviceStatus FindIndex(const UINT16 MACAddress, UINT8* index);
	void ClearTable();
	UINT8 BringOutYourDead(UINT32 delay);
	Neighbor_t* GetNeighborPtr(UINT16 address);
	UINT8 NumberOfNeighbors();
	UINT8 NumberOfNeighborsTotal();
	UINT8 PreviousNumberOfNeighbors();
	void SetPreviousNumberOfNeighbors(UINT8 previousNeighborCnt);
	DeviceStatus InsertNeighbor(const NeighborTableCommonParameters_One_t *neighborTableCommonParameters_One_t, const NeighborTableCommonParameters_Two_t *neighborTableCommonParameters_Two_t);
	DeviceStatus FindOrInsertNeighbor(const UINT16 address, UINT8* index);
	DeviceStatus UpdateLink(UINT16 address, Link_t *forwardLink, Link_t *reverseLink, UINT8* index);
	DeviceStatus UpdateFrameLength(UINT16 address, NeighborStatus status, UINT16 frameLength, UINT8* index);
	DeviceStatus UpdateDutyCycle(UINT16 address, UINT8 dutyCycle, UINT8* index);
	DeviceStatus UpdateNeighbor(const NeighborTableCommonParameters_One_t *neighborTableCommonParameters_One_t, const NeighborTableCommonParameters_Two_t *neighborTableCommonParameters_Two_t);
	//DeviceStatus UpdateNeighbor(UINT16 address, NeighborStatus status, UINT64 currTime, UINT16  lastSeed, UINT16  dataInterval, UINT16  radioStartDelay, UINT16  counterOffset, UINT8* index);
	DeviceStatus UpdateNeighbor(const NeighborTableCommonParameters_One_t *neighborTableCommonParameters_One_t);
	UINT8  UpdateNeighborTable(UINT64 livelinessDelayInTicks, UINT64 currentTime);
	UINT8  UpdateNeighborTable(UINT32 NeighborLivenessDelay);

	DeviceStatus RecordTimeSyncRequestSent(UINT16 address, UINT64 _LastTimeSyncTime);
	DeviceStatus RecordTimeSyncSent(UINT16 address, UINT64 _LastTimeSyncTime);
	DeviceStatus RecordTimeSyncRecv(UINT16 address, UINT64 _lastTimeSyncRecv);
	UINT64 GetLastTimeSyncRecv(UINT16 address);
	Neighbor_t* GetMostObsoleteTimeSyncNeighborPtr(NeighborStatus ns = Alive);
	//Neighbor_t* GetNeighborWOldestSyncPtr(const UINT64& curticks, const UINT64& request_limit);
	Neighbor_t* GetCritalSyncNeighborWOldestSyncPtr(const UINT64& curticks, const UINT64& request_limit,const UINT64& forcererequest_limit, const UINT64& fast_disco_request_interval);
	void DegradeLinks();
	UINT16 GetMaxNeighbors();

	DeviceStatus RecordLastHeardTime(UINT16 MACAddress, UINT64 currTime);
	DeviceStatus RecordSenderDelayIncoming(UINT16 MACAddress, const UINT8& delay);


	UINT8 NumberofNeighborsWithNoPacketinQueue(){
		UINT8 numneigh = 0;
		for (UINT8 tableIndex=0; tableIndex<MAX_NEIGHBORS; tableIndex++){
			if(ISMAC_VALID(Neighbor[tableIndex].MACAddress)){
				if(!IsThereAPacketWithDest(Neighbor[tableIndex].MACAddress)){
					++numneigh;
				}
			}
			else{
				++numneigh;
			}
		}
		return numneigh;
	}

	bool DeletePacket(Message_15_4_t* msg_carrier){
		//		for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		//			if(Neighbor[i].MACAddress == msg_carrier->GetHeader()->dest){
		//				if(msg_carrier == FindTSRPacketForNeighbor(Neighbor[i].MACAddress)){
		//					Neighbor[i].tsr_send_buffer.DropOldest(1);
		//				}
		//				else if(msg_carrier == FindDataPacketForNeighbor(Neighbor[i].MACAddress)){
		//					Neighbor[i].send_buffer.DropOldest(1);
		//				}
		//			}
		//		}
		return send_buffer.DeletePacket(msg_carrier);
	}
	Message_15_4_t* FindStalePacketWithRetryAttemptsGreaterThan(UINT8 retryLimit,  BufferOwner n_buf_ow){
		return send_buffer.FindStalePacketWithRetryAttemptsGreaterThan(retryLimit,  n_buf_ow);
	}

	bool ChangeOwnerShipOfElementwIndex(PacketID_T index, BufferOwner n_buf_ow){
		return send_buffer.ChangeOwnerShipOfElementwIndex(index, n_buf_ow);
	}

	/*void FindAndRemoveStalePackets(UINT8 retryLimit){
		send_buffer.RemovePacketsWithRetryAttemptsGreaterThan(retryLimit);
		//		for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		//			if(Neighbor[i].neighborStatus != Dead){
		////				if( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.IsNeighborTimeAvailable(Neighbor[i].MACAddress)){
		//					while(Neighbor[i].tsr_send_buffer.GetNumberMessagesInBuffer() > 0
		//							&& Neighbor[i].tsr_send_buffer.GetOldestwithoutRemoval()->GetMetaData()->GetRetryAttempts() > retryLimit // This can be handled more gracefully
		//					){
		//						Message_15_4_t* msg = Neighbor[i].tsr_send_buffer.GetOldestwithoutRemoval();
		//
		//						ClearMsgContents(msg);
		//						Neighbor[i].tsr_send_buffer.DropOldest(1);
		//						Neighbor[i].SendLink.RecordPacketSuccess(false);
		//					}
		//					while(Neighbor[i].send_buffer.GetNumberMessagesInBuffer() > 0
		//							&& Neighbor[i].send_buffer.GetOldestwithoutRemoval()->GetMetaData()->GetRetryAttempts() > retryLimit
		//					){
		//						Message_15_4_t* msg = Neighbor[i].send_buffer.GetOldestwithoutRemoval();
		//
		//						ClearMsgContents(Neighbor[i].send_buffer.GetOldestwithoutRemoval());
		//						Neighbor[i].send_buffer.DropOldest(1);
		//						Neighbor[i].SendLink.RecordPacketSuccess(false);
		//					}
		////				}
		//			}
		//		}
	}*/
	bool IsThereAPacketWithDest(const UINT16 address){
		if(IsThereADataPacketWithDest(address) || IsThereATSRPacketWithDest(address)){
			return true;
		}
		return false;
	}
	bool IsThereADataPacketWithDest(const UINT16 address){
		//		for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		//			if(Neighbor[i].MACAddress == address){
		//				if(Neighbor[i].send_buffer.GetNumberMessagesInBuffer() > 0){
		//					return true;
		//				}
		//				return false;
		//			}
		//		}
		//		return false;
		return send_buffer.IsThereADataPacketWithDest(address);
	}
	bool IsThereATSRPacketWithDest(const UINT16 address){
		//		for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		//			if(Neighbor[i].MACAddress == address){
		//				if(Neighbor[i].tsr_send_buffer.GetNumberMessagesInBuffer() > 0){
		//					return true;
		//				}
		//				return false;
		//			}
		//		}
		//		return false;
		return send_buffer.IsThereATSRPacketWithDest(address);
	}
	Message_15_4_t* FindTSRPacketForNeighbor(const UINT16 neigh){
		//		Neighbor_t* neigh_ptr = GetNeighborPtr(neigh);
		//		if(neigh_ptr == NULL){
		//			return NULL;
		//		}
		//		else{
		//			return(neigh_ptr->tsr_send_buffer.GetOldestwithoutRemoval());
		//		}
		return send_buffer.FindTSRPacketForNeighbor(neigh);
	}
	Message_15_4_t* FindDataPacketForNeighbor(const UINT16 neigh){
		//		Neighbor_t* neigh_ptr = GetNeighborPtr(neigh);
		//		if(neigh_ptr == NULL){
		//			return NULL;
		//		}
		//		else{
		//			return(neigh_ptr->send_buffer.GetOldestwithoutRemoval());
		//		}
		return send_buffer.FindDataPacketForNeighbor(neigh);
	}
	bool InsertMessage(Message_15_4_t* msg_carrier){
		if(ISPACKET_ID_VALID(InsertMessageGetIndex(msg_carrier)) ) return true;
		else return false;
	}
	PacketID_T InsertMessageGetIndex(Message_15_4_t* msg_carrier){ //TODO: BK: This needs to be revisited. The criteria should not be whether there is a packet for a destination but whether there is a packet for a destination that piggybacks time info

		if(msg_carrier){ // If the msg is valid
			if(IsThereAPacketWithDest(msg_carrier->GetHeader()->dest)){//Preserve space for nodes that do not have any packets in the buffer
				UINT8 numneigh = NumberofNeighborsWithNoPacketinQueue();
				if(numneigh < Total_Buffer_SIZE - send_buffer.GetNumberofElements()){ //If the remaining space is larger than the amount of space that needs to be preserved
					while(IsThereATSRPacketWithDest(msg_carrier->GetHeaderConst()->GetDestConst())) {
						DeletePacket(FindTSRPacketForNeighbor(msg_carrier->GetHeaderConst()->GetDestConst()));
					}
	#if NEIGHBORS_DEBUG_QUEUE_INSERT
					hal_printf("\n There is enough space in queue attempting to insert");
	#endif
					return send_buffer.InsertMessageGetIndex(msg_carrier);
				}
				else{ // Refuse packet If we need to preserve empty space
	#if NEIGHBORS_DEBUG_QUEUE_INSERT
					hal_printf("\nRefuse packet due to need for preserving space numneigh = %u ,  nuel = %u \n", numneigh, send_buffer.GetNumberofElements());
	#endif
					return INVALID_PACKET_ID;
				}

			}
			else{ //This dest has no packets directly insert
#if NEIGHBORS_DEBUG_QUEUE_INSERT
				hal_printf("\n There might be critical space in queue but inserting since this since no other packets exists for this dest");
#endif
				return send_buffer.InsertMessageGetIndex(msg_carrier);
//				if(msg_carrier->GetHeader()->flags & (TIMESTAMPED_FLAG)){
//#if NEIGHBORS_DEBUG_QUEUE_INSERT
//				hal_printf("\n There might be critical space in queue but inserting since this since no other packets exists and this is a timesync packet");
//#endif
//				return send_buffer.InsertMessageGetIndex(msg_carrier);
//				}
//				else{
//					UINT8 numneigh = NumberofNeighborsWithNoPacketinQueue(); //Preserve space for nodes that do not have any packets in the buffer
//					if(numneigh < Total_Buffer_SIZE - send_buffer.GetNumberofElements()){ //If the remaining space is larger than the amount of space that needs to be preserved
//						while(IsThereATSRPacketWithDest(msg_carrier->GetHeaderConst()->GetDestConst())) {
//							DeletePacket(FindTSRPacketForNeighbor(msg_carrier->GetHeaderConst()->GetDestConst()));
//						}
//		#if NEIGHBORS_DEBUG_QUEUE_INSERT
//						hal_printf("\n There is enough space in queue attempting to insert");
//		#endif
//						return send_buffer.InsertMessageGetIndex(msg_carrier);
//					}
//					else{ // Refuse packet If we need to preserve empty space
//		#if NEIGHBORS_DEBUG_QUEUE_INSERT
//						hal_printf("\nRefuse packet due to need for preserving space numneigh = %u ,  nuel = %u \n", numneigh, send_buffer.GetNumberofElements());
//		#endif
//						return INVALID_PACKET_ID;
//					}
//				}
			}
		}
		else{  // Refuse packet If no packet exists
#if NEIGHBORS_DEBUG_QUEUE_INSERT
			hal_printf("\nRefuse packet due to pointer null\n");
#endif

			return INVALID_PACKET_ID;
		}
	}
	Message_15_4_t* GetPacketPtrWithIndex(PacketID_T index){
		return send_buffer.GetPacketPtrWithIndex(index);
	}
	PacketID_T GetIndexWithPtr(Message_15_4_t* msg_carrier){
		if(msg_carrier){
			return send_buffer.GetIndex(*msg_carrier);
		}
		return INVALID_PACKET_ID;
	}

	void FlushDataPacketQueueForNeighbor(const UINT16 address){
		Message_15_4_t* msg_carrier = FindDataPacketForNeighbor(address);
		while(msg_carrier){
			DeletePacket(msg_carrier);
		}
	}
	void FlushTSRPacketQueueForNeighbor(const UINT16 address){
		Message_15_4_t* msg_carrier = FindTSRPacketForNeighbor(address);
		while(msg_carrier){
			DeletePacket(msg_carrier);
		}
	}

	void FlushAllPacketQueueForNeighbor(const UINT16 address){
		FlushTSRPacketQueueForNeighbor(address);
		FlushDataPacketQueueForNeighbor(address);
	}
};


struct PACK MACNeighborInfo	//6bytes
{
	UINT16 MACAddress;
	NeighborStatus neighborStatus;
	bool IsAvailableForUpperLayers;
	bool IsMyScheduleKnown;
	UINT8 NumInitializationMessagesSent;
	UINT8 NumTimeSyncMessagesRecv;
};

struct PACK MACNeighborLinkInfo	//8bytes
{
	Link_t SendLink;
	Link_t ReceiveLink;
};


struct PACK EntendedMACInfoMsgSummary //3 bytes
{
	UINT8 NumTotalEntries;
	UINT8 NNeigh_AFUL;
	UINT8 NumEntriesInMsg;
};

extern "C"
{
	void ClearMsgContents(Message_15_4_t* msg);
}
#endif /* NEIGHBORS_H_ */