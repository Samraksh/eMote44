/*
 * TimesyncTest.cpp
 */

#include "OMACTest.h"
//#include "OMAConstants.h"
#define WLO_OMAC 0x00000010
extern void WakeLock(uint32_t lock);

//extern HALTimerManager gHalTimerManagerObject;
//extern VirtualTimer gVirtualTimerObject;
//extern VirtualTimerMapper gVirtualTimerMapperObject;
extern OMACTest gOMACTest;
extern OMACType g_OMAC;
extern UINT16 MF_NODE_ID;
//extern Buffer_15_4_t m_receive_buffer;

#define AUSTERE_NODE_ID_1 8334
#define AUSTERE_NODE_ID_2 14728

#define OMACTEST_PRINT_TIME_MISMATCH 0
#define OMACTEST_PRINT_RX_PACKET_INFO 1

#define MINEVENTTIM 1000

#define USEONESHOTTIMER TRUE
#define TESTRADIONAME SX1276RADIO
//#define TESTRADIONAME RF231RADIO

//NEIGHBORCLOCKMONITORPERIOD in ticks
//#define NEIGHBORCLOCKMONITORPERIOD 200000 800000
#define NEIGHBORCLOCKMONITORPERIOD_MICRO 1000000
#define NEIGHBORCLOCKMONITORPERIOD (g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO))
#define INITIALDELAY 100000

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
	static bool state = FALSE;
	if (state)
		state = FALSE;
	else
		state = TRUE;
	CPU_GPIO_SetPinState(LED3, state);
}


void ToggleLocalClockMonitorPIN(){
	//Toggle Pin State for monitoring with Logic Analyzer
	if(gOMACTest.LocalClkPINState){
		CPU_GPIO_SetPinState(gOMACTest.m_LOCALCLOCKMONITORPIN, false);
		gOMACTest.LocalClkPINState = false;
	}
	else {
		CPU_GPIO_SetPinState(gOMACTest.m_LOCALCLOCKMONITORPIN, true);
		gOMACTest.LocalClkPINState = true;
	}
}

void OMACTest_ReceiveHandler (void* msg, UINT16 PacketType){
	++gOMACTest.rx_packet_count ;
	Message_15_4_t* packet_ptr = static_cast<Message_15_4_t*>(msg);
	UINT64 packetID;
	memcpy(&packetID,packet_ptr->GetPayload(),sizeof(UINT64));
#if OMACTEST_PRINT_RX_PACKET_INFO
//	hal_printf("\r\n OMACTest_RX: rx_packet_count = %llu ", gOMACTest.rx_packet_count);
//	hal_printf("src = %u PacketID = %llu \r\n", packet_ptr->GetHeader()->src, packetID );
	hal_printf("\r\n OMACTest_RX: rx_packet_count = %s\r\n", l2s(gOMACTest.rx_packet_count,0));
	//hal_printf("src = %s ", l2s(packet_ptr->GetHeader()->src,0));
	//hal_printf("PacketID = %s \r\n", l2s(packetID,0));

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
	//hal_printf("\r\n OMACTest_SendAckHandler: status = %s radioAckStatus = %s\r\n", l2s(status,0), l2s(radioAckStatus,0));

	Message_15_4_t* packet_ptr = static_cast<Message_15_4_t*>(msg);
	DataMsg_t* data_msg = (DataMsg_t*) packet_ptr->GetPayload();
	UINT64 packetID;
	if(data_msg->size <= sizeof(UINT64)) memcpy(&packetID,data_msg->payload,data_msg->size);
	else memcpy(&packetID,data_msg->payload,sizeof(UINT64));

	g_NeighborTable.DeletePacket(packet_ptr);

	hal_printf("\r\n OMACTest_SendAckHandler: send_packet = %s \r\n", l2s(gOMACTest.sent_packet_count,0));
	//hal_printf(" dest = %u  PacketID = %llu rx_packet_count = %llu \r\n",packet_ptr->GetHeader()->dest, packetID,  gOMACTest.rx_packet_count );
	//hal_printf("\r\n dest = %s ",l2s(packet_ptr->GetHeader()->dest,0));
	//hal_printf("PacketID = %s ", l2s(packetID,0));
	//hal_printf("\r\nrx_packet_count = %s \r\n", l2s(gOMACTest.rx_packet_count,0));


}

void CMaxTSLocalClockMonitorTimerHandler(void * arg) {
	ToggleLocalClockMonitorPIN();
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
	ASSERT_SP(rm == TimerSupported);

	//ToggleLocalClockMonitorPIN();
	BOOL rv = gOMACTest.ScheduleNextLocalCLK();
	//ToggleLocalClockMonitorPIN();
	//ToggleLocalClockMonitorPIN();
	//ToggleLocalClockMonitorPIN();
	//ToggleLocalClockMonitorPIN();
	//ToggleLocalClockMonitorPIN();
	//rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, NEIGHBORCLOCKMONITORPERIOD_MICRO, USEONESHOTTIMER, OMACClockSpecifier);
	//ASSERT_SP(rm == TimerSupported);
	//rm = VirtTimer_Start(LocalClockMonitor_TIMER1);

}

void CMaxTSNeighborClockMonitorTimerHandler(void * arg) {

	UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(VIRT_CONT_TEST_TIMER1);
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
			CPU_GPIO_SetPinState(gOMACTest.m_NEIGHBORCLOCKMONITORPIN, false);
			gOMACTest.NeighborClkPINState = false;
		}
		else {
			CPU_GPIO_SetPinState(gOMACTest.m_NEIGHBORCLOCKMONITORPIN, true);
			gOMACTest.NeighborClkPINState = true;
		}
	}
#if OMACTEST_PRINT_TIME_MISMATCH
		if( (y > gOMACTest.TargetTimeinTicks) && (y -  gOMACTest.TargetTimeinTicks> 8000)
		 || (y < gOMACTest.TargetTimeinTicks) && (gOMACTest.TargetTimeinTicks - y > 8000)
		){

			//hal_printf("\r\n TargetTimeinTicks = %llu y = %llu \r\n", gOMACTest.TargetTimeinTicks, y );
			hal_printf("\r\n TargetTimeinTicks = %s y = %s \r\n", l2s(gOMACTest.TargetTimeinTicks,0), l2s(y,0) );

		}
#endif

	BOOL rv = gOMACTest.ScheduleNextNeighborCLK();
	if(rv){
		gOMACTest.SendPacketToNeighbor();
	}

}


// TIMESYNCTEST

BOOL OMACTest::Initialize(){
	WakeLock(WLO_OMAC);
	sequence_number = 0;
	sent_packet_count = 0;
	rx_packet_count = 0;


	LocalClkPINState = true;
	NeighborClkPINState = true;

	LocalClockMonitorFrameNum = 0;
	NeighborClockMonitorFrameNum = 0;
	NextEventTime = 0;

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

	MAC_Initialize(&myEventHandler, MacId, MyAppID, TESTRADIONAME, (void*) &Config);

	hal_printf("Initialize OMACTest");



#ifdef DEBUG_GPIO_EMOTE_AUSTERE
	if(AUSTERE_RADIO_GPIO_PIN == DISABLED_PIN ) {
		hal_printf("OMACTest: Periodic messaging test is running with AUSTERE_RADIO_GPIO_PIN disabled!!");
		this->m_LOCALCLOCKMONITORPIN = DISABLED_PIN;
		this->m_NEIGHBORCLOCKMONITORPIN = DISABLED_PIN;
	}
	else if(g_OMAC.GetMyID() == AUSTERE_NODE_ID_1){
		this->m_LOCALCLOCKMONITORPIN = AUSTERE_RADIO_GPIO_PIN;
		this->m_NEIGHBORCLOCKMONITORPIN = DISABLED_PIN;
		hal_printf("OMACTest: AUSTERE_NODE_ID_1");
	}
	else if(g_OMAC.GetMyID() == AUSTERE_NODE_ID_2){
		this->m_LOCALCLOCKMONITORPIN = DISABLED_PIN;
		this->m_NEIGHBORCLOCKMONITORPIN = AUSTERE_RADIO_GPIO_PIN;
		hal_printf("OMACTest: AUSTERE_NODE_ID_2");
	}
	else{
		hal_printf("OMACTest periodic messaging test is running with UNKNOWN IDS!!");
	}
#else
	this->m_LOCALCLOCKMONITORPIN = LOCALCLOCKMONITORPIN;
	this->m_NEIGHBORCLOCKMONITORPIN = NEIGHBORCLOCKMONITORPIN;
#endif

//	UINT64 i =0;
//	UINT64 j =0;
//	while(i<10000000){
//		if((CPU_Radio_ChangeChannel(TESTRADIONAME, 4)) == DS_Success) {
//			hal_printf("Radio channel is changed to channel 4");
//			break;
//		}
//		else{
//			hal_printf("Radio channel change failed!");
//		}
//		j = 0;
//		while(j<10000000){
//			++j;
//		}
//		if(i == 10000000){
//			SOFT_BREAKPOINT();
//			return FALSE;
//		}
//	}

	CPU_GPIO_EnableOutputPin(gOMACTest.m_NEIGHBORCLOCKMONITORPIN, TRUE);
	CPU_GPIO_EnableOutputPin(gOMACTest.m_LOCALCLOCKMONITORPIN, TRUE);
	CPU_GPIO_SetPinState(gOMACTest.m_NEIGHBORCLOCKMONITORPIN, FALSE);
	CPU_GPIO_SetPinState(gOMACTest.m_LOCALCLOCKMONITORPIN, FALSE);


	UINT64 TicksTillNextEvent = 0;
	UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	while(NextEventTime <= y || TicksTillNextEvent <= MINEVENTTIM ){
		LocalClockMonitorFrameNum = LocalClockMonitorFrameNum + 1;
		NextEventTime = LocalClockMonitorFrameNum * (g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO));
		y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		TicksTillNextEvent = NextEventTime - y;
	}

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1, 0, NEIGHBORCLOCKMONITORPERIOD_MICRO, USEONESHOTTIMER, FALSE, CMaxTSLocalClockMonitorTimerHandler, OMACClockSpecifier);
	ASSERT_SP(rm == TimerSupported);
	rm = VirtTimer_SetTimer(VIRT_CONT_TEST_TIMER1, 0, NEIGHBORCLOCKMONITORPERIOD_MICRO, USEONESHOTTIMER, FALSE, CMaxTSNeighborClockMonitorTimerHandler, OMACClockSpecifier);
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
	//rm = VirtTimer_Start(VIRT_CONT_TEST_TIMER1);
	//ASSERT_SP(rm == TimerSupported);

	return TRUE;
}

void OMACTest::SendPacketToNeighbor(){
	UINT16 Nbr2beFollowed;
	if(g_NeighborTable.Neighbor[0].IsAvailableForUpperLayers == true){
		Nbr2beFollowed = g_NeighborTable.Neighbor[0].MACAddress; //g_NeighborTable.Neighbor[0].MACAddress;
		if(!NeighborFound){
			NeighborFound = true;
			hal_printf("\r\n NEIGHBOR FOUND!! \r\n");
		}


	}
	else{
		Nbr2beFollowed = 0;
		if(NeighborFound){
			NeighborFound = false;
			hal_printf("\r\n NEIGHBOR LOST!! \r\n");
		}
	}
	VirtualTimerReturnMessage rm;

	if (g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) > 2 ) {//if ( g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(Nbr2beFollowed) >= 5 ){
		++(sequence_number);
		//hal_printf("\r\n sequence_number = %s \r\n",l2s(sequence_number,0));
	//	if( (sent_packet_count % 20 < 10 && sequence_number % 10 == 0)
	//	||  (sent_packet_count % 20 >= 10 && sequence_number % 300 == 0)
	//	)
		if (sequence_number % 60 == 0)
		{
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
					if(g_OMAC.Send(Nbr2beSent, TYPE31, &sent_packet_count, sizeof(UINT64))){
						//hal_printf("\r\n PACKET ACCEPTED Dest = %u PacketID = %llu!! \r\n", Nbr2beSent, sent_packet_count);
						hal_printf("\r\n PACKET ACCEPTED Dest = %s PacketID = %s!! \r\n", l2s(Nbr2beSent,0), l2s(sent_packet_count,0));
						++sent_packet_count;
					}
					else{
						//hal_printf("\r\n PACKET REJECTED!! Dest = %u PacketID = %llu!!\r\n", Nbr2beSent, sent_packet_count);
						hal_printf("\r\n PACKET REJECTED!! Dest = %s PacketID = %s!!\r\n", l2s(Nbr2beSent,0), l2s(sent_packet_count,0));
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
			hal_printf("\r\n NEIGHBOR FOUND!! \r\n");
		}


	}
	else{
		Nbr2beFollowed = 0;
		if(NeighborFound){
			NeighborFound = false;
			hal_printf("\r\n NEIGHBOR LOST!! \r\n");
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

		rm = VirtTimer_Change(VIRT_CONT_TEST_TIMER1, 0, MicSTillNextEvent + ProcessingLatency, USEONESHOTTIMER, OMACClockSpecifier);
		ASSERT_SP(rm == TimerSupported);
		rm = VirtTimer_Start(VIRT_CONT_TEST_TIMER1);
		ASSERT_SP(rm == TimerSupported);

//		if(MicSTillNextEvent + ProcessingLatency < NEIGHBORCLOCKMONITORPERIOD - 8000 || MicSTillNextEvent + ProcessingLatency > NEIGHBORCLOCKMONITORPERIOD + 8000){
//			hal_printf("Possible incorrect estimation: T = %llu MicSTillNextEvent = %llu ProcessingLatency = %llu", MicSTillNextEvent + ProcessingLatency, MicSTillNextEvent, ProcessingLatency);
//		}
		return TRUE;
	}
	else {
		rm = VirtTimer_Start(VIRT_CONT_TEST_TIMER1);
		ASSERT_SP(rm == TimerSupported);
		return FALSE;
	}


}

BOOL OMACTest::ScheduleNextLocalCLK(){
		UINT64 n2= NEIGHBORCLOCKMONITORPERIOD_MICRO;

		//ToggleLocalClockMonitorPIN();
		UINT32 MicSTillNextEvent;
		VirtualTimerReturnMessage rm;
		UINT64 y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		UINT64 ystart = y;
		//UINT64 NextEventTime = 0;
		UINT64 TicksTillNextEvent = 0;
		while(NextEventTime <= y || TicksTillNextEvent <= MINEVENTTIM ){
			LocalClockMonitorFrameNum = LocalClockMonitorFrameNum + 1;
			//NextEventTime = LocalClockMonitorFrameNum * (g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO));
			NextEventTime += (g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO));
			y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
			TicksTillNextEvent = NextEventTime - y;
		}
		//hal_printf("SNLC1: %llu - y= %llu, TTNE= %llu, NET= %llu, YS= %llu, CPT= %llu \r\n" ,LocalClockMonitorFrameNum, y,TicksTillNextEvent,NextEventTime, ystart, g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO));

		//ToggleLocalClockMonitorPIN();



//		rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, NEIGHBORCLOCKMONITORPERIOD_MICRO, USEONESHOTTIMER, OMACClockSpecifier);
//		ASSERT_SP(rm == TimerSupported);
//		rm = VirtTimer_Start(LocalClockMonitor_TIMER1);


		/*y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		//NextEventTime = 0;
		//TicksTillNextEvent = 0;
		while(NextEventTime <= y || TicksTillNextEvent <= MINEVENTTIM ){
			LocalClockMonitorFrameNum = LocalClockMonitorFrameNum + 1;
			//NextEventTime = LocalClockMonitorFrameNum * (g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO), NEIGHBORCLOCKMONITORPERIOD_MICRO, g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO));
			NextEventTime += (g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO));
			y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
			TicksTillNextEvent = NextEventTime - y;
		}
		hal_printf("SNLC2: %llu - y= %llu, TTNE= %llu, NET= %llu, CP= %llu, CPT= %llu  \r\n" ,LocalClockMonitorFrameNum, y,TicksTillNextEvent,NextEventTime, NEIGHBORCLOCKMONITORPERIOD_MICRO, g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO));
	    */


		//ToggleLocalClockMonitorPIN();
		MicSTillNextEvent = (UINT32) (g_OMAC.m_Clock.ConvertTickstoMicroSecs(TicksTillNextEvent)) ;
		rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, MicSTillNextEvent, USEONESHOTTIMER, OMACClockSpecifier);
		ASSERT_SP(rm == TimerSupported);
		y = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		//hal_printf("SNLC3: %llu - y= %llu, TTNE= %llu, NET= %llu, CP= %llu, CPT= %llu  \r\n" ,LocalClockMonitorFrameNum, y,TicksTillNextEvent,NextEventTime, MicSTillNextEvent, g_OMAC.m_Clock.ConvertMicroSecstoTicks(NEIGHBORCLOCKMONITORPERIOD_MICRO));

	//	ToggleLocalClockMonitorPIN();
//		ToggleLocalClockMonitorPIN();


		rm = VirtTimer_Start(LocalClockMonitor_TIMER1);

		//ToggleLocalClockMonitorPIN();
		//ToggleLocalClockMonitorPIN();

		return TRUE;
}

void OMACTest_Initialize(){
	BOOL ret;
	ret = gOMACTest.Initialize();
	ret = gOMACTest.StartTest();
}

void ApplicationEntryPoint()
{
	CPU_GPIO_EnableOutputPin(LED1, TRUE);
	CPU_GPIO_EnableOutputPin(LED3, TRUE);

	VirtTimer_SetTimer(VIRT_TIMER_LED_GREEN, 0, 700000, FALSE, FALSE, Timer_Green_Handler);
	VirtTimer_Start(VIRT_TIMER_LED_GREEN);
	VirtTimer_SetTimer(VIRT_TIMER_LED_RED, 0, 800000, FALSE, FALSE, Timer_Red_Handler, RTC_32BIT);
	VirtTimer_Start(VIRT_TIMER_LED_RED);
	
    do
    {
    	OMACTest omacTest();
    	OMACTest_Initialize();

    	//dsTestObject.Execute(TEST_INITIALIZATION);

    } while(false); // run only once!

    while(true){
    	::Events_WaitForEvents(0, 100);
    }
}
