/*
 * OMACTest.h
 */

#ifndef OMACTEST_H_
#define OMACTEST_H_

#include <tinyhal.h>
#include <Samraksh/MAC_decl.h>
#include <Samraksh/MAC/OMAC/OMAC.h>
#include <Samraksh/VirtualTimer.h>
#include <Samraksh/Message.h>


typedef struct  {
	UINT32 MSGID;
	char* msgContent;
}Payload_t_ping;

enum RadioID : UINT8
{
	RF231RADIO,
	RF231RADIOLR,
};


class OMACTest{
public:
	UINT8 MyAppID;
	Payload_t_ping pingPayload;
	MACEventHandler myEventHandler;
	MACConfig Config;
	UINT8 MacId;
	static UINT32 sendPingCount;
	static UINT32 recvCount;
	static UINT32 missedPingID;

	BOOL Initialize();
	BOOL StartTest();
	void Receive(void* msg, UINT16 size);
	BOOL Send();
	void SendAck(void *msg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus);
	void GetStatistics();
};

//extern OMACTest g_OMACTest;

void OMACTest_Initialize();

UINT32 OMACTest::sendPingCount = 0;
UINT32 OMACTest::recvCount = 0;
UINT32 OMACTest::missedPingID = 1;

#endif /* OMACTEST_H_ */



