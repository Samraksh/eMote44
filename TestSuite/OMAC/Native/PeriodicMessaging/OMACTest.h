/*
 * OMACTest.h
 */

#ifndef OMACTest_H_
#define OMACTest_H_



#include <tinyhal.h>
#include <Samraksh/MAC_decl.h>
#include <Samraksh/MAC/OMAC/OMAC.h>
#include <Samraksh/VirtualTimer.h>
#include <Samraksh/Message.h>
#include <Samraksh/Radio_decl.h>

typedef struct {
	UINT16 MACAddress;
	UINT16 SendCount;
	UINT16 ReceiveCount;
	UINT16 ConvTime;
}OMACTest_Neighbor_List_t;


class OMACTest{
public:
	OMACTest_Neighbor_List_t OMACTest_Neighbor_List[10]; 
	UINT64 sequence_number;
	UINT64 sent_packet_count;
	UINT64 rx_packet_count;

	UINT64 NextEventTime;

	GPIO_PIN m_LOCALCLOCKMONITORPIN;
	GPIO_PIN m_NEIGHBORCLOCKMONITORPIN;

	OMACTest();

	GlobalTime m_globalTime;
	UINT8 MyAppID;
	MACEventHandler myEventHandler;
	MACConfig Config;
	UINT8 MacId;
	BOOL LocalClkPINState;
	BOOL NeighborClkPINState;

	UINT64 LocalClockMonitorFrameNum;
	UINT64 NeighborClockMonitorFrameNum;

	UINT64 TargetTimeinTicks;

	bool NeighborFound;

	BOOL Initialize();
	BOOL StartTest();

	BOOL ScheduleNextNeighborCLK();
	BOOL ScheduleNextLocalCLK();
	void SendPacketToNeighbor();
};

OMACTest gOMACTest;

void OMACTest_Initialize();


#endif /* OMACTest_H_ */


