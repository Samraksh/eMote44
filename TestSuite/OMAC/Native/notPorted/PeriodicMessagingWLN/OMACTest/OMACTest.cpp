/*
 * TimesyncTest.cpp
 */

#include "OMACTest.h"
//#include "OMAConstants.h"


//extern HALTimerManager gHalTimerManagerObject;
//extern VirtualTimer gVirtualTimerObject;
//extern VirtualTimerMapper gVirtualTimerMapperObject;
extern OMACTest gOMACTest;
extern OMACType g_OMAC;
extern UINT16 MF_NODE_ID;
//extern Buffer_15_4_t m_receive_buffer;

#define OMACTEST_PRINT_TIME_MISMATCH 0
#define OMACTEST_PRINT_RX_PACKET_INFO 1

#define MINEVENTTIM 1000

#define USEONESHOTTIMER TRUE

//NEIGHBORCLOCKMONITORPERIOD in ticks
//#define NEIGHBORCLOCKMONITORPERIOD 200000 800000
#define NEIGHBORCLOCKMONITORPERIOD_MICRO (100000)
#define NEIGHBORCLOCKMONITORPERIOD (g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO))
#define INITIALDELAY 100000

void OMACTest_ReceiveHandler (void* msg, UINT16 PacketType){
	++gOMACTest.rx_packet_count ;
	Message_15_4_t* packet_ptr = static_cast<Message_15_4_t*>(msg);
	UINT64 packetID;
	memcpy(&packetID,packet_ptr->GetPayload(),sizeof(UINT64));
#if OMACTEST_PRINT_RX_PACKET_INFO
	hal_printf("\r\n OMACTest_RX: rx_packet_count = %llu ", gOMACTest.rx_packet_count);
	hal_printf("src = %u PacketID = %llu \r\n", packet_ptr->GetHeader()->src, packetID );
#endif
}

void OMACTest_NeighborChangeHandler (INT16 args){
	hal_printf("\r\n Neighbor Change Notification for %u neighbors!\r\n", args);

	for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		if(g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers){
//			hal_printf("MACAddress = %u IsAvailableForUpperLayers = %u NumTimeSyncMessagesSent = %u NumberOfRecordedElements = %u \r\n"
//					, g_NeighborTable.Neighbor[i].MACAddress
//					, g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers
//					, g_NeighborTable.Neighbor[i].NumTimeSyncMessagesSent
//					, g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(g_NeighborTable.Neighbor[i].MACAddress)
//					);
			hal_printf("MAC=%u \r\n", g_NeighborTable.Neighbor[i].MACAddress);
		}

	}
}

void OMACTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus){
	hal_printf("\r\n OMACTest_SendAckHandler: status = %u radioAckStatus = %u ", status, radioAckStatus);

	Message_15_4_t* packet_ptr = static_cast<Message_15_4_t*>(msg);
	DataMsg_t* data_msg = (DataMsg_t*) packet_ptr->GetPayload();
	UINT64 packetID;
	if(data_msg->size <= sizeof(UINT64)) memcpy(&packetID,data_msg->payload,data_msg->size);
	else memcpy(&packetID,data_msg->payload,sizeof(UINT64));

	hal_printf(" dest = %u  PacketID = %llu rx_packet_count = %llu \r\n",packet_ptr->GetHeader()->dest, packetID,  gOMACTest.rx_packet_count );


}

void CMaxTSLocalClockMonitorTimerHandler(void * arg) {
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);


	//Toggle Pin State for monitoring with Logic Analyzer
	if(gOMACTest.LocalClkPINState){
		CPU_GPIO_SetPinState(LOCALCLOCKMONITORPIN, false);
		gOMACTest.LocalClkPINState = false;
	}
	else {
		CPU_GPIO_SetPinState(LOCALCLOCKMONITORPIN, true);
		gOMACTest.LocalClkPINState = true;
	}
	BOOL rv = gOMACTest.ScheduleNextLocalCLK();
}

void CMaxTSNeighborClockMonitorTimerHandler(void * arg) {

	UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(NeighborClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);

	//Toggle Pin State for monitoring with Logic Analyzer
	//UINT16 Nbr2beFollowed = g_OMAC.Neighbor2beFollowed;
	UINT16 Nbr2beFollowed;
	if(g_NeighborTable.Neighbor[0].IsAvailableForUpperLayers == true){
		Nbr2beFollowed = g_NeighborTable.Neighbor[0].MACAddress;
	}
	else{
		Nbr2beFollowed = 0;
	}
	if(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) > 2 ) {//if ( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) >= 5 ))
		if(gOMACTest.NeighborClkPINState){
			CPU_GPIO_SetPinState(NEIGHBORCLOCKMONITORPIN, false);
			gOMACTest.NeighborClkPINState = false;
		}
		else {
			CPU_GPIO_SetPinState(NEIGHBORCLOCKMONITORPIN, true);
			gOMACTest.NeighborClkPINState = true;
		}
	}
#if OMACTEST_PRINT_TIME_MISMATCH
		if( (y > gOMACTest.TargetTimeinTicks) && (y -  gOMACTest.TargetTimeinTicks> 8000)
		 || (y < gOMACTest.TargetTimeinTicks) && (gOMACTest.TargetTimeinTicks - y > 8000)
		){

			hal_printf("\r\n TargetTimeinTicks = %llu y = %llu \r\n", gOMACTest.TargetTimeinTicks, y );

		}
#endif

	BOOL rv = gOMACTest.ScheduleNextNeighborCLK();
	if(rv){
		gOMACTest.SendPacketToNeighbor();
	}

}


// TIMESYNCTEST

BOOL OMACTest::Initialize(){

	sequence_number = 0;
	sent_packet_count = 0;
	rx_packet_count = 0;

	CPU_GPIO_EnableOutputPin(NEIGHBORCLOCKMONITORPIN, TRUE);
	CPU_GPIO_EnableOutputPin(LOCALCLOCKMONITORPIN, TRUE);
	CPU_GPIO_SetPinState(NEIGHBORCLOCKMONITORPIN, FALSE);
	CPU_GPIO_SetPinState(LOCALCLOCKMONITORPIN, FALSE);
	LocalClkPINState = true;
	NeighborClkPINState = true;

	LocalClockMonitorFrameNum = 0;
	NeighborClockMonitorFrameNum = 0;

	NeighborFound = false;

	MyAppID = 3; //pick a number less than MAX_APPS currently 4.
	//Config.Network = 138;
	Config.NeighborLivenessDelay = 620;
	MACReceiveFuncPtrType rx_fptr = &OMACTest_ReceiveHandler;
	myEventHandler.SetReceiveHandler(rx_fptr);
	SendAckFuncPtrType sa_fptr = &OMACTest_SendAckHandler;
	myEventHandler.SetSendAckHandler(sa_fptr);
	NeighborChangeFuncPtrType nc_fptr =  &OMACTest_NeighborChangeHandler;
	myEventHandler.SetNeighborChangeHandler(nc_fptr);
	MacId = OMAC;
	VirtTimer_Initialize();

	MAC_Initialize(&myEventHandler,MacId, MyAppID, SI4468_SPI2, (void*) &Config);



//		UINT64 i =0;
//		UINT64 j =0;
//		while(i<10000000){
//			if((CPU_Radio_ChangeChannel(SI4468_SPI2, 2)) == DS_Success) {
//				break;
//			}
//			j = 0;
//			while(j<10000000){
//				++j;
//			}
//			if(i == 10000000){
//				SOFT_BREAKPOINT();
//				return FALSE;
//			}
//		}



	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1, 0, NEIGHBORCLOCKMONITORPERIOD_MICRO, USEONESHOTTIMER, FALSE, CMaxTSLocalClockMonitorTimerHandler, OMACClockSpecifier);
	ASSERT_SP(rm == TimerSupported);
	rm = VirtTimer_SetTimer(NeighborClockMonitor_TIMER1, 0, NEIGHBORCLOCKMONITORPERIOD_MICRO, USEONESHOTTIMER, FALSE, CMaxTSNeighborClockMonitorTimerHandler, OMACClockSpecifier);
	ASSERT_SP(rm == TimerSupported);


	return TRUE;
}

BOOL OMACTest::StartTest(){
	VirtualTimerReturnMessage rm;

	UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	UINT64 LocalClockMonitorFrameNum = y /( (UINT64) NEIGHBORCLOCKMONITORPERIOD );

	ScheduleNextLocalCLK();
	ScheduleNextNeighborCLK();

	rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);
	rm = VirtTimer_Start(NeighborClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);

	return TRUE;
}

void OMACTest::SendPacketToNeighbor(){
	UINT16 Nbr2beFollowed;
	if(g_NeighborTable.Neighbor[0].IsAvailableForUpperLayers == true){
		Nbr2beFollowed = g_NeighborTable.Neighbor[0].MACAddress; //g_NeighborTable.Neighbor[0].MACAddress;
		if(!NeighborFound){
			NeighborFound = true;
			hal_printf("\n NEIGHBOR FOUND!! \n");
		}


	}
	else{
		Nbr2beFollowed = 0;
		if(NeighborFound){
			NeighborFound = false;
			hal_printf("\n NEIGHBOR LOST!! \n");
		}
	}
	VirtualTimerReturnMessage rm;

	if (g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) > 2 ) {//if ( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) >= 5 ){
		++(sequence_number);
		//hal_printf("\n sequence_number = %Lu \n", sequence_number);
		if( (sent_packet_count % 20 < 10 && sequence_number % 3000 == 0)
		||  (sent_packet_count % 20 >= 10 && sequence_number % 3000 == 0)
				) {

			//Choose neighbor to send
			hal_printf("\r\n Choosing Neighbor \r\n");
				UINT16 Nbr2beSent = 0;
				UINT8 chosen_index = sent_packet_count%g_NeighborTable.PreviousNumberOfNeighbors();
				for(UINT8 i = 0; i < 8 ; ++i){
					if(g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers){
						if(chosen_index == 0){
							Nbr2beSent = g_NeighborTable.Neighbor[i].MACAddress;
							break;
						}
						else if(chosen_index > 0){
							--chosen_index;
						}
						else{
							hal_printf("\r\n Error in Choosing Neeighbor! \r\n");
							break;
						}
					}
				}
				hal_printf("\r\n Chosen Neighbor = %u \r\n", Nbr2beSent);
				if(Nbr2beSent){
					if(g_OMAC.Send(Nbr2beSent, 128, &sent_packet_count, sizeof(UINT64))){
						hal_printf("\r\n PACKET ACCEPTED Dest = %u PacketID = %llu!! \r\n", Nbr2beSent, sent_packet_count);
						++sent_packet_count;
					}
					else{
						hal_printf("\r\n PACKET REJECTED!! Dest = %u PacketID = %llu!!\r\n", Nbr2beSent, sent_packet_count);
					}
				}
		}

	}
}

BOOL OMACTest::ScheduleNextNeighborCLK(){




	UINT16 Nbr2beFollowed;
	if(g_NeighborTable.Neighbor[0].IsAvailableForUpperLayers == true){
		Nbr2beFollowed = g_NeighborTable.Neighbor[0].MACAddress; //g_NeighborTable.Neighbor[0].MACAddress;
		if(!NeighborFound){
			NeighborFound = true;
			hal_printf("\n NEIGHBOR FOUND!! \n");
		}


	}
	else{
		Nbr2beFollowed = 0;
		if(NeighborFound){
			NeighborFound = false;
			hal_printf("\n NEIGHBOR LOST!! \n");
		}
	}
	VirtualTimerReturnMessage rm;

	if (g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) > 2 ) {//if ( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) >= 5 ){
		UINT64 NextEventTime = 0, neighborTime = 0, y = 0;
		UINT64 LocalFireUpTime = 0;
		UINT64 TicksTillNextEvent;


		//y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		while (LocalFireUpTime <= y || TicksTillNextEvent <= MINEVENTTIM){
			y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
			neighborTime = g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Local2NeighborTime(Nbr2beFollowed, y);
			//neighborTime = y;
			NeighborClockMonitorFrameNum =  neighborTime / ((UINT64)NEIGHBORCLOCKMONITORPERIOD);
			while(NextEventTime <= neighborTime){
				NeighborClockMonitorFrameNum = NeighborClockMonitorFrameNum + 1;
				NextEventTime = ( NeighborClockMonitorFrameNum ) * ((UINT64)NEIGHBORCLOCKMONITORPERIOD);
			}
			LocalFireUpTime = g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.Neighbor2LocalTime(Nbr2beFollowed, NextEventTime);
			//LocalFireUpTime = NextEventTime;
			TicksTillNextEvent = LocalFireUpTime - y;
		}
		UINT32 MicSTillNextEvent = (UINT32) (g_OMAC.m_Clock.ConvertTickstoMicroSecs(TicksTillNextEvent));
		UINT32 ProcessingLatency = (UINT32) (g_OMAC.m_Clock.ConvertTickstoMicroSecs( g_OMAC.m_Clock.GetCurrentTimeinTicks() - y));

		TargetTimeinTicks = g_OMAC.m_Clock.GetCurrentTimeinTicks() + TicksTillNextEvent;

		rm = VirtTimer_Change(NeighborClockMonitor_TIMER1, 0, MicSTillNextEvent + ProcessingLatency, USEONESHOTTIMER, OMACClockSpecifier);
		ASSERT_SP(rm == TimerSupported);
		rm = VirtTimer_Start(NeighborClockMonitor_TIMER1);
		ASSERT_SP(rm == TimerSupported);

//		if(MicSTillNextEvent + ProcessingLatency < NEIGHBORCLOCKMONITORPERIOD - 8000 || MicSTillNextEvent + ProcessingLatency > NEIGHBORCLOCKMONITORPERIOD + 8000){
//			hal_printf("Possible incorrect estimation: T = %llu MicSTillNextEvent = %llu ProcessingLatency = %llu", MicSTillNextEvent + ProcessingLatency, MicSTillNextEvent, ProcessingLatency);
//		}
		return TRUE;
	}
	else {
		rm = VirtTimer_Start(NeighborClockMonitor_TIMER1);
		ASSERT_SP(rm == TimerSupported);
		return FALSE;
	}


}

BOOL OMACTest::ScheduleNextLocalCLK(){




		VirtualTimerReturnMessage rm;
		UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		UINT64 NextEventTime = 0;
		UINT64 TicksTillNextEvent = 0;
		while(NextEventTime <= y || TicksTillNextEvent <= MINEVENTTIM ){
			LocalClockMonitorFrameNum = LocalClockMonitorFrameNum + 1;
			NextEventTime = LocalClockMonitorFrameNum * (g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO));
			TicksTillNextEvent = NextEventTime - y;
		}

		UINT32 MicSTillNextEvent = (UINT32) (g_OMAC.m_Clock.ConvertTickstoMicroSecs(TicksTillNextEvent)) ;
		rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, MicSTillNextEvent, USEONESHOTTIMER, OMACClockSpecifier);
		ASSERT_SP(rm == TimerSupported);
		rm = VirtTimer_Start(LocalClockMonitor_TIMER1);

		return TRUE;
}

void OMACTest_Initialize(){
	BOOL ret;
	ret = gOMACTest.Initialize();
	ret = gOMACTest.StartTest();
}


