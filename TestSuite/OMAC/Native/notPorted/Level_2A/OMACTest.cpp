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

#define FAN_IN
//#define FAN_OUT

#define FAN_IN_TIMER	32
#define FAN_OUT_TIMER	33

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
const UINT32 endOfTestCounter = 150;
const UINT32 dataSendStartingDelay = 0;	//in secs
const UINT32 delayBetweenPackets = 5;	//in secs
UINT16 neighborList[MAX_NEIGHBORS];		//table to store neighbor's MAC address

UINT16 FAN_IN_TIMER_Counter = 0;
UINT16 FAN_OUT_TIMER_Counter = 0;


void FAN_IN_TIMER_Handler(void* arg){
	g_OMACTest.Send_fan_in();
	FAN_IN_TIMER_Counter++;
}

void FAN_OUT_TIMER_Handler(void* arg){
	g_OMACTest.Send_fan_out();
	FAN_OUT_TIMER_Counter++;
}

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

	VirtTimer_Initialize();
#ifdef DEBUG_OMACTest
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 24, FALSE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 25, FALSE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 29, FALSE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 30, FALSE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN) 31, FALSE);
#endif
	Mac_Initialize(&myEventHandler, MacId, MyAppID, Config.RadioID, (void*) &Config);

#ifdef FAN_IN
	//VirtTimer_SetTimer(FAN_IN_TIMER, dataSendStartingDelay*ONEMSEC_IN_USEC*ONESEC_IN_MSEC, delayBetweenPackets*ONEMSEC_IN_USEC*ONESEC_IN_MSEC, FALSE, FALSE, FAN_IN_TIMER_Handler); //period (3rd argument) is in micro seconds
#endif
#ifdef FAN_OUT
	VirtTimer_SetTimer(FAN_OUT_TIMER, dataSendStartingDelay*ONEMSEC_IN_USEC*ONESEC_IN_MSEC, delayBetweenPackets*ONEMSEC_IN_USEC*ONESEC_IN_MSEC, FALSE, FALSE, FAN_OUT_TIMER_Handler); //period (3rd argument) is in micro seconds
#endif
	return TRUE;
}


BOOL OMACTest::StartTest(){
	SendCount = 0;
	recvCount = 0;
#ifdef FAN_IN
	hal_printf(">>>>>>>>>>>>STARTING FAN_IN<<<<<<<<<<<<\n");
	VirtTimer_Start(FAN_IN_TIMER);
#endif
#ifdef FAN_OUT
	hal_printf("<<<<<<<<<<<<STARTING FAN_OUT>>>>>>>>>>>\n");
	VirtTimer_Start(FAN_OUT_TIMER);
#endif
	while(1)
	{
		if(FAN_IN_TIMER_Counter == endOfTestCounter && FAN_OUT_TIMER_Counter == 0){
			VirtTimer_Stop(FAN_IN_TIMER);
			FAN_IN_TIMER_Counter = 0;
#ifdef FAN_OUT
			hal_printf("<<<<<<<<<<<<<<<<STARTING FAN_OUT>>>>>>>>>>>>>>>>\n");
			VirtTimer_Start(FAN_OUT_TIMER);
#endif
		}
		else if(FAN_IN_TIMER_Counter == 0 && FAN_OUT_TIMER_Counter == endOfTestCounter){
			VirtTimer_Stop(FAN_OUT_TIMER);
			FAN_OUT_TIMER_Counter = 0;
#ifdef FAN_IN
			hal_printf(">>>>>>>>>>>>STARTING FAN_IN<<<<<<<<<<<<\n");
			VirtTimer_Start(FAN_IN_TIMER);
#endif
		}
	}

	return TRUE;
}

//Multiple nodes sending to one common node
BOOL OMACTest::Send_fan_in(){
	static bool flag = false;
	bool ispcktScheduled = false;

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 24, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 24, FALSE);
	CPU_GPIO_SetPinState((GPIO_PIN) 24, TRUE);
#endif
	Mac_GetNeighborList(neighborList);

	for(int i = 0; i < MAX_NEIGHBORS; i++){
		if(neighborList[i] != 0){
			if(!flag){
				payload.msgContent = (char*)"PING";
				for(int i = 1; i <= payloadSize; i++){
					payload.data[i-1] = i;
				}
				flag = true;
			}
			SendCount++;
			payload.MSGID = SendCount;

			hal_printf("---------%u--------\n\n", SendCount);
			hal_printf("OMACTest::Send_fan_out. I am node: %d\n", g_OMAC.GetAddress());
			hal_printf("Sending to node %d; msgID is %d\n", neighborList[i], payload.MSGID);
			ispcktScheduled = Mac_Send(neighborList[i], MFM_DATA, (void*) &payload, sizeof(Payload_t));
			if(!ispcktScheduled){
				hal_printf("Send to neighbor node %d failed \n", neighborList[i]);
			}
		}
	}
}

//One node sending to multiple nodes
BOOL OMACTest::Send_fan_out(){
	static bool flag = false;
	bool ispcktScheduled = false;

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 24, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 24, FALSE);
	CPU_GPIO_SetPinState((GPIO_PIN) 24, TRUE);
#endif

	Mac_GetNeighborList(neighborList);

	for(int i = 0; i < MAX_NEIGHBORS; i++){
		if(neighborList[i] != 0){
			if(!flag){
				payload.msgContent = (char*)"PING";
				for(int i = 1; i <= payloadSize; i++){
					payload.data[i-1] = i;
				}
				flag = true;
			}
			SendCount++;
			payload.MSGID = SendCount;

			hal_printf("---------%u--------\n\n", SendCount);
			hal_printf("OMACTest::Send_fan_out. I am node: %d\n", g_OMAC.GetAddress());
			hal_printf("Sending to node %d; msgID is %d\n", neighborList[i], payload.MSGID);
			ispcktScheduled = Mac_Send(neighborList[i], MFM_DATA, (void*) &payload, sizeof(Payload_t));
			if(!ispcktScheduled){
				hal_printf("Send to neighbor node %d failed \n", neighborList[i]);
			}
		}
	}
}


void OMACTest::SendAck(void* tmpMsg, UINT16 size, NetOpStatus status){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 31, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 31, FALSE);
#endif
	Message_15_4_t* rcvdMsg = (Message_15_4_t*)tmpMsg;
	Payload_t* payload = (Payload_t*)rcvdMsg->GetPayload();
	if(status == NO_Success){
		hal_printf("OMACTest::SendAck sent msgID: %d from source %d to dest %d\n", payload->MSGID, rcvdMsg->GetHeader()->src, rcvdMsg->GetHeader()->dest);
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

	Payload_t* payload = (Payload_t*)rcvdMsg->GetPayload();
	UINT16 msgId = payload->MSGID;
	hal_printf("OMACTest msgID: %d\n", msgId);
	hal_printf("OMACTest payload is \n");
	for(int i = 1; i <= payloadSize; i++){
		hal_printf(" %d\n", payload->data[i-1]);
	}
	hal_printf("msgContent: %s\n", payload->msgContent);

	/*if(hal_stricmp(payload->msgContent, (char*)"PING") == 0){
		hal_printf("Inside ping\n");
		msgId += 1;
		payload.MSGID = msgId;
		hal_printf("msgId inside ping is %d\n", payload.MSGID);
		payload.msgContent = (char*)"PONG";
	}
	else if(hal_stricmp(payload->msgContent, (char*)"PONG") == 0){
		hal_printf("Inside pong\n");
		msgId += 2;
		payload.MSGID = msgId;
		hal_printf("msgId inside pong is %d\n", payload.MSGID);
		payload.msgContent = (char*)"PING";
	}
	else {
		hal_printf("something wrong\n");
	}*/

	hal_printf("\n");

#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState((GPIO_PIN) 30, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) 30, FALSE);
#endif
	hal_printf("end OMACTest::Receive\n");
}


void OMACTest_Initialize(){
	g_OMACTest.Initialize();
	g_OMACTest.StartTest();
}


