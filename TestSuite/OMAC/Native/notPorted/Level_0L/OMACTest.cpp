/*
 * TimesyncTest.cpp
 */

/*
 * OMACTest.cpp
 *
 *  Created on: Dec 24, 2015
 *      Author: Ananth Muralidharan
 *      A simple test to measure the pseudo randomness of the PRNG
 */

#include "OMACTest.h"
#include <TinyCLR_Runtime.h>

#define DEBUG_OMACTest 1

const UINT16 ONESEC_IN_MSEC = 1000;
const UINT16 ONEMSEC_IN_USEC = 1000;

OMACTest g_OMACTest;
CLR_RT_Random rand1;
extern OMACType g_OMAC;
extern OMACScheduler g_omac_scheduler;

#define TEST_0L_TIMER	10
#define TIMER_PERIOD 	1*ONESEC_IN_MSEC

#define Test_0L_Timer_Pin 1 //2

#define USEONESHOTTIMER FALSE

//NEIGHBORCLOCKMONITORPERIOD in ticks
#define NEIGHBORCLOCKMONITORPERIOD 800000
#define INITIALDELAY 100000


void Timer_Test_0L_Handler(void * arg){
	g_OMACTest.GenerateRandVal();
}

void OMACTest_ReceiveHandler (void* msg, UINT16 size){
	//return g_OMACTest.Receive(msg, size);
}

void OMACTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status){
	//g_OMACTest.SendAck(msg,size,status);
}

void OMACTest::GenerateRandVal(){
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(Test_0L_Timer_Pin, TRUE);
#endif
	//UINT16 randVal = g_omac_scheduler.m_seedGenerator.RandWithMask(&nextSeed, mask);
	int randVal1 = rand1.Next();
	UINT16 randVal = randVal1 & 0xFFFF;
#ifdef DEBUG_OMACTest
	CPU_GPIO_SetPinState(Test_0L_Timer_Pin, FALSE);
#endif
	hal_printf("%u\n", randVal);
}

BOOL OMACTest::Initialize(){
	CPU_GPIO_EnableOutputPin((GPIO_PIN) Test_0L_Timer_Pin, TRUE);
	CPU_GPIO_SetPinState((GPIO_PIN) Test_0L_Timer_Pin, FALSE);

	MyAppID = 3; //pick a number less than MAX_APPS currently 4.
	Config.Network = 138;
	Config.NeighborLivenessDelay = 900000;
	myEventHandler.SetReceiveHandler(OMACTest_ReceiveHandler);
	myEventHandler.SetSendAckHandler(OMACTest_SendAckHandler);
	MacId = OMAC;

	mask = 137 * 29 * (g_OMAC.GetAddress() + 1);
	nextSeed = 119 * 119 * (g_OMAC.GetAddress() + 1); // The initial seed
	//rand1.Initialize();
	rand1.Initialize(nextSeed);

	VirtTimer_Initialize();
	Mac_Initialize(&myEventHandler, MacId, MyAppID, Config.RadioID, (void*) &Config);

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(TEST_0L_TIMER, 0, TIMER_PERIOD*ONEMSEC_IN_USEC, FALSE, FALSE, Timer_Test_0L_Handler); //period (3rd argument) is in micro seconds
	ASSERT(rm == TimerSupported);

	return TRUE;
}

BOOL OMACTest::StartTest(){
	VirtualTimerReturnMessage rm;

	rm = VirtTimer_Start(TEST_0L_TIMER);
	ASSERT(rm == TimerSupported);

	return TRUE;
}

void OMACTest_Initialize(){
	g_OMACTest.Initialize();
	g_OMACTest.StartTest();
}


