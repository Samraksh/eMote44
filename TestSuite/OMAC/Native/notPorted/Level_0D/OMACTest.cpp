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

const UINT16 ONESEC_IN_MSEC = 1000;
const UINT16 ONEMSEC_IN_USEC = 1000;

OMACTest g_OMACTest;
extern NeighborTable g_NeighborTable;
extern OMACType g_OMAC;
extern OMACScheduler g_omac_scheduler;
extern UINT16 MF_NODE_ID;
extern Buffer_15_4_t g_send_buffer;
extern DataTransmissionHandler g_DataTransmissionHandler;
extern DataReceptionHandler g_DataReceptionHandler;

UINT32 txCounter = 0;


void Timer_32_Handler(void * arg){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 29, TRUE);
#endif
	txCounter++;
	g_OMACTest.Send();
	UINT16 Neighbor2beFollowed = g_omac_scheduler.m_TimeSyncHandler.Neighbor2beFollowed;
	//UINT32 m_nextWakeupSlot = g_DataReceptionHandler.GetWakeupSlot();
	UINT64 currentTicks = HAL_Time_CurrentTicks();

	hal_printf("---------%u--------\n", txCounter);

	hal_printf( "[LT: %llu NT: %llu]\n", currentTicks, g_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(Neighbor2beFollowed, currentTicks) );
	hal_printf("Node %d's nextWakeupSlot: %u\n", g_OMAC.GetAddress(), g_DataReceptionHandler.GetWakeupSlot());
	hal_printf("Node %d's current slotNumber: %u\n", g_OMAC.GetAddress(), g_omac_scheduler.GetSlotNumber());

	hal_printf("-----------------\n");

	INT64 tmp_nbrGlobalTime = g_DataTransmissionHandler.GetNeighborGlobalTime();
	//UINT32 m_nextTXTicks = g_DataTransmissionHandler.GetTxTicks();
	//UINT32 m_nextTXCounter = g_DataTransmissionHandler.GetTxCounter();

	if(tmp_nbrGlobalTime > 0){
		hal_printf("Neighbor node %u's global time: %lld [NT should be equal to this]\n", Neighbor2beFollowed, tmp_nbrGlobalTime);
	}
	hal_printf("Neighbor node %u's wakeup slot: %u [should be equal to current slotNumber on other node]\n", Neighbor2beFollowed, g_DataTransmissionHandler.GetNeighborWakeupSlot());
	//hal_printf("Neighbor node %u's next wakeup: %u [should be equal to current slotNumber on other node]\n", Neighbor2beFollowed, g_DataTransmissionHandler.GetNeighborNextWakeup());
	hal_printf("Neighbor node %d's nextTXTicks: %llu [should be equal to LT on other node]\n", Neighbor2beFollowed, g_DataTransmissionHandler.GetTxTicks());
	//hal_printf("Neighbor node %d's nextTXCounter: %u [should be equal to current slotNumber on other node]\n", Neighbor2beFollowed, g_DataTransmissionHandler.GetTxCounter());

	hal_printf("\n");


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
	//MF_NODE_ID=10;	//Set address
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

	VirtTimer_SetTimer(32, 0, 5*ONEMSEC_IN_USEC*ONESEC_IN_MSEC, FALSE, FALSE, Timer_32_Handler); //period (3rd argument) is in micro seconds
	return TRUE;
}

BOOL OMACTest::StartTest(){
	//msg.MSGID = 0;
	SendCount = 1;
	RcvCount = 0;
	//while(1){
		VirtTimer_Start(32);
	//}

	return TRUE;
}


void OMACTest::Receive(void* tmpMsg, UINT16 size){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN)30, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN)30, FALSE);
#endif

	hal_printf("start OMACTest::Receive. I am node: %d\n", g_OMAC.GetAddress());
	Message_15_4_t* rcvdMsg = (Message_15_4_t*)tmpMsg;
	hal_printf("OMACTest src is %u\n", rcvdMsg->GetHeader()->src);
	hal_printf("OMACTest dest is %u\n", rcvdMsg->GetHeader()->dest);

	Payload_t* payload = (Payload_t*)rcvdMsg->GetPayload();
	UINT16 msgId = payload->MSGID;
	hal_printf("OMACTest msgID: %d\n", msgId);
	hal_printf("OMACTest payload is \n");
	for(int i = 1; i <= payloadSize; i++){
		hal_printf(" %d\n", payload->data[i-1]);
	}
	hal_printf("msgContent: %s\n", payload->msgContent);

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 30, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 30, FALSE);
#endif
	hal_printf("end OMACTest::Receive\n");
}


void OMACTest::SendAck(void *msg, UINT16 size, NetOpStatus status){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 31, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 31, FALSE);
#endif
	if(status == NO_Success){

	}else {

	}
}


BOOL OMACTest::Send(){
	//msg.data[10] = 10;
	for(int i = 1; i <= payloadSize; i++){
		msg.data[i-1] = i;
	}

	msg.MSGID = SendCount;
	msg.msgContent = (char*)"PING";

	UINT16 Neighbor2beFollowed = g_omac_scheduler.m_TimeSyncHandler.Neighbor2beFollowed;
	if (g_NeighborTable.GetNeighborPtr(Neighbor2beFollowed) == NULL) {
		return FALSE;
	}

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 24, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 24, FALSE);
	CPU_GPIO_SetPinState((GPIO_PIN) 24, TRUE);
#endif
	//Mac_Send(MacId, MAC_BROADCAST_ADDRESS, MFM_DATA, (void*) &msg.data, sizeof(Payload_t));
	hal_printf("msgId before sending is %d\n", msg.MSGID);
	bool ispcktScheduled = Mac_Send(Neighbor2beFollowed, MFM_DATA, (void*) &msg, sizeof(Payload_t));
	if (ispcktScheduled == 0) {SendCount++;}
}

void OMACTest_Initialize(){
	g_OMACTest.Initialize();
	g_OMACTest.StartTest();
}


