/*
 * DiscoveryTimesyncHandler.cpp
 *
 *  Created on: Aug 30, 2012
 *      Author: Mukundan Sridharan
 *
 *  Modified on: Oct 30, 2015
 *  	Authors: Bora Karaoglu; Ananth Muralidharan
 *
 *  Copyright The Samraksh Company
 */

#include <Samraksh/Radio_decl.h>
#include <Samraksh/MAC/OMAC/DiscoveryHandler.h>
#include <Samraksh/MAC/OMAC/OMACConstants.h>
#include <Samraksh/MAC/OMAC/Scheduler.h>
#include <Samraksh/MAC/OMAC/OMAC.h>
//#include <DeviceCode\LCD_PCF85162_HAL\LCD_PCF85162.h>

extern OMACType g_OMAC;

UINT16 DiscoveryHandler::m_period1 = 0;
UINT16 DiscoveryHandler::m_period2 = 0;

//UINT16 CONTROL_P1[] = {2131, 2099, 2129, 2111, 2153, 2113, 2137};
//UINT16 CONTROL_P2[] = {8429, 8419, 8623, 8443, 8627, 8447, 8467};
//UINT16 CONTROL_P3[] = {2131, 2099, 2129, 2111, 2153, 2113, 2137};
//UINT16 CONTROL_P4[] = {8429, 8419, 8623, 8443, 8627, 8447, 8467};

//UINT16 CONTROL_P1[] = {911, 727, 787, 769, 773, 853, 797};
//UINT16 CONTROL_P2[] = {2131, 2099, 2129, 2111, 2153, 2113, 2137};
//UINT16 CONTROL_P3[] = {911, 727, 787, 769, 773, 853, 797};
//UINT16 CONTROL_P4[] = {2131, 2099, 2129, 2111, 2153, 2113, 2137};

//Expected Disco Time (2 disco receptions) 28.21hours,  Typical MaxDiscoTime Time (2 disco receptions) 37.62h, MaxDiscoTime = 151.6hours
UINT16 CONTROL_P3[] = {2099, 2111, 2113, 2129, 2131, 2137, 2153};
UINT16 CONTROL_P4[] = {8627, 8623, 8467, 8447, 8443, 8429, 8419};
//UINT16 CONTROL_P1[] = {197, 157, 151, 163, 211, 113, 127};
//UINT16 CONTROL_P2[] = {911, 727, 787, 769, 773, 853, 797};

//UINT16 CONTROL_P3[] = {197, 157, 151, 163, 211, 113, 127};
//UINT16 CONTROL_P4[] = {911, 727, 787, 769, 773, 853, 797};
//UINT16 CONTROL_P1[] = {19, 17, 13, 37, 11, 5, 7};
//UINT16 CONTROL_P2[] = {67, 43, 53, 47, 61, 59};

UINT16 CONTROL_P1[] = {47, 37, 43, 37, 53, 29, 31};
UINT16 CONTROL_P2[] = {227, 181, 197, 191, 211, 199};

//Expected disco time(2 disco receptions) 2.63 mins, Typical MaxDiscoTime Time (2 disco receptions) 3.51 mins, Non typical MaxDiscoTime = 11.46 mins
//#if defined(PLATFORM_ARM_Austere) || defined(PLATFORM_ARM_EmoteDotNow)
//UINT16 CONTROL_P1[] = { 67,  71,  79,  83,  89,  97, 101};
//UINT16 CONTROL_P2[] = {257, 251, 241, 239, 233, 229, 227};
//UINT16 CONTROL_P2[] = {911, 727, 787, 769, 773, 853, 797};
//#else //#if defined(PLATFORM_ARM_EmoteDotNow)
//UINT16 CONTROL_P1[] = {2099, 2111, 2113, 2129, 2131, 2137, 2153};
//UINT16 CONTROL_P2[] = {8627, 8623, 8467, 8447, 8443, 8429, 8419};
//#endif


/*
 *
 */
void PublicBeaconNCallback(void * param){
	g_OMAC.m_omac_scheduler.m_DiscoveryHandler.BeaconNTimerHandler();
}

void PublicDiscoPostExec(void * param){
	g_OMAC.m_omac_scheduler.m_DiscoveryHandler.FailsafeStop();
	g_OMAC.m_omac_scheduler.m_DiscoveryHandler.PostExecuteEvent();
}


/*
 *
 */
void DiscoveryHandler::Initialize(UINT8 radioID, UINT8 macID){
#ifdef OMAC_DEBUG_GPIO
	OMAC_CPU_GPIO_EnableOutputPin( DISCO_SYNCSENDPIN, TRUE);
	OMAC_CPU_GPIO_EnableOutputPin( DISCO_SYNCRECEIVEPIN, TRUE);
	OMAC_CPU_GPIO_SetPinState(  DISCO_SYNCSENDPIN, FALSE );
	OMAC_CPU_GPIO_SetPinState(  DISCO_SYNCRECEIVEPIN, FALSE );
	CPU_GPIO_EnableOutputPin(  DISCO_NEXT_EVENT, FALSE );
	CPU_GPIO_SetPinState(  DISCO_NEXT_EVENT, FALSE );
	CPU_GPIO_EnableOutputPin(DISCO_BEACON_N, TRUE);
	CPU_GPIO_SetPinState( DISCO_BEACON_N, FALSE );
	CPU_GPIO_EnableOutputPin(OMAC_DISCO_POST_EXEC, TRUE);
	CPU_GPIO_SetPinState( OMAC_DISCO_POST_EXEC, FALSE );
	CPU_GPIO_EnableOutputPin(OMAC_DISCO_EXEC_EVENT, TRUE);
	CPU_GPIO_SetPinState( OMAC_DISCO_EXEC_EVENT, FALSE );
	CPU_GPIO_EnableOutputPin(OMAC_DISCO_EXEC_EVENT, TRUE);
	CPU_GPIO_SetPinState( OMAC_DISCO_EXEC_EVENT, FALSE );
	CPU_GPIO_EnableOutputPin(OMAC_DISCO_BEACON_ACK_HANDLER_PIN, TRUE);
	CPU_GPIO_SetPinState( OMAC_DISCO_BEACON_ACK_HANDLER_PIN, FALSE );
	CPU_GPIO_EnableOutputPin(OMAC_DISCO_BEACONNTIMERHANDLER_PIN, TRUE);
	CPU_GPIO_SetPinState( OMAC_DISCO_BEACONNTIMERHANDLER_PIN, FALSE );
	
	CPU_GPIO_EnableOutputPin( RX_RADIO_TURN_ON, TRUE);
	CPU_GPIO_SetPinState( RX_RADIO_TURN_ON, FALSE );
	CPU_GPIO_EnableOutputPin( RX_RADIO_TURN_OFF, TRUE);
	CPU_GPIO_SetPinState( RX_RADIO_TURN_OFF, FALSE );


#endif

	m_state = DISCO_INITIAL;

	m_num_sleep_retry_attempts = 0;

	firstHighRateDiscoTimeinSlotNum = 0;
	PermanentlyDecreaseDiscoRate(); //Select initial disco parameters

#ifdef OMAC_DEBUG_PRINTF
	OMAC_HAL_PRINTF("prime 1: %d\tprime 2: %d\r\n",m_period1, m_period2);
#endif
#ifdef OMAC_DEBUG_PRINTF
	OMAC_HAL_PRINTF("Estimated disco interval : %llu secs\r\n", (UINT64)m_period1*(UINT64)m_period2*(UINT64)g_OMAC.DISCO_SLOT_PERIOD_MICRO/1000000);
#endif
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_DISCOVERY, 0, g_OMAC.DISCO_SLOT_PERIOD_MICRO, TRUE, FALSE, PublicBeaconNCallback, OMACClockSpecifier); //1 sec Timer in micro seconds
	//rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_DISCOVERY_POST_EXEC, 0, 100000, TRUE, FALSE, PublicDiscoPostExec, OMACClockSpecifier); //1 sec Timer in micro seconds
	
	//rm = VirtTimer_SetTimer(VIRT_TIMER_OMAC_DISCOVERY_POST_EXEC, 0, DISCO_BEACON_TX_MAX_DURATION_MICRO, TRUE, FALSE, PublicDiscoPostExec, OMACClockSpecifier); //1 sec Timer in micro seconds
	//ASSERT_SP(rm == TimerSupported);
}

UINT64 DiscoveryHandler::GetSlotNumber(){
	UINT64 currentTicks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	UINT64 currentSlotNum = currentTicks / ((UINT64)g_OMAC.DISCO_SLOT_PERIOD_MICRO * TICKS_PER_MICRO);
	return currentSlotNum;
}

UINT64 DiscoveryHandler::GetTimeTillTheEndofSlot(){
	UINT64 cur_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	UINT64 ticks_till_end = ((UINT64)g_OMAC.DISCO_SLOT_PERIOD_MICRO * TICKS_PER_MICRO) - ( (cur_ticks + ((UINT64)g_OMAC.DISCO_SLOT_PERIOD_MICRO * TICKS_PER_MICRO)) % ((UINT64)g_OMAC.DISCO_SLOT_PERIOD_MICRO * TICKS_PER_MICRO));
	UINT32 ms_till_end = ((UINT32) ticks_till_end) / (TICKS_PER_MILLI / MILLISECINMICSEC ) ;
	return ms_till_end;
}

UINT64 DiscoveryHandler::NextEvent(){
	UINT64 currentSlotNum = GetSlotNumber();
	UINT16 nextEventsSlot = 0;
	UINT64 nextEventsMicroSec = 0;

	if(firstHighRateDiscoTimeinSlotNum == 0) {
		firstHighRateDiscoTimeinSlotNum = currentSlotNum;
		if(g_OMAC.HIGH_DISCO_PERIOD_IN_SLOTS){
			TempIncreaseDiscoRate();
		}
	}

	if(highdiscorate && ( (currentSlotNum - firstHighRateDiscoTimeinSlotNum) > g_OMAC.HIGH_DISCO_PERIOD_IN_SLOTS ) ) {
		PermanentlyDecreaseDiscoRate();
	}

	nextEventsSlot = NextEventinSlots(currentSlotNum);
	nextEventsMicroSec = nextEventsSlot * g_OMAC.DISCO_SLOT_PERIOD_MICRO;
	nextEventsMicroSec = nextEventsMicroSec + GetTimeTillTheEndofSlot();
	if(nextEventsMicroSec < OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO ) { //If we cannot react this fast, skip the next one and consider the upcoming event
		currentSlotNum = currentSlotNum +1;
		nextEventsSlot = NextEventinSlots(currentSlotNum);
		nextEventsSlot = nextEventsSlot + 1;
		nextEventsMicroSec = nextEventsSlot * g_OMAC.DISCO_SLOT_PERIOD_MICRO;
		nextEventsMicroSec = nextEventsMicroSec + GetTimeTillTheEndofSlot();
	}
	return(nextEventsMicroSec);
}

/*
 *
 */
UINT64 DiscoveryHandler::NextEventinSlots(const UINT64 &currentSlotNum){
	//UINT64 currentSlotNum = g_OMAC.m_omac_scheduler.GetSlotNumber();
	UINT64 period1Remaining, period2Remaining;
	period1Remaining = m_period1 - (currentSlotNum) % m_period1;
	period2Remaining = m_period2 - (currentSlotNum) % m_period2;

	if (period1Remaining == 0 || period2Remaining == 0) {
		return 0;
	}
	else  {
		if(period1Remaining < period2Remaining){
			return (period1Remaining - 1);
		}
		else{
			return (period2Remaining - 1);
		}
		//		return ((period1Remaining < period2Remaining) ? (2*period1Remaining ) : (2*period2Remaining ));
	}
}

/*
 *
 */
void DiscoveryHandler::ExecuteEvent(){
	beacon_received = false;
	m_num_sleep_retry_attempts = 0;
	VirtualTimerReturnMessage rm;
	m_state = DISCO_INITIAL;

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( OMAC_DISCO_EXEC_EVENT, TRUE );
	CPU_GPIO_SetPinState(  DISCO_NEXT_EVENT, TRUE );
#endif

	//rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY_POST_EXEC);
	
	DeviceStatus e = DS_Fail;
	

	e = g_OMAC.m_omac_RadioControl.StartRx();
	if (e == DS_Success){
		m_state = DISCO_LISTEN_SUCCESS;
		///rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, 20000, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, 1, TRUE, OMACClockSpecifier );
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( OMAC_DISCO_EXEC_EVENT, FALSE );
		CPU_GPIO_SetPinState(  DISCO_NEXT_EVENT, FALSE );
#endif

	
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
	}
	else {
		m_state = DISCO_LISTEN_FAIL;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, 1, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState( OMAC_DISCO_EXEC_EVENT, FALSE );
		CPU_GPIO_SetPinState(  DISCO_NEXT_EVENT, FALSE );
		CPU_GPIO_SetPinState(  DISCO_NEXT_EVENT, TRUE );
		CPU_GPIO_SetPinState(  DISCO_NEXT_EVENT, FALSE );
#endif
	}
}


/*
 *
 */
void DiscoveryHandler::PostExecuteEvent(){
	VirtualTimerReturnMessage rm;
	DeviceStatus  ds = DS_Success;
	m_state = WAITING_FOR_SLEEP;

	//rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY_POST_EXEC);
	ds = g_OMAC.m_omac_RadioControl.Stop();

	if (ds == DS_Success) {
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		m_state = SLEEP_SUCCESSFUL;
		g_OMAC.m_omac_scheduler.PostExecution();
	}
	else {
		OMAC_HAL_PRINTF(" \r\n OMACScheduler::PostPostExecution() Radio stop failure! m_num_sleep_retry_attempts = %u  \r\n", m_num_sleep_retry_attempts);
		if(m_num_sleep_retry_attempts < MAX_SLEEP_RETRY_ATTEMPTS){
			++m_num_sleep_retry_attempts;
			rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
			rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, RADIO_STOP_RETRY_PERIOD_IN_MICS, TRUE, OMACClockSpecifier );
			rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
			if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
				PostExecuteEvent();
			}
		}
		else{ // Radio does not stop proceed anyway

			g_OMAC.m_omac_scheduler.PostExecution();
		}
	}

}

bool DiscoveryHandler::FailsafeStop(){
	VirtualTimerReturnMessage rm;
	m_state = FAILSAFE_STOPPING;
	rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
	if(rm != TimerSupported){ //Could not stop the timer
		return false;
	}
	return true;
}

////////////////////Private Functions////////////////////////////
/*
 *
 */
//Mukundan: Can add conditions to suppress beaconing, will keep this true for now
BOOL DiscoveryHandler::ShouldBeacon(){
	return TRUE;
}

DeviceStatus DiscoveryHandler::CADDoneHandler(bool status){
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
	//OMAC_HAL_PRINTF("DiscoveryHandler::1 m_state = %d  \r\n", m_state);

	if (status) {
		switch(m_state) { 
			case CAD_FOR_BEACON1:
				m_state = BEACON1_SKIPPED;
				break;
			case CAD_FOR_BEACON2:
				m_state = BEACON2_SKIPPED;
				break;				
		}
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("DiscoveryHandler::CAD!\r\n");
		g_OMAC.m_omac_RadioControl.StartRx();
#endif
	}
	else {
#ifdef OMAC_DEBUG_PRINTF
		//OMAC_HAL_PRINTF("DiscoveryHandler::no channel activity detected! \r\n");
#endif		
	}	

	rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  1, TRUE, OMACClockSpecifier );
	rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
	if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
		PostExecuteEvent();
	}
	//OMAC_HAL_PRINTF("DiscoveryHandler::2 m_state = %d  \r\n", m_state);
	
}

void DiscoveryHandler::ExecuteCAD() {
	VirtualTimerReturnMessage rm;
	DeviceStatus DS;
	int tempCADDetectionTime = 1500;
	//OMAC_HAL_PRINTF("DiscoveryHandler::3 m_state = %d  \r\n", m_state);
	DS = CPU_Radio_ClearChannelAssesment(g_OMAC.radioName);
	switch(m_state) { 
		case DISCO_LISTEN_SUCCESS:
			m_state = CAD_FOR_BEACON1;
			break;
		case WAIT_AFTER_BEACON1:
			m_state = CAD_FOR_BEACON2;
			break;
	}	
	
	if(DS == DS_Success) {	
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);	
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  tempCADDetectionTime, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		if(rm != TimerSupported){
			PostExecuteEvent();
		}		
	}
	else {	
		switch(m_state) { 
			case CAD_FOR_BEACON1:
				m_state = BEACON1_SKIPPED;
				break;
			case CAD_FOR_BEACON2:
				m_state = BEACON2_SKIPPED;
				break;
		}
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, 1, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		if(rm != TimerSupported){
			PostExecuteEvent();
		}
	}
	//OMAC_HAL_PRINTF("DiscoveryHandler::4 m_state = %d  \r\n", m_state);	
}

/*
 *
 */
DeviceStatus DiscoveryHandler::Beacon(RadioAddress_t dst, Message_15_4_t* msgPtr){
#ifdef OMAC_DEBUG_GPIO
	OMAC_CPU_GPIO_SetPinState(  DISCO_SYNCSENDPIN, TRUE );
	OMAC_CPU_GPIO_SetPinState(  DISCO_SYNCSENDPIN, FALSE );
#endif
	DeviceStatus DS = DS_Fail;

	DiscoveryMsg_t* m_discoveryMsg = (DiscoveryMsg_t*)msgPtr->GetPayload();
	CreateMessage(m_discoveryMsg);

#ifdef OMAC_DEBUG_GPIO
	OMAC_CPU_GPIO_SetPinState(  DISCO_SYNCSENDPIN, TRUE );
	OMAC_CPU_GPIO_SetPinState(  DISCO_SYNCSENDPIN, FALSE );
#endif

	DS = Send(dst, msgPtr, sizeof(DiscoveryMsg_t), 0 );

	return DS;
}

void DiscoveryHandler::CreateMessage(DiscoveryMsg_t* discoveryMsg){
	discoveryMsg->nextSeed = g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_nextSeed;
	discoveryMsg->mask = g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_mask;
	UINT64 nextwakeupSlot = g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_nextwakeupSlot;
	discoveryMsg->nextwakeupSlot0 = (UINT32)nextwakeupSlot;
	discoveryMsg->nextwakeupSlot1 = (UINT32)(nextwakeupSlot>>32);
	discoveryMsg->seedUpdateIntervalinSlots = g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_seedUpdateIntervalinSlots;
	//discoveryMsg->nodeID = g_OMAC.GetMyAddress();

	//	UINT64 curticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	//	discoveryMsg->localTime0 = (UINT32) curticks;
	//	discoveryMsg->localTime1 = (UINT32) (curticks>>32);
	//
	//	discoveryMsg->lastwakeupSlotUpdateTimeinTicks0 = (UINT32) g_OMAC.m_omac_scheduler.m_DataReceptionHandler.lastwakeupSlotUpdateTimeinTicks;
	//	discoveryMsg->lastwakeupSlotUpdateTimeinTicks1 = (UINT32) (g_OMAC.m_omac_scheduler.m_DataReceptionHandler.lastwakeupSlotUpdateTimeinTicks>>32);
	//	discoveryMsg->msg_identifier = 33686018; // 0x02020202
}

/*
 *
 */
void DiscoveryHandler::BeaconAckHandler(Message_15_4_t* msg, UINT8 len, NetOpStatus status){
	VirtualTimerReturnMessage rm;
	OMAC_CPU_GPIO_SetPinState(OMAC_DISCO_BEACON_ACK_HANDLER_PIN, TRUE);
	OMAC_CPU_GPIO_SetPinState(OMAC_DISCO_BEACON_ACK_HANDLER_PIN, FALSE);
	OMAC_CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, FALSE);
	OMAC_CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, TRUE);

	switch(m_state){
	case BEACON1_SEND_START:
		m_state = BEACON1_SEND_DONE;
		
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  1, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
		//OMAC_HAL_PRINTF("Disco Message1 Sent\r\n");
		break;
	case BEACON2_SEND_START:
		m_state = BEACON2_SEND_DONE;

		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  1, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		//OMAC_HAL_PRINTF("Disco Message2 Sent\r\n");
		break;
	default:
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("DiscoveryHandler::Received Unexpected SendACK \r\n");
#endif
		break;
	}
	/* Don't use this for now
	if(status == NO_Busy){
		OMAC_HAL_PRINTF("NO_Busy - What do we do? Just ignore? \r\n");
		//m_busy = TRUE;
	}
	else if(status == NO_Success){
		//m_busy = FALSE;
	}
	else{
		OMAC_HAL_PRINTF("Need to investigate. Status: %d \r\n", status);
		ASSERT_SP(0);
	}

	if (msg != &m_discoveryMsgBuffer) {
		return;
	}

#ifndef DISABLE_SIGNAL
		//call SlotScheduler.printState();
		//signalBeaconDone(error, call GlobalTime.getLocalTime());
#endif
	 */
	/*if(m_disco_state == DISCO_STATE_BEACON_N){
		VirtualTimerReturnMessage rm;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, 0, TRUE, OMACClockSpecifier); //1 sec Timer in micro seconds
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
	}*/
}

void DiscoveryHandler::HandleRadioInterrupt(){
}
/*
 *
 */
void DiscoveryHandler::Beacon1(){
	VirtualTimerReturnMessage rm;
	DeviceStatus ds = DS_Fail;
	if (ShouldBeacon()) {		
		m_state = BEACON1_SEND_START;
		ds = Beacon(RADIO_BROADCAST_ADDRESS, &m_discoveryMsgBuffer);
	}
	if(ds == DS_Success) {
    	//rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, 44337/2, TRUE, OMACClockSpecifier );// g_OMAC.MAX_PACKET_TX_DURATION_MICRO/2, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.DISCO_PACKET_TX_TIME_MICRO, TRUE, OMACClockSpecifier );
		
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();			
		}
	}
	else {
		m_state = BEACON1_SKIPPED;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, g_OMAC.DISCO_PACKET_TX_TIME_MICRO, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
		//OMAC_HAL_PRINTF("Beacon1 failed. ds = %d; \r\n", ds);
	}
	//If Beacon 1 fails, just continue operation. There is one more beacon
}

/*
 *
 */
void DiscoveryHandler::BeaconN(){
	VirtualTimerReturnMessage rm;
	DeviceStatus ds = DS_Fail;
	if (ShouldBeacon()) {
		m_state = BEACON2_SEND_START;
		ds = Beacon(RADIO_BROADCAST_ADDRESS, &m_discoveryMsgBuffer);
	}
	if(ds == DS_Success) {
		///rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.MAX_PACKET_TX_DURATION_MICRO, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.DISCO_PACKET_TX_TIME_MICRO, TRUE, OMACClockSpecifier );
		//rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.DISCO_SLOT_PERIOD_MICRO/2, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
		
	}
	else {
		m_state = BEACON2_SKIPPED;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, g_OMAC.DISCO_PACKET_TX_TIME_MICRO, TRUE, OMACClockSpecifier );
		//rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.DISCO_SLOT_PERIOD_MICRO/2, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		if(rm != TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			PostExecuteEvent();
		}
	}
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DISCO_BEACON_N, FALSE );
#endif
	//VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
	//VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0, 0, TRUE, OMACClockSpecifier); //1 sec Timer in micro seconds
	//VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
	//this->PostExecuteEvent();

	//VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
	//VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY_POST_EXEC);
}


/*
 *
 */
void DiscoveryHandler::BeaconNTimerHandler(){
	VirtualTimerReturnMessage rm;


	OMAC_CPU_GPIO_SetPinState(OMAC_DISCO_BEACONNTIMERHANDLER_PIN, TRUE);
	OMAC_CPU_GPIO_SetPinState(OMAC_DISCO_BEACONNTIMERHANDLER_PIN, FALSE);

	OMAC_CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, FALSE);
	OMAC_CPU_GPIO_SetPinState(SCHED_DISCO_EXEC_PIN, TRUE);

	switch(m_state){
	case FAILSAFE_STOPPING:
		break;
	case DISCO_LISTEN_SUCCESS:
		ExecuteCAD();
		break;
	case CAD_FOR_BEACON1:
		Beacon1();
		break;	
	case BEACON1_SEND_START:
		//hal_printf("DiscoveryHandler::Beacon1 transmission send ACK is missing \r\n");
#ifdef OMAC_DEBUG_PRINTF
	//	OMAC_HAL_PRINTF("DiscoveryHandler::Beacon1 transmission send ACK is missing \r\n");
#endif
		if(!g_OMAC.isSendDone){
			//rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.MAX_PACKET_TX_DURATION_MICRO/2, TRUE, OMACClockSpecifier );
			rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
			rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.DISCO_PACKET_TX_TIME_MICRO, TRUE, OMACClockSpecifier );
		
			rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
			if(rm == TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
				break;
			}
		}
	case BEACON1_SKIPPED:
	case BEACON1_SEND_DONE:
		// BK: This was  a hack to wait additional time before sending second beacon. Commening it out.
		m_state = WAIT_AFTER_BEACON1;
		rm = VirtTimer_Stop(VIRT_TIMER_OMAC_DISCOVERY);
		rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.MAX_PACKET_TX_DURATION_MICRO, TRUE, OMACClockSpecifier );
		rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		if(rm == TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
			break;
		}
		break;
	case WAIT_AFTER_BEACON1:
		ExecuteCAD();
		break;
	case CAD_FOR_BEACON2:
		// BK: Second beacon is not very useful without CCA, commenting it out.
		BeaconN();
		break;		
	case BEACON2_SEND_START:
		/* BK: Second beacon is not very useful without CCA, commenting it out.
		hal_printf("DiscoveryHandler::Beacon2 transmission send ACK is missing \r\n");
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("DiscoveryHandler::Beacon2 transmission send ACK is missing \r\n");
#endif
		if(!g_OMAC.isSendDone){
			rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.MAX_PACKET_TX_DURATION_MICRO, TRUE, OMACClockSpecifier );
			rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
			if(rm == TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
				break;
			}
		}
		*/
	case BEACON2_SKIPPED:
	case BEACON2_SEND_DONE:
		/* BK: This was  a hack to wait additional time before sending second beacon. Commening it out.
		//m_state = WAIT_AFTER_BEACON2;
		//rm = VirtTimer_Change(VIRT_TIMER_OMAC_DISCOVERY, 0,  g_OMAC.MAX_PACKET_TX_DURATION_MICRO, TRUE, OMACClockSpecifier );
		//rm = VirtTimer_Start(VIRT_TIMER_OMAC_DISCOVERY);
		//if(rm == TimerSupported){ //Could not start the timer to turn the radio off. Turn-off immediately
		//	break;
		//}
		//break;/
		*/
	case WAIT_AFTER_BEACON2:
		PostExecuteEvent();
		break;
	case DISCO_INITIAL:
	case SLEEP_SUCCESSFUL:
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("DiscoveryHandler::Unexpected firing of VIRT_TIMER_OMAC_DISCOVERY m_state = %d  \r\n", m_state);
#endif
		break;
	case DISCO_LISTEN_FAIL:
	case WAITING_FOR_SLEEP:
	default:
		PostExecuteEvent();
	}
}

/*
 *
 */
DeviceStatus DiscoveryHandler::Receive(RadioAddress_t source, DiscoveryMsg_t* discoMsg, MsgLinkQualityMetrics_t* msgLinkQualityMetrics){  //(Message_15_4_t* msg, void* payload, UINT8 len){
	beacon_received = true;
	Neighbor_t tempNeighbor;
	Neighbor_t* neigh_ptr;
	UINT8 nbrIdx = 0;
	UINT64 localTime = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	NeighborTableCommonParameters_One_t neighborTableCommonParameters_One_t;
	NeighborTableCommonParameters_Two_t neighborTableCommonParameters_two_t;

	//	if(discoMsg -> msg_identifier != 33686018){
	//		localTime = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	//		//ASSERT_SP(0);
	//	}
#ifdef def_Neighbor2beFollowed
	if (source == g_OMAC.Neighbor2beFollowed){
#ifdef OMAC_DEBUG_GPIO
		OMAC_CPU_GPIO_SetPinState(  DISCO_SYNCRECEIVEPIN, TRUE );
#endif
	}
#endif

#ifdef OMAC_DEBUG_GPIO
	OMAC_CPU_GPIO_SetPinState(DISCO_SYNCRECEIVEPIN, TRUE );
#endif
	//DiscoveryMsg_t* discoMsg = (DiscoveryMsg_t *) msg->GetPayload();
	//RadioAddress_t source = msg->GetHeader()->src;

	UINT64 nextwakeupSlot = (((UINT64)discoMsg->nextwakeupSlot1) <<32) + discoMsg->nextwakeupSlot0;
#ifdef OMAC_DEBUG_PRINTF
		//OMAC_HAL_PRINTF("DiscoveryHandler::Receive DISCO nextwakeupSlot = %s\r\n", l2s(nextwakeupSlot, 0));
#endif

	neighborTableCommonParameters_One_t.MACAddress = source;
	neighborTableCommonParameters_One_t.status = Alive;
	neighborTableCommonParameters_One_t.lastHeardTime = localTime;
	neighborTableCommonParameters_One_t.linkQualityMetrics.AvgRSSI = msgLinkQualityMetrics->RSSI;
	//neighborTableCommonParameters_One_t.linkQualityMetrics.LinkQuality = msgLinkQualityMetrics->LinkQuality;
	neighborTableCommonParameters_two_t.nextSeed = discoMsg->nextSeed;
	neighborTableCommonParameters_two_t.mask = discoMsg->mask;
	neighborTableCommonParameters_two_t.nextwakeupSlot = nextwakeupSlot;
	neighborTableCommonParameters_two_t.seedUpdateIntervalinSlots = discoMsg->seedUpdateIntervalinSlots;
	g_NeighborTable.UpdateNeighbor(&neighborTableCommonParameters_One_t, &neighborTableCommonParameters_two_t);



#if OMAC_DEBUG_PRINTF_DISCO_RX
	hal_printf("Disco RX SourceID = %u TimeAvail = %s \r\n"
			,source
			,g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.IsNeighborTimeAvailable(source) ? "TRUE" : "FALSE"
	);
	//	g_OMAC.PrintNeighborTable();
	g_OMAC.is_print_neigh_table = true;
#endif


#ifdef def_Neighbor2beFollowed
	if (source == g_OMAC.Neighbor2beFollowed){
#ifdef OMAC_DEBUG_GPIO
		OMAC_CPU_GPIO_SetPinState(  DISCO_SYNCRECEIVEPIN, FALSE );
#endif
	}
#endif
#ifdef OMAC_DEBUG_GPIO
	OMAC_CPU_GPIO_SetPinState(DISCO_SYNCRECEIVEPIN, FALSE );
#endif

	return DS_Success;
}

DeviceStatus DiscoveryHandler::Send(RadioAddress_t address, Message_15_4_t* msg, UINT16 size, UINT64 event_time){
	DeviceStatus retValue = DS_Fail;
	static UINT8 seqNumber = 0;
	UINT8 finalSeqNumber = 0;
	IEEE802_15_4_Header_t * header = msg->GetHeader();
	/****** Taking the word value of below bits gives FCF_WORD_VALUE *******/
	/*header->fcf->IEEE802_15_4_Header_FCF_BitValue.frameType = FRAME_TYPE_MAC;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.securityEnabled = 0;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.framePending = 0;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.ackRequired = 1;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.intraPAN = 1;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.reserved = 0;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.destAddrMode = 2;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.frameVersion = 1;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.srcAddrMode = 2;*/
	/**************************************************************/
	header->fcf.fcfWordValue = FCF_WORD_VALUE_DISCO;
	//header->fcf = (65 << 8);
	//header->fcf |= 136;
	finalSeqNumber = g_OMAC.GetMyAddress() ^ 0xAA;
	finalSeqNumber += ((g_OMAC.GetMyAddress() >> 8) ^ 0x55);
	finalSeqNumber += seqNumber;
	//header->dsn = finalSeqNumber;
	//header->dsn = OMAC_DISCO_SEQ_NUMBER;
	//header->srcpan = SRC_PAN_ID;
	//header->destpan = DEST_PAN_ID;
	/*if(g_OMAC.GetMyAddress() == 6846){
		header->dest = 0x0DB1;
	}
	else{
		header->dest = 0x1ABE;
	}*/
	//6846(0x1ABE) is the receiver
	/*if(g_OMAC.GetMyAddress() != 0x1ABE){
		header->dest = 0x1ABE;
	}
	else{
		header->dest = 0x1111;
	}*/
	header->dest = address;
	header->src = g_OMAC.GetMyAddress();
	//header->retryAttempt = 0;
	seqNumber++;

	IEEE802_15_4_Metadata* metadata = msg->GetMetaData();
	//UINT8 * payload = msg->GetPayload();
	header->length = (size + sizeof(IEEE802_15_4_Header_t));
	//header->SetLength(IEEE802_15_4_MAX_PAYLOAD + sizeof(IEEE802_15_4_Header_t) + sizeof(IEEE802_15_4_Footer_t) + sizeof(IEEE802_15_4_Metadata));
	header->macName = (g_OMAC.macName);
	header->payloadType = (MFM_OMAC_DISCOVERY);
	header->flags = (0); //Initialize flags to zero

	msg->GetMetaData()->SetReceiveTimeStamp((INT64)event_time);

#ifdef OMAC_DEBUG_GPIO
	OMAC_CPU_GPIO_SetPinState(DISCO_SYNCSENDPIN, TRUE);
#endif

	if(beacon_received) retValue = DS_Fail;
	else retValue = g_OMAC.m_omac_RadioControl.Send(address, msg, header->length);

#ifdef OMAC_DEBUG_GPIO
	OMAC_CPU_GPIO_SetPinState(DISCO_SYNCSENDPIN, FALSE );
#endif



	return retValue;
}

void DiscoveryHandler::TempIncreaseDiscoRate(){
	m_period1 = CONTROL_P1[g_OMAC.GetMyAddress() % DISCOVERY_SIZE_OF_PRIME_NUMBER_POOL] ;
	m_period2 = CONTROL_P2[g_OMAC.GetMyAddress() % DISCOVERY_SIZE_OF_PRIME_NUMBER_POOL] ;
	highdiscorate = true;
	firstHighRateDiscoTimeinSlotNum = GetSlotNumber();
	g_OMAC.m_omac_RadioControl.stayOn = HIGH_DISCO_PERIOD_ALWAYS_ON;
#if OMAC_DEBUG_PRINTF_HIGH_DISCO_MODE
	hal_printf("DiscoveryHandler::switching to fast disco mode \r\n");
	hal_printf("prime 1: %d\tprime 2: %d g_OMAC.m_omac_RadioControl.stayOn : %d \r\n",m_period1, m_period2, g_OMAC.m_omac_RadioControl.stayOn);
	hal_printf("Estimated disco interval : %llu secs\r\n", (UINT64)m_period1*(UINT64)m_period2*(UINT64)g_OMAC.DISCO_SLOT_PERIOD_MICRO/1000000);
#endif

}

void DiscoveryHandler::PermanentlyDecreaseDiscoRate(){
	m_period1 = CONTROL_P3[g_OMAC.GetMyAddress() % DISCOVERY_SIZE_OF_PRIME_NUMBER_POOL] ;
	m_period2 = CONTROL_P4[g_OMAC.GetMyAddress() % DISCOVERY_SIZE_OF_PRIME_NUMBER_POOL] ;
	g_OMAC.m_omac_RadioControl.stayOn = false;
#if OMAC_DEBUG_PRINTF_HIGH_DISCO_MODE
	hal_printf("DiscoveryHandler::switching to slow disco mode \r\n");
	hal_printf("prime 1: %d\tprime 2: %d\r\n",m_period1, m_period2);
	hal_printf("Estimated disco interval : %llu secs\r\n", (UINT64)m_period1*(UINT64)m_period2*(UINT64)g_OMAC.DISCO_SLOT_PERIOD_MICRO/1000000);
#endif
	highdiscorate = false;
}
