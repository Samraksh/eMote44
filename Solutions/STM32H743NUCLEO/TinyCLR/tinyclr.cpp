////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <tinyclr_application.h>
#include <tinyhal.h>
#include <Samraksh/VirtualTimer.h>
#include "LinkTest.h"
//#include "platform_selector.h"

#define PKT_PERIOD_MICRO 100000 // 2 secs
//#define PKT_PERIOD_MICRO 2000000 // 2 secs

//#define PKT_PERIOD_MICRO 10000000 // 10 secs
#define TESTRADIONAME SX1276RADIO
//#define TESTRADIONAME RF231RADIO

#define LinkTest_PRINT_RX_PACKET_INFO 1

#define GPIO_0 _P(A,4)//
#define GPIO_1 _P(C,10)//
#define GPIO_2 _P(C,12)				

UINT16 testDest=0;
LinkTest gLinkTest(0,0);
extern UINT8 currentMacName;
extern NeighborTable g_NeighborTable;

void PrintHex(char *x, int size){
	for (int j=0;j<size; j++){
		hal_printf("0x%.2X , ",x[j]);
	}
	hal_printf("\r\n");
}


LinkTest::LinkTest( int seedValue, int numberOfEvents ){

}



BOOL LinkTest::Execute( int testLevel )
{
	Initialize();
	//VirtTimer_Start(LocalClockMonitor_TIMER1);
} //Execute


void LinkTest_ReceiveHandler (void* msg, UINT16 PacketType)
{
	UINT8 index;

	Message_15_4_t* packet_ptr = static_cast<Message_15_4_t*>(msg);
	UINT16 srcID= packet_ptr->GetHeader()->src;

	UINT64 packetID;
	memcpy(&packetID,packet_ptr->GetPayload(),sizeof(UINT64));
	if(g_NeighborTable.FindIndex(srcID, &index) == DS_Success)
	{
		++gLinkTest.rx_packet_count[index];
		#if LinkTest_PRINT_RX_PACKET_INFO
			hal_printf("\r\nLinkTest_RX: rcd pkt size %u, rx_packet_count = %u ", packet_ptr->GetHeader()->length, gLinkTest.rx_packet_count[index]);
			hal_printf("src = %u, PacketID = %llu \r\n", srcID, packetID );
		#endif
	}else {
		#if LinkTest_PRINT_RX_PACKET_INFO
			hal_printf("\r\nLinkTest_RX: rcd pkt size %u, from unkown neighbor ", packet_ptr->GetHeader()->length);
			hal_printf("ID = %u, PacketID = %llu \r\n", srcID, packetID );
		#endif
	}
}



void LinkTest_NeighborChangeHandler (INT16 args){
	hal_printf("\r\nNeighbor Change Notification for %u neighbors!\r\n", args);

	VirtTimer_Start(LocalClockMonitor_TIMER1);

	/*for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		if(g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers){
//			hal_printf("MACAddress = %u IsAvailableForUpperLayers = %u NumTimeSyncMessagesSent = %u NumberOfRecordedElements = %u \r\n"
//					, g_NeighborTable.Neighbor[i].MACAddress
//					, g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers
//					, g_NeighborTable.Neighbor[i].NumTimeSyncMessagesSent
//					, g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(g_NeighborTable.Neighbor[i].MACAddress)
//					);
			testDest= g_NeighborTable.Neighbor[i].MACAddress;
			VirtTimer_Start(LocalClockMonitor_TIMER1);
			hal_printf("Neighbor available MAC=%u \r\n", testDest);
			break;
		}
	}*/
}

void LinkTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus){
	//hal_printf("\r\n LinkTest_SendAckHandler: status = %u radioAckStatus = %u ", status, radioAckStatus);

	Message_15_4_t* pktPtr = static_cast<Message_15_4_t*>(msg);

	UINT64 packetID;
	if(size <= sizeof(UINT64)) {
		memcpy(&packetID,pktPtr->GetPayload(),size);
	}
	else {
		memcpy(&packetID,pktPtr->GetPayload(),sizeof(UINT64));
	}

	g_NeighborTable.DeletePacket(pktPtr);

	//hal_printf(" dest = %u  PacketID = %llu rx_packet_count = %llu \r\n",pktPtr->GetHeader()->dest, packetID,  gLinkTest.rx_packet_count );
}

void SendTimerHandler(void * arg){
	//hal_printf("Send Timer\r\n");
	//gLinkTest.SendMsg();
	DeviceStatus DS = DS_Success;
	CPU_GPIO_SetPinState(GPIO_0, TRUE);
	uint8_t detectionCount = 0;
	uint8_t cadCount = 0;
	
	for (cadCount = 0; cadCount < 1; cadCount++) {
		DS = CPU_Radio_ClearChannelAssesment(TESTRADIONAME);
		
//		while(CPU_Radio_CADCurrentRunningStatus(TESTRADIONAME) == 1) {
//			
//		};
		
//		if (CPU_Radio_CADCurrentStatus(TESTRADIONAME) == 1) {
//			detectionCount++;
//			hal_printf("CAD Detected\r\n");
//		}
//		else if (CPU_Radio_CADCurrentStatus(TESTRADIONAME) == 2) {
//			hal_printf("CAD Done\r\n");
//		}
	}

	//if(detectionCount > 3) {
	//	hal_printf("CAD Detected:%d\r\n", detectionCount);
	//	return DS_Success;
	//}
	//else {
	//	hal_printf("CAD Done:%d\r\n", detectionCount);
	//	return DS_Fail;
	//}
	
	CPU_GPIO_SetPinState(GPIO_0, FALSE);	
}

void LinkTest::Initialize()
{
	for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		rx_packet_count[i]=0;
		sent_packet_count[i]=0;
	}

	MyAppID = 3; //pick a number less than MAX_APPS currently 4.
	//Config.Network = 138;
	//Config.NeighborLivenessDelay = 620;
	MACReceiveFuncPtrType rx_fptr = &LinkTest_ReceiveHandler;
	myEventHandler.SetReceiveHandler(rx_fptr);
	SendAckFuncPtrType sa_fptr = &LinkTest_SendAckHandler;
	myEventHandler.SetSendAckHandler(sa_fptr);
	NeighborChangeFuncPtrType nc_fptr =  &LinkTest_NeighborChangeHandler;
	myEventHandler.SetNeighborChangeHandler(nc_fptr);
	MacId = CSMAMAC;
	//MacId = OMAC;


	VirtTimer_Initialize();

	MAC_Initialize(&myEventHandler,MacId, MyAppID, TESTRADIONAME, (void*) &Config);

	//This is important.
	//currentMacName == CSMAMAC;

	hal_printf("Initialize LinkTest\r\n");

	VirtualTimerReturnMessage rm;
	//rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1, 0, PKT_PERIOD_MICRO, FALSE, FALSE, SendTimerHandler, DEFAULT_TIMER);
	rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1, 0, PKT_PERIOD_MICRO, FALSE, FALSE, SendTimerHandler, LOW_DRIFT_TIMER);
	VirtTimer_Start(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);
};


void LinkTest::SendMsg(){
	UINT8 helloPayload[100];
	for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		if(g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers){
			testDest= g_NeighborTable.Neighbor[i].MACAddress;
			sent_packet_count[i]++;
			UINT16 dest=testDest;
			if(dest==0) dest=0xFFFF;
			DeviceStatus ret= MAC_Send(dest, 128,  (void *) helloPayload, 100);//sizeof(UINT64));
			if(ret!=DS_Success){
				hal_printf("LinkTest::SendMsg: Sending failed\r\n");
			}else {
				hal_printf("LinkTest::SendMsg: Sending to %u Success\r\n", dest);
			}
		}
	}

}



extern void HAL_CPU_Sleep(SLEEP_LEVEL level, UINT64 wakeEvents);

void CPU_Sleep(SLEEP_LEVEL level, UINT64 wakeEvents)
{
    HAL_CPU_Sleep(level, wakeEvents);
}

void Timer_Green_Handler(void *arg)
{
	static bool state = FALSE;
	if (state)
		state = FALSE;
	else
		state = TRUE;
	CPU_GPIO_SetPinState(LED1, state);
}

void Timer_Red_Handler(void *arg)
{
	static bool state = TRUE;
	if (state)
		state = FALSE;
	else
		state = TRUE;
	CPU_GPIO_SetPinState(LED3, state);
}

void Timer_RTC_Handler(void *arg)
{

	CPU_GPIO_SetPinState(GPIO_0, TRUE);
	CPU_GPIO_SetPinState(GPIO_0, FALSE);
}

void Timer_1_Handler(void *arg)
{
	CPU_GPIO_SetPinState(GPIO_1, TRUE);
	CPU_GPIO_SetPinState(GPIO_1, FALSE);
	//uint64_t time = CPU_Timer_CurrentTicks(LPTIM);
	//hal_printf("%llu\r\n", time);
}

void nathan_rtc_handler(void *arg) {
	static unsigned i=0;
	if (i++ & 1)
		CPU_GPIO_SetPinState(GPIO_2, TRUE);
	else
		CPU_GPIO_SetPinState(GPIO_2, FALSE);
}

// extern "C" {
// void MX_X_CUBE_AI_Init(void);
// int aiRun(const void *in_data, void *out_data);
// }

//static float in_data[64][51];
//static float out_data[8];

////////////////////////////////////////////////////////////////////////////////
void ApplicationEntryPoint()
{
    CLR_SETTINGS clrSettings;

	//hal_printf(" CLR 20 ");
	// Initial delay to allow UART terminals to start and catch startup messages
    HAL_Delay(5000);

	memset(&clrSettings, 0, sizeof(CLR_SETTINGS));

    clrSettings.MaxContextSwitches         = 50;
    clrSettings.WaitForDebugger            = false;
    clrSettings.EnterDebuggerLoopAfterExit = true;

	//CPU_GPIO_EnableOutputPin(LED1, TRUE);
    //CPU_GPIO_EnableOutputPin(LED3, TRUE);
	//CPU_GPIO_EnableOutputPin(GPIO_2, FALSE);

	// VirtTimer_SetTimer(VIRT_TIMER_LED_GREEN, 0, 500000, FALSE, FALSE, Timer_Green_Handler);
	// VirtTimer_Start(VIRT_TIMER_LED_GREEN);
	// VirtTimer_SetTimer(VIRT_TIMER_LED_RED, 0, 500000, FALSE, FALSE, Timer_Red_Handler, LOW_DRIFT_TIMER);
	// VirtTimer_Start(VIRT_TIMER_LED_RED);

	//VirtTimer_SetTimer(VIRT_TIMER_LED_RED, 0, 100000, FALSE, FALSE, nathan_rtc_handler, LOW_DRIFT_TIMER);
	//VirtTimer_Start(VIRT_TIMER_LED_RED);

	//MX_X_CUBE_AI_Init();
	//aiRun(in_data, out_data);

	//CPU_GPIO_EnableOutputPin(GPIO_0, FALSE);
	//CPU_GPIO_EnableOutputPin(GPIO_1, FALSE);
/*	CPU_GPIO_EnableOutputPin(GPIO_2, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_3, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_4, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_5, FALSE);
*/

	/*VirtTimer_SetTimer(VIRT_TIMER_TIME_TEST, 0, 1000000, FALSE, FALSE, Timer_1_Handler);
	VirtTimer_Start(VIRT_TIMER_TIME_TEST);
	VirtTimer_SetTimer(VIRT_TIMER_RTC_TEST, 0, 450000, FALSE, FALSE, Timer_RTC_Handler, LOW_DRIFT_TIMER);
	VirtTimer_Start(VIRT_TIMER_RTC_TEST);*/
	//VirtTimer_SetTimer(VIRT_TIMER_LED_GREEN, 0, 800000, FALSE, FALSE, Timer_Green_Handler, LOW_DRIFT_TIMER);
	//VirtTimer_Start(VIRT_TIMER_LED_GREEN);
	//I2S_Internal_Initialize();
	//I2S_Test();
    //hal_printf(" CLR 30 ");
	LinkTest test(0,0);

    test.Execute(0);

	
    ClrStartup( clrSettings );

	// while(1) {
		// while( HAL_CONTINUATION::Dequeue_And_Execute() == TRUE ) ;
		// __WFI();
	// }

#if !defined(BUILD_RTM)
    debug_printf( "Exiting.\r\n" );
#else
    ::CPU_Reset();
#endif
}

BOOL Solution_GetReleaseInfo(MfReleaseInfo& releaseInfo)
{
    MfReleaseInfo::Init(releaseInfo,
                        VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_REVISION,
                        OEMSYSTEMINFOSTRING, hal_strlen_s(OEMSYSTEMINFOSTRING)
                        );
    return TRUE; // alternatively, return false if you didn't initialize the releaseInfo structure.
}
