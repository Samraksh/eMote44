/*
 * OMACTest.h
 */

#ifndef OMACTEST_H_
#define OMACTEST_H_

#include <tinyhal.h>
//#include <../DeviceCode/Targets/Native/STM32F10x/DeviceCode/drivers/radio/RF231/RF231.h>
#include <Samraksh/MAC_decl.h>
#include <Samraksh/MAC/OMAC/OMAC.h>
#include <Samraksh/VirtualTimer.h>
#include <Samraksh/Message.h>

#define MAX_NEIGHBORS 12

typedef struct  {
	UINT16 MSGID;
	UINT8 data[5];
}Payload_t;

typedef struct{
	UINT16 nodeID;
	UINT32 lastMSGID;
	UINT32 totalRecvCount;
}NodeStats_t;

typedef struct{
	NodeStats_t nodeStats;
}NewNode_t;

enum RadioID : UINT8
{
	RF231RADIO,
	RF231RADIOLR,
	SI4468_SPI2,
};

class OMACTest{
public:
	UINT8 MyAppID;
	Payload_t msg;
	NewNode_t newNode[MAX_NEIGHBORS];
	static UINT8 nodeCount;
	MACEventHandler myEventHandler;
	MACConfig Config;
	UINT8 MACId;
	UINT16 SendCount, receiveCount;
	static UINT64 startTicks, endTicks;

	BOOL DisplayStats(BOOL result, char* resultParameter1, char* resultParameter2, int accuracy);
	BOOL Initialize();
	BOOL StartTest();
	int LookupNodeID(UINT16 nodeID);
	void Receive(void* msg, UINT16 size);
	void ShowStats();
	BOOL Send();
	void SendAck(void *msg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus);
};

//extern OMACTest g_OMACTest;

void OMACTest_Initialize();

#endif /* OMACTEST_H_ */



