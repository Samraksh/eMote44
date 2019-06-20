/*
 * FTSPTimeSync.h
 *
 *  Created on: Sep 10, 2012
 *      Author: Mukundan
 */

#ifndef FTSPTIMESYNC_H_
#define FTSPTIMESYNC_H_

#include "PacketTimeSync_15_4.h"
#include <Samraksh/Message.h>
#include <Samraksh/Hal_util.h>

//typedef class FTSPTimeSync : public TimeSyncMessage_15_4  {
typedef class FTSPTimeSync {
	float m_skew;
	UINT32 m_offsetAverage;
	UINT32 m_localAverage;

	//Message_15_4_t m_timeSyncBeaconBuffer;
	//Message_15_4_t *m_processedMsg;
	TimeSyncMsg *m_timeSyncBeacon;
	PacketTimeSync_15_4 m_packetTimeSync;

public:
	void Initialize();
	float GetSkew();
	UINT32 GetOffset();
	UINT32 GetSyncPoint();
	DeviceStatus Receive(Message_15_4_t* msg, void* payload, UINT8 len);
	void SendAckHandler(Message_15_4_t* msg, UINT8 len, NetOpStatus success);
	DeviceStatus Send(RadioAddress_t address, Message_15_4_t * msg, UINT16 size, UINT64 event_time);
	bool IsInTransitionPeriod(RadioAddress_t nodeID);
	void SetCounterOffset(UINT16 counterOffset);
	bool IsSynced(RadioAddress_t nodeID);
	void InsertBeacon(Message_15_4_t *ptr);
	bool IsProccessingBeacon();
	void SetSeedInfo(UINT16 seed, UINT32 nextFrame);

}FTSPTimeSync_t;



#endif /* FTSPTIMESYNC_H_ */
