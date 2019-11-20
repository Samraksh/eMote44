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


//#define TWO_NODES_TX_RX
#if defined(TWO_NODES_TX_RX)
#define def_Neighbor2beFollowed
#define TXNODEID 3505
#define RXNODEID 6846
#endif


//#define FAN_OUT
//#define FAN_IN

#if defined(FAN_OUT)
#define def_Neighbor2beFollowed2
#define RXNODEID1 3505
#define RXNODEID2 31436
#define TXNODEID 6846
#elif defined(FAN_IN)
#define def_Neighbor2beFollowed2
#define TXNODEID1 3505
#define TXNODEID2 31436
#define RXNODEID 6846
#endif


const char payloadSize = 5;

typedef struct  {
	UINT16 MSGID;
	UINT8 data[payloadSize];
	char* msgContent;
}Payload_t;

class OMACTest{
public:
	UINT8 MyAppID;
	Payload_t payload;
	MacEventHandler myEventHandler;
	MacConfig Config;
	UINT8 MacId;
	UINT16 SendCount;
	UINT16 recvCount;

	BOOL Initialize();
	BOOL StartTest();
	void Receive(void* msg, UINT16 size);
	BOOL Send_fan_in();
	BOOL Send_fan_out();
	void SendAck(void *msg, UINT16 size, NetOpStatus status);
};

//extern OMACTest g_OMACTest;

void OMACTest_Initialize();

#endif /* OMACTEST_H_ */



