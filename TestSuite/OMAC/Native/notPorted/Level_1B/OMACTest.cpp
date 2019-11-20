/*
 * TimesyncTest.cpp
 */

/*
 * OMACTest.cpp
 *
 *  Created on: Oct 8, 2012
 *      Author: Mukundan Sridharan
 *      A simple Common test for all MACs
 */

//#include <Samraksh/HALTimer.h>
#include "OMACTest.h"

//#define DEBUG_OMACTest 1
#define OMACTEST_TIMER	32

OMACTest g_OMACTest;
extern NeighborTable g_NeighborTable;
extern OMACType g_OMAC;
extern OMACScheduler g_omac_scheduler;
extern UINT16 MF_NODE_ID;
extern Buffer_15_4_t g_send_buffer;
extern DataTransmissionHandler g_DataTransmissionHandler;
extern DataReceptionHandler g_DataReceptionHandler;

const UINT16 ONESEC_IN_MSEC = 1000;
const UINT16 ONEMSEC_IN_USEC = 1000;
const UINT32 endOfTestCounter = 1000;
const UINT32 dataSendStartingDelay = 0;	//in secs
const UINT32 delayBetweenPackets = 5;	//in secs
UINT32 txCounter = 0;

UINT16 neighborList[MAX_NEIGHBORS];		//table to store neighbor's MAC address


static bool MyStrCmp(char* str1, char* str2)
{
	bool retVal = false;
	int i = 0;
	while(str1[i] != '\0' && str2[i] != '\0')
	{
		if(str1[i] != str2[i]){
			retVal = false;
			break;
		}
		else {
			retVal = true;
		}
		i++;
	}
	return retVal;
}


void Timer_OMACTEST_TIMER_Handler(void * arg){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 29, TRUE);
#endif
	g_OMACTest.Send();

	if(g_OMACTest.sendPingCount == endOfTestCounter){
		VirtTimer_Stop(OMACTEST_TIMER);
		g_OMACTest.GetStatistics();
	}

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 29, FALSE);
#endif
}

// Typedef defining the signature of the receive function
//void ReceiveHandler (void *msg, UINT16 Size, UINT16 Src, BOOL Unicast, UINT8 RSSI, UINT8 LinkQuality){
void OMACTest_ReceiveHandler (void *msg, UINT16 size){
	return g_OMACTest.Receive(msg, size);
}

void OMACTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status){
	g_OMACTest.SendAck(msg,size,status);
}

BOOL OMACTest::Initialize(){
	MyAppID = 3; //pick a number less than MAX_APPS currently 4.
	Config.Network = 138;
	Config.NeighborLivenessDelay = 900000;
	myEventHandler.SetReceiveHandler(OMACTest_ReceiveHandler);
	myEventHandler.SetSendAckHandler(OMACTest_SendAckHandler);
	MacId = OMAC;

	VirtTimer_Initialize();
#ifdef DEBUG_OMACTest
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 24, FALSE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 25, FALSE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 29, FALSE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 30, FALSE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 31, FALSE);
#endif
	Mac_Initialize(&myEventHandler, MacId, MyAppID, Config.RadioID, (void*) &Config);

	VirtTimer_SetTimer(OMACTEST_TIMER, dataSendStartingDelay*ONEMSEC_IN_USEC*ONESEC_IN_MSEC, delayBetweenPackets*ONEMSEC_IN_USEC*ONESEC_IN_MSEC, FALSE, FALSE, Timer_OMACTEST_TIMER_Handler); //period (3rd argument) is in micro seconds

	//<start> Initialize payload
	for(int i = 1; i <= payloadSize; i++){
		pingPayload.data[i-1] = i;
	}
	pingPayload.MSGID = sendPingCount;
	pingPayload.msgContent = (char*)"PING";

	pingPayload.pongPayload.MSGID = 0;
	pingPayload.pongPayload.msgContent = (char*)"NULL";
	//<end> Initialize payload

	return TRUE;
}

BOOL OMACTest::StartTest(){
	VirtTimer_Start(OMACTEST_TIMER);
	return TRUE;
}

BOOL OMACTest::Send(){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 24, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 24, FALSE);
	CPU_GPIO_SetPinState((GPIO_PIN) 24, TRUE);
#endif

	if(txCounter > 0){
		txCounter++;
	}
	Mac_GetNeighborList(neighborList);

	for(int i = 0; i < MAX_NEIGHBORS; i++){
		if(neighborList[i] != 0){
			if(txCounter == 0){
				txCounter++;
			}
			//sendPingCount++;
			//pingPayload.MSGID = sendPingCount;
			//pingPayload.msgContent = (char*)"PING";
			hal_printf("---------%u--------\n\n", txCounter);
			hal_printf(">>>>>OMACTest::Send. I am node: %d\n", g_OMAC.GetAddress());
			hal_printf(">>>>>OMACTest::Send PING to neighbor: %d msgId: %d\n", neighborList[i], pingPayload.MSGID);
			hal_printf(">>>>>OMACTest::Send PONG to neighbor: %d msgId: %d\n", neighborList[i], pingPayload.pongPayload.MSGID);
			//hal_printf(">>>>>OMACTest::Send PING msgContent %s\n", pingPayload.msgContent);
			hal_printf(">>>>>OMACTest::Send PONG msgContent %s\n", pingPayload.pongPayload.msgContent);
			bool ispcktScheduled = Mac_Send(neighborList[i], MFM_DATA, (void*) &pingPayload, sizeof(Payload_t_ping));
		}
	}
}


void OMACTest::SendAck(void *tmpMsg, UINT16 size, NetOpStatus status){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 31, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 31, FALSE);
#endif
	Message_15_4_t* rcvdMsg = (Message_15_4_t*)tmpMsg;
	Payload_t_ping* payload = (Payload_t_ping*)rcvdMsg->GetPayload();
	if(status == NO_Success){
		//hal_printf("OMACTest::SendAck sent msgID: %d from source %d to dest %d\n", payload->MSGID, rcvdMsg->GetHeader()->src, rcvdMsg->GetHeader()->dest);
	}else {
		hal_printf("OMACTest::SendAck Error while sending msgID: %d from source %d to dest %d\n", payload->MSGID, rcvdMsg->GetHeader()->src, rcvdMsg->GetHeader()->dest);
	}
}


void OMACTest::Receive(void* tmpMsg, UINT16 size){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN)30, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN)30, FALSE);
#endif

	hal_printf("start OMACTest::Receive. I am node: %d\n", g_OMAC.GetAddress());
	recvCount++;
	hal_printf("OMACTest::Receive recvCount is %d\n", recvCount);
	Message_15_4_t* rcvdMsg = (Message_15_4_t*)tmpMsg;
	hal_printf("OMACTest src is %u\n", rcvdMsg->GetHeader()->src);
	hal_printf("OMACTest dest is %u\n", rcvdMsg->GetHeader()->dest);

	Payload_t_ping* payload = (Payload_t_ping*)rcvdMsg->GetPayload();
	UINT16 msgId = payload->MSGID;
	hal_printf("PING msgID: %d\n", msgId);

	hal_printf("PONG msgID: %d\n", payload->pongPayload.MSGID);
	hal_printf("PONG msgContent: %s\n", payload->pongPayload.msgContent);

	//When a node X sends a "PING", it is expecting to hear back a "PONG" with same msgID from node Y.
	//Node X, on hearing a "PONG", checks if PONG msgID is same as PING msgID.
	//	If yes, increments global static msgID and sends a new "PING".
	//	If no, keeps sending old PING.
	//
	//When a node gets a ping, check if its msgId is +1 of pong msgId.
	//If yes, then it means that a pong reached the other node and that the other node
	//is responding with a new ping.
	if(payload->MSGID == sendPongCount+1){
		ASSERT(sendPongCount+1 == sendPingCount);
		sendPongCount = sendPingCount;
		//pingPayload.pongPayload.MSGID = sendPongCount;
		//pingPayload.pongPayload.msgContent = (char*)"PONG";
	}
	//Continue to send a "PONG", until "PING" msgID from other node is +1 of current PONG.
	//If yes, then send a new PONG
	if(payload->pongPayload.MSGID == 0){
		pingPayload.pongPayload.MSGID = sendPongCount;
		hal_printf("1. Sending a PONG. msgId is %d\n", pingPayload.pongPayload.MSGID);
		pingPayload.pongPayload.msgContent = (char*)"PONG";
	}
	//When a node gets a pong, check if its msgId matches the current ping.
	//If yes, then it means that the ping reached.
	//Increase the ping msgID.
	else if(payload->pongPayload.MSGID == sendPingCount){
		sendPingCount++;
	}
	hal_printf("current sendPingCount: %d\n", sendPingCount);
	hal_printf("current sendPongCount: %d\n", sendPongCount);
	pingPayload.MSGID = sendPingCount;
	pingPayload.msgContent = (char*)"PING";
	if(payload->pongPayload.MSGID != 0){
		pingPayload.pongPayload.MSGID = sendPongCount;
		pingPayload.pongPayload.msgContent = (char*)"PONG";
	}

	hal_printf("\n");

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 30, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 30, FALSE);
#endif
	hal_printf("end OMACTest::Receive\n");
}

void OMACTest::GetStatistics()
{
	hal_printf("===========================\n");
	hal_printf("OMACTest Level_1A. I am node: %d\n", g_OMAC.GetAddress());
	hal_printf("Total packets transmitted: %u\n", sendPingCount);
	hal_printf("Total packets received: %u\n", recvCount);
	//hal_printf("percentage received: %f \n", (recvCount/sendPingCount)*100);
	hal_printf("===========================\n");
}

void OMACTest_Initialize(){
	g_OMACTest.Initialize();
	g_OMACTest.StartTest();
}


