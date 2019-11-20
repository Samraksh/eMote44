/*
 * OMACTest.cpp
 *
 *  Created on: Oct 30, 2015
 *      Author: Ananth Muralidharan
 *      A simple Common test for all MACs
 */

//#include <Samraksh/HALTimer.h>
#include "OMACTest.h"

#define OMACTEST_TIMER	32

#define PING_PONG_TIMER_PIN (GPIO_PIN)120
#define PING_PONG_SEND_PIN (GPIO_PIN)120
#define PING_PONG_SEND_ACK_PIN (GPIO_PIN)120
#define PING_PONG_RX_PIN (GPIO_PIN)120

OMACTest g_OMACTest;

extern NeighborTable g_NeighborTable;
extern OMACType g_OMAC;
extern OMACScheduler g_omac_scheduler;

const UINT16 ONESEC_IN_MSEC = 1000;
const UINT16 ONEMSEC_IN_USEC = 1000;
const UINT32 endOfTestCounter = 10000;
const UINT32 dataSendStartingDelay = 20;	//in secs
const UINT32 delayBetweenPackets = 20;	//in secs

UINT16 neighborList[MAX_NEIGHBORS];		//table to store neighbor's MAC address


void Timer_OMACTEST_TIMER_Handler(void * arg){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(PING_PONG_TIMER_PIN, TRUE);
#endif
	if(g_OMAC.GetRadioAddress() != 6846)
		g_OMACTest.Send();

	if(g_OMACTest.sendPingCount == endOfTestCounter){
		VirtTimer_Stop(OMACTEST_TIMER);
		g_OMACTest.GetStatistics();
	}

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(PING_PONG_TIMER_PIN, FALSE);
#endif
}

// Typedef defining the signature of the receive function
//void ReceiveHandler (void *msg, UINT16 Size, UINT16 Src, BOOL Unicast, UINT8 RSSI, UINT8 LinkQuality){
void OMACTest_ReceiveHandler (void *msg, UINT16 size){
	return g_OMACTest.Receive(msg, size);
}

void OMACTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus){
	//g_OMACTest.SendAck(msg, size, status, radioAckStatus);
}

BOOL OMACTest::Initialize(){
	MyAppID = 3; //pick a number less than MAX_APPS currently 4.
	//Config.Network = 138;
	Config.NeighborLivenessDelay = 900000;
	myEventHandler.SetReceiveHandler(OMACTest_ReceiveHandler);
	myEventHandler.SetSendAckHandler(OMACTest_SendAckHandler);
	MacId = OMAC;

	VirtTimer_Initialize();
#ifdef DEBUG_OMACTest
	CPU_GPIO_EnableOutputPin(PING_PONG_SEND_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(PING_PONG_TIMER_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(PING_PONG_RX_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(PING_PONG_SEND_ACK_PIN, FALSE);
#endif
	MAC_Initialize(&myEventHandler, MacId, MyAppID, RF231RADIO, (void*) &Config);

	VirtTimer_SetTimer(OMACTEST_TIMER, dataSendStartingDelay*ONEMSEC_IN_USEC*ONESEC_IN_MSEC, delayBetweenPackets*ONEMSEC_IN_USEC*ONESEC_IN_MSEC, FALSE, FALSE, Timer_OMACTEST_TIMER_Handler); //period (3rd argument) is in micro seconds

	//<start> Initialize payload
	pingPayload.MSGID = sendPingCount;
	pingPayload.msgContent = (char*)"PING";
	//<end> Initialize payload

	return TRUE;
}

BOOL OMACTest::StartTest(){
	VirtTimer_Start(OMACTEST_TIMER);
	return TRUE;
}

BOOL OMACTest::Send(){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(PING_PONG_SEND_PIN, TRUE);
	CPU_GPIO_SetPinState(PING_PONG_SEND_PIN, FALSE);
	CPU_GPIO_SetPinState(PING_PONG_SEND_PIN, TRUE);
#endif

	//MAC_GetNeighborList(neighborList);

	//for(int i = 0; i < MAX_NEIGHBORS; i++){
		//if(neighborList[i] != 0){
			sendPingCount++;
			pingPayload.MSGID = sendPingCount;
			pingPayload.msgContent = (char*)"PING";
			hal_printf("---------%u--------\n\n", sendPingCount);
			hal_printf(">>>>>OMACTest::Send. I am node: %d\n", g_OMAC.GetRadioAddress());
			hal_printf(">>>>>OMACTest::Send PING to neighbor: %d msgId: %d\n", 6846, pingPayload.MSGID);
			hal_printf(">>>>>OMACTest::Send PING msgContent %s\n", pingPayload.msgContent);
			//bool ispcktScheduled = MAC_Send(neighborList[i], MFM_DATA, (void*) &pingPayload, sizeof(Payload_t_ping));
			bool ispcktScheduled = MAC_Send(6846, MFM_DATA, (void*) &pingPayload, sizeof(Payload_t_ping));
		//}
	//}
}


void OMACTest::SendAck(void *tmpMsg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(PING_PONG_SEND_ACK_PIN, TRUE);
	CPU_GPIO_SetPinState(PING_PONG_SEND_ACK_PIN, FALSE);
#endif
	Message_15_4_t* rcvdMsg = (Message_15_4_t*)tmpMsg;
	Payload_t_ping* payload = (Payload_t_ping*)rcvdMsg->GetPayload();
	if(status == NetworkOperations_Success){
		/*hal_printf(">>>>>OMACTest::SendAck sent source %d to dest %d\n", rcvdMsg->GetHeader()->src, rcvdMsg->GetHeader()->dest);
		hal_printf(">>>>>OMACTest::SendAck sent msgID: %d\n", payload->MSGID);
		hal_printf(">>>>>OMACTest::SendAck sent msgContent: %s\n", payload->msgContent);*/
	}else {
		hal_printf(">>>>>OMACTest::SendAck Error while sending msgID: %d from source %d to dest %d\n", payload->MSGID, rcvdMsg->GetHeader()->src, rcvdMsg->GetHeader()->dest);
	}
}


void OMACTest::Receive(void* tmpMsg, UINT16 size){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(PING_PONG_RX_PIN, TRUE);
	CPU_GPIO_SetPinState(PING_PONG_RX_PIN, FALSE);
#endif

	hal_printf("OMACTest::Receive start. I am node: %d\n", g_OMAC.GetRadioAddress());
	recvCount++;
	hal_printf("OMACTest::Receive recvCount is %d\n", recvCount);
	Message_15_4_t* rcvdMsg = (Message_15_4_t*)tmpMsg;
	hal_printf("OMACTest::Receive OMACTest src is %u\n", rcvdMsg->GetHeader()->src);
	hal_printf("OMACTest::ReceiveOMACTest dest is %u\n", rcvdMsg->GetHeader()->dest);

	Payload_t_ping* payload = (Payload_t_ping*)rcvdMsg->GetPayload();
	hal_printf("OMACTest::Receive PING msgID: %d\n", payload->MSGID);
	hal_printf("OMACTest::Receive PING msgContent: %s\n", payload->msgContent);

	while(missedPingID < payload->MSGID){
		hal_printf("Missed packetID: %d\n", missedPingID);
		missedPingID++;
	}

	missedPingID = payload->MSGID+1;

	hal_printf("\n");

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(PING_PONG_RX_PIN, TRUE);
	CPU_GPIO_SetPinState(PING_PONG_RX_PIN, FALSE);
#endif
}

void OMACTest::GetStatistics()
{
	hal_printf("===========================\n");
	hal_printf("OMACTest Level_1A. I am node: %d\n", g_OMAC.GetRadioAddress());
	hal_printf("Total packets transmitted: %u\n", sendPingCount);
	hal_printf("Total packets received: %u\n", recvCount);
	//hal_printf("percentage received: %f \n", (recvCount/sendPingCount)*100);
	hal_printf("===========================\n");
}

void OMACTest_Initialize(){
	g_OMACTest.Initialize();
	g_OMACTest.StartTest();
}


