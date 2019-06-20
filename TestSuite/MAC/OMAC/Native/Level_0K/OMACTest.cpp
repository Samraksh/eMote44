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

#define DEBUG_OMACTest 1
#define OMACTEST_Timer (GPIO_PIN)120//29
#define OMACTEST_Rx (GPIO_PIN)120//30
#define OMACTEST_Tx (GPIO_PIN)120//24
#define OMACTEST_TxAck (GPIO_PIN)120//31

#define TEST_0B_TIMER	4

const UINT16 ONESEC_IN_MSEC = 1000;
const UINT16 ONEMSEC_IN_USEC = 1000;

OMACTest g_OMACTest;
extern NeighborTable g_NeighborTable;
extern OMACType g_OMAC;
extern OMACScheduler g_omac_scheduler;
extern UINT16 MF_NODE_ID;
extern Buffer_15_4_t g_send_buffer;

void Timer_32_Handler(void * arg){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Timer, TRUE);
#endif
	g_OMACTest.Send();
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Timer, FALSE);
#endif
}

// Typedef defining the signature of the receive function
//void ReceiveHandler (void *msg, UINT16 Size, UINT16 Src, BOOL Unicast, UINT8 RSSI, UINT8 LinkQuality){
void OMACTest_ReceiveHandler (void* msg, UINT16 size){
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

	m_TimerPeriod = 5*ONESEC_IN_MSEC*ONEMSEC_IN_USEC;

	VirtTimer_Initialize();
#ifdef DEBUG_OMACTest
	CPU_GPIO_EnableOutputPin(OMACTEST_Tx, FALSE);
	CPU_GPIO_EnableOutputPin(OMACTEST_Timer, FALSE);
	CPU_GPIO_EnableOutputPin(OMACTEST_Rx, FALSE);
	CPU_GPIO_EnableOutputPin(OMACTEST_TxAck, FALSE);
#endif
	Mac_Initialize(&myEventHandler, MacId, MyAppID, Config.RadioID, (void*) &Config);

	VirtTimer_SetTimer(TEST_0B_TIMER, 0, m_TimerPeriod, FALSE, FALSE, Timer_32_Handler); //period (3rd argument) is in micro seconds
	return TRUE;
}

BOOL OMACTest::StartTest(){
	msg.MSGID = 1;
	SendCount = 1;
	receiveCount = 0;

	VirtTimer_Start(TEST_0B_TIMER);

	return TRUE;
}

void OMACTest::Receive(void* tmpMsg, UINT16 size){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Rx, TRUE);
	CPU_GPIO_SetPinState(OMACTEST_Rx, FALSE);
#endif
	receiveCount++;
	Message_15_4_t* rcvdMsg = (Message_15_4_t*)tmpMsg;
	hal_printf("start OMACTest::Receive\n");

	hal_printf("OMACTest src is %u\n", rcvdMsg->GetHeader()->src);
	hal_printf("OMACTest dest is %u\n", rcvdMsg->GetHeader()->dest);
	Payload_t* payload = (Payload_t*)rcvdMsg->GetPayload();
	hal_printf("Receive count is %u\n", receiveCount);
	hal_printf("OMACTest msgID is: %u\n", payload->MSGID);
	hal_printf("OMACTest payload is \n");
	for(int i = 1; i <= 5; i++){
		hal_printf(" %d\n", payload->data[i-1]);
	}
	hal_printf("\n");

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Rx, TRUE);
	CPU_GPIO_SetPinState(OMACTEST_Rx, FALSE);
#endif
	hal_printf("end OMACTest::Receive\n");
}

void OMACTest::SendAck(void *msg, UINT16 size, NetOpStatus status){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_TxAck, TRUE);
	CPU_GPIO_SetPinState(OMACTEST_TxAck, FALSE);
#endif
	if(status == NetworkOperations_Success){

	}else {

	}
}


BOOL OMACTest::Send(){
	VirtualTimerReturnMessage rm;
	msg.MSGID = SendCount;
	//msg.data[10] = 10;
	for(int i = 1; i <= 5; i++){
		msg.data[i-1] = i;
	}

	UINT16 Nbr2beFollowed = g_OMAC.Neighbor2beFollowed;
	if (g_NeighborTable.GetNeighborPtr(Nbr2beFollowed) == NULL) {
		return FALSE;
	}
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(OMACTEST_Tx, TRUE);
	CPU_GPIO_SetPinState(OMACTEST_Tx, FALSE);
	CPU_GPIO_SetPinState(OMACTEST_Tx, TRUE);
#endif
	//Mac_Send(MacId, MAC_BROADCAST_ADDRESS, MFM_DATA, (void*) &msg.data, sizeof(Payload_t));
	hal_printf("Sending msgID: %u\n", SendCount);
	bool ispcktScheduled = Mac_Send(Nbr2beFollowed, MFM_DATA, (void*) &msg, sizeof(Payload_t));
	//if (ispcktScheduled) {SendCount++;}
	SendCount++;

	if( m_TimerPeriod == 5*ONESEC_IN_MSEC*ONEMSEC_IN_USEC && SendCount % 20 == 0 ){

		m_TimerPeriod = 50*ONESEC_IN_MSEC*ONEMSEC_IN_USEC;
		rm = VirtTimer_Change(TEST_0B_TIMER, 0, m_TimerPeriod, FALSE );
	}
	else if( m_TimerPeriod == 50*ONESEC_IN_MSEC*ONEMSEC_IN_USEC && SendCount % 4 == 0 ){
		m_TimerPeriod = 5*ONESEC_IN_MSEC*ONEMSEC_IN_USEC;
		rm = VirtTimer_Change(TEST_0B_TIMER, 0, m_TimerPeriod, FALSE );
	}
}

void OMACTest_Initialize(){
	g_OMACTest.Initialize();
	g_OMACTest.StartTest();
}


