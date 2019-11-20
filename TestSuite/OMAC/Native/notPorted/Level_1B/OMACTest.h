/*
 * OMACTest.h
 */

#ifndef OMACTEST_H_
#define OMACTEST_H_

#include <tinyhal.h>
#include <Samraksh/Mac_decl.h>
#include <Samraksh/MAC/OMAC/OMAC.h>
#include <Samraksh/VirtualTimer.h>
#include <Samraksh/Message.h>

const char payloadSize = 5;

typedef struct  {
	UINT32 MSGID;
	char* msgContent;
}Payload_t_pong;

typedef struct  {
	UINT32 MSGID;
	UINT8 data[payloadSize];
	char* msgContent;
	Payload_t_pong pongPayload;
}Payload_t_ping;


class OMACTest{
public:
	UINT8 MyAppID;
	Payload_t_ping pingPayload;
	MacEventHandler myEventHandler;
	MacConfig Config;
	UINT8 MacId;
	static UINT32 sendPingCount;
	static UINT32 sendPongCount;
	static UINT32 recvCount;

	BOOL Initialize();
	BOOL StartTest();
	void Receive(void* msg, UINT16 size);
	BOOL Send();
	void SendAck(void *msg, UINT16 size, NetOpStatus status);
	void GetStatistics();
};

//extern OMACTest g_OMACTest;

void OMACTest_Initialize();

UINT32 OMACTest::sendPingCount = 1;
UINT32 OMACTest::sendPongCount = 1;
UINT32 OMACTest::recvCount = 0;

#endif /* OMACTEST_H_ */



