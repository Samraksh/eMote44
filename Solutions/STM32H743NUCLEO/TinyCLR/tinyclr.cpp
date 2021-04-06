////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <tinyclr_application.h>
#include <tinyhal.h>
#include <Samraksh/VirtualTimer.h>
#include <Samraksh/serial_frame_pal.h>
#include "LoRa_test.h"

#define PKT_PERIOD_MICRO 100000 // 2 secs
#define PKT_FOR_CHANNEL 100 // 100 packets per channel
//#define PKT_PERIOD_MICRO 10000000 // 10 secs
#define TESTRADIONAME SX1276RADIO
//#define TESTRADIONAME RF231RADIO

#define GPIO_0 _P(B,12)
#define GPIO_1 _P(B,13)
#define GPIO_2 _P(C,12)

#define BASE 100
#define FENCE 101


// Debug functions for timing
static void reset_cnt()
{
    CoreDebug->DEMCR |= 0x01000000;
    DWT->CYCCNT = 0; // reset the counter
    DWT->CTRL = 0;
}

static void start_cnt()
{
    DWT->CTRL |= 0x00000001 ; // enable the counter
}

static void stop_cnt()
{
    DWT->CTRL &= 0xFFFFFFFE ; // disable the counter
}

static unsigned getCycles()
{
    return DWT->CYCCNT ;
}
// End Debug functions for timing



UINT16 testDest=0;

RadioEvents_t native_events;

UINT16 nodeID;
UINT8 nodeType = FENCE;
UINT8 stateType = 0;
//UINT8 countForSendingPkg = 1;
UINT8 countForStateZero = 0;
UINT8 countForStateTwo = 0;
UINT8 countForStateThree = 0;
UINT8 checkLossInfoPacket = 0;
// Fence array
int16_t noiseSignalforFence[100];
int16_t SNRforFence[100];
int16_t RSSIforFence[100];
UINT8 allInfoforFence[25];
UINT8 oldAllInfoforFence[25];

UINT8 DataChannel = 2;
UINT8 BeaconChannel = 2;

UINT8 ListenChannel = 2;
// This somehow gets put in the radio function. Out of scope for now, but fix me later.
static void GetCPUSerial(uint8_t * ptr, unsigned num_of_bytes ){
	uint32_t Device_Serial0;
	uint32_t Device_Serial1;
	uint32_t Device_Serial2;
	Device_Serial0 = (*(uint32_t*)0x1FF1E800);// *(uint32_t*)(0x1FF0F420);
	Device_Serial1 = (*(uint32_t*)0x1FF1E804);// *(uint32_t*)(0x1FF0F424);
	Device_Serial2 = (*(uint32_t*)0x1FF1E808);// *(uint32_t*)(0x1FF0F428);

	if(num_of_bytes==12){
	    ptr[0] = (uint8_t)(Device_Serial0 & 0x000000FF);
	    ptr[1] = (uint8_t)((Device_Serial0 & 0x0000FF00) >> 8);
	    ptr[2] = (uint8_t)((Device_Serial0 & 0x00FF0000) >> 16);
	    ptr[3] = (uint8_t)((Device_Serial0 & 0xFF000000) >> 24);

	    ptr[4] = (uint8_t)(Device_Serial1 & 0x000000FF);
	    ptr[5] = (uint8_t)((Device_Serial1 & 0x0000FF00) >> 8);
	    ptr[6] = (uint8_t)((Device_Serial1 & 0x00FF0000) >> 16);
	    ptr[7] = (uint8_t)((Device_Serial1 & 0xFF000000) >> 24);

	    ptr[8] = (uint8_t)(Device_Serial2 & 0x000000FF);
	    ptr[9] = (uint8_t)((Device_Serial2 & 0x0000FF00) >> 8);
	    ptr[10] = (uint8_t)((Device_Serial2 & 0x00FF0000) >> 16);
	    ptr[11] = (uint8_t)((Device_Serial2 & 0xFF000000) >> 24);
	}
}

void PrintHex(char *x, int size){
	for (int j=0;j<size; j++){
		hal_printf("0x%.2X , ",x[j]);
	}
	hal_printf("\r\n");
}

void PeriodTimerHandler(void * arg){
	//hal_printf("Send Timer\r\n");
	ChangeState();
}

void radio_tx_done(void) {
	//debug_printf("%s\r\n", __func__);
	//SX1276SetRx(0);
}

void rx_timeout(void) {
	debug_printf("%s\r\n", __func__);
}

void tx_timeout(void) {
	debug_printf("%s\r\n", __func__);
}

void rx_error(void) {
	//debug_printf("%s\r\n", __func__);
	hal_printf("CRC ");
}

void radio_change_channel( uint8_t currentChannel ) {
//	debug_printf("%s\r\n", __func__);
}

void radio_cad_done(bool channelActivityDetected) {
//	debug_printf("%s\r\n", __func__);
}

void valid_header_detected() {
//	debug_printf("%s\r\n", __func__);
}

void rx_done(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {

	UINT8 index;
	VirtualTimerReturnMessage rm;

	//debug_printf("Packet Size:%d\r\n", size);
	if (size == 4 && stateType == 0) { // channel_info_pkt_struct
		channel_info_pkt_t *ci_pkt=(channel_info_pkt_t *) payload;
		if (ci_pkt->packet_id == 11) {
			debug_printf("Channel Info(%d) %d %d\r\n", ci_pkt->packet_id, ci_pkt->next_channel, ci_pkt->node_id);
			ListenChannel = ci_pkt->next_channel;
			stateType = 1;

			rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
			rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 1, TRUE, LOW_DRIFT_TIMER );
			rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
		}
	}
	else if (size == 28 && stateType == 2) { // data_pkt_struct
		data_pkt_t *dp_pkt=(data_pkt_t *) payload;
		if (dp_pkt->packet_id == 44) {
			noiseSignalforFence[dp_pkt->count] = SX1276ReadRssi(MODEM_LORA);
			SNRforFence[dp_pkt->count] = snr;
			RSSIforFence[dp_pkt->count] = rssi;

			//debug_printf("Channel Info(%d) %d %d\r\n", ci_pkt->packet_id, ci_pkt->next_channel, ci_pkt->node_id);

			if (dp_pkt->count >= 0 && dp_pkt->count < 101) {
				rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
				rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 100000*(101-dp_pkt->count), TRUE, LOW_DRIFT_TIMER );
				rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
			}
		}
	}
	else if (stateType == 0) {
		info_pkt_t *ip_pkt=(info_pkt_t *) payload;
		if (ip_pkt->packet_id == 22) {
			debug_printf("Node ID : %d size :%d\r\n", ip_pkt->node_id, size);

			for (int i = 0; i < 25; i=i+5) {
				int freq = 902000000 + (ip_pkt->info[i] * 500000);
				debug_printf("Frequency: %d, Noise: %d, SNR: %d, RSSI: %d, Count: %d\r\n", freq, ip_pkt->info[i+1]-200, ip_pkt->info[i+2]-200, ip_pkt->info[i+3]-200, ip_pkt->info[i+4]);
			}

			ack_pkt_t ap_pkt_send;
			ap_pkt_send.packet_id = 33;
			ap_pkt_send.node_id = ip_pkt->node_id;

			SX1276Send( (uint8_t *)&ap_pkt_send, sizeof(ap_pkt_send), 0 );
		}
	}
	else if (stateType == 3) {
		ack_pkt_t *ap_pkt = (ack_pkt_t *) payload;
		if (ap_pkt->packet_id == 33) {
			if (ap_pkt->node_id == nodeID) {
				debug_printf("ACK Node ID : %d size :%d\r\n", ap_pkt->node_id, size);
				if(checkLossInfoPacket != 2) countForStateThree = 5;
				checkLossInfoPacket = 0;

				rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
				rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 1, TRUE, LOW_DRIFT_TIMER );
				rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
			}
		}
	}
}

void native_link_test()
{
	CPU_SPI_Init(SPI_TYPE_RADIO);

	native_events.TxDone 				= radio_tx_done;
	native_events.TxTimeout 			= tx_timeout;
	native_events.RxDone 				= rx_done;
	native_events.RxTimeout 			= rx_timeout;
	native_events.RxError 				= rx_error;
	native_events.FhssChangeChannel 	= radio_change_channel;
	native_events.CadDone 				= radio_cad_done;
	native_events.ValidHeaderDetected	= valid_header_detected;

	{
		//Get cpu serial and hash it to use as node id. THIS IS NOT A DRIVER FUNCTION and NOT A MAC FUNCTION. CREATE A NAMING SERVICE
		UINT8 cpuserial[12];
		memset(cpuserial, 0, 12);
		GetCPUSerial(cpuserial, 12);
		UINT16 tempNum=0;
		UINT16 * temp = (UINT16 *) cpuserial;
		for (int i=0; i< 6; i++){
			tempNum=tempNum ^ temp[i]; //XOR 72-bit number to generate 16-bit hash
		}
		nodeID = tempNum;
	}

	SX1276Init(&native_events);

	SX1276SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    SX1276SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

	sx1276_interop_change_channel(BeaconChannel);
	//SX1276SetChannel(RF_FREQUENCY);
	SX1276SetSleep();
	if(nodeType == FENCE) SX1276SetRx(0);

	VirtTimer_Initialize();

	//This is important.
	//currentMacName == CSMAMAC;
	for (UINT8 i = 0; i < 100; i++) {
		noiseSignalforFence[i] = 999;
		SNRforFence[i] = 255;
		RSSIforFence[i] = 999;
	}

	hal_printf("Initialize LinkTest\r\n");

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1 , 0, 1000000, TRUE, FALSE, PeriodTimerHandler, LOW_DRIFT_TIMER); //1 sec Timer in micro seconds
	rm = VirtTimer_Start(LocalClockMonitor_TIMER1);

	//rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1, 0, PKT_PERIOD_MICRO, FALSE, FALSE, SendTimerHandler, DEFAULT_TIMER);

	//rm = VirtTimer_SetTimer(VIRT_CONT_TEST_TIMER2, 0, PKT_PERIOD_MICRO * (PKT_FOR_CHANNEL+10), FALSE, FALSE, TotalCountTimerHandler, DEFAULT_TIMER);
	ASSERT_SP(rm == TimerSupported);
};


void ChangeState(){
	VirtualTimerReturnMessage rm;
	if (nodeType == BASE) {
		if (stateType == 0) {
			if (countForStateZero == 0) {
				if (DataChannel%5 == 2) {
					BeaconChannel = DataChannel;
				}
				else {
					if (BeaconChannel+10 > 50) BeaconChannel = BeaconChannel - 40;
					else BeaconChannel = BeaconChannel + 10;
					sx1276_interop_change_channel(BeaconChannel);
				}
				hal_printf("Beacon channel: %d\r\n", BeaconChannel);
			}

			channel_info_pkt_t ci_pkt;
			ci_pkt.packet_id = 11;
			ci_pkt.next_channel = DataChannel;
			ci_pkt.node_id = nodeID;

			SX1276Send( (uint8_t *)&ci_pkt, sizeof(ci_pkt), 0);
			countForStateZero++;
			if (countForStateZero >= 5) {
				countForStateZero = 0;
				stateType = 2;
			}
			rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
			rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 100000, TRUE, LOW_DRIFT_TIMER );
			rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
		}
		else if (stateType == 1) {
			SX1276SetSleep();
			BeaconChannel = DataChannel;
			sx1276_interop_change_channel(BeaconChannel);
			hal_printf("Listening channel: %d\r\n", BeaconChannel);
			SX1276SetRx(0);
			stateType = 0;
			//countForSendingPkg++;

			rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
			rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 10500000, TRUE, LOW_DRIFT_TIMER );
			rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
		}
		else if (stateType == 2) {
			if (countForStateTwo == 0) {
				//if (countForSendingPkg%6 != 1 && countForSendingPkg%6 != 2) sx1276_interop_change_channel(DataChannel);
				if (DataChannel%5 != 2) sx1276_interop_change_channel(DataChannel);
				hal_printf("Sending channel: %d\r\n", DataChannel);
			}

			data_pkt_t dp_pkt;
			dp_pkt.packet_id = 44;
			dp_pkt.count = countForStateTwo;
			dp_pkt.node_id = nodeID;
			strncpy(dp_pkt.name, "ABCDEFGHIJKLMN Samraksh", sizeof(dp_pkt.name));

			SX1276Send( (uint8_t *)&dp_pkt, sizeof(dp_pkt), 0 );
			countForStateTwo++;
			if (countForStateTwo >= 100) {
				countForStateTwo = 0;
				//countForSendingPkg++;
				DataChannel++;
				if (DataChannel >= 52) DataChannel = 2;
				//if (countForSendingPkg >= 61) countForSendingPkg = 1;
				//if (countForSendingPkg%6 == 2) stateType = 1;
				if (DataChannel%5 == 2) stateType = 1;
				else stateType = 0;
			}

			rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
			rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 100000, TRUE, LOW_DRIFT_TIMER );
			rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
		}
	}
	else if (nodeType == FENCE) {
		if (stateType == 0) {
			if (countForStateZero == 0) {
				SX1276SetSleep();
				hal_printf("Listening: %d\r\n", ListenChannel);
				sx1276_interop_change_channel(ListenChannel);
			}

			SX1276SetRx(0);
			stateType = 0;
			countForStateZero++;

			if (countForStateZero >= 6) {
				countForStateZero = 0;
				if (ListenChannel > 25 && ListenChannel < 47) ListenChannel = ListenChannel - 20;
				else if (ListenChannel <= 25) ListenChannel = ListenChannel + 25;
				else if (ListenChannel >= 47) ListenChannel = 2;
			}

			rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
			rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 10500000, TRUE, LOW_DRIFT_TIMER );
			rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
		}
		else if (stateType == 1) {
			SX1276SetSleep();
			hal_printf("Receiving Data Packet : %d\r\n", ListenChannel);
			sx1276_interop_change_channel(ListenChannel);
			SX1276SetRx(0);
			stateType = 2;

			rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
			rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 10500000, TRUE, LOW_DRIFT_TIMER );
			rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
		}
		else if (stateType == 2) {
			UINT8 countTotal = 0;
			int16_t NoiseSignalKey = 0, NoiseSignalJ = 0;
			int16_t SNRKey = 0, SNRJ = 0;
			int16_t RSSIKey = 0, RSSIJ = 0;

			for (UINT8 i = 1; i < 100; i++) {
				NoiseSignalKey = noiseSignalforFence[i];
				NoiseSignalJ = i - 1;

				SNRKey = SNRforFence[i];
				SNRJ = i - 1;

				RSSIKey = RSSIforFence[i];
				RSSIJ = i - 1;

				while (NoiseSignalJ >= 0 && noiseSignalforFence[NoiseSignalJ] > NoiseSignalKey)
				{
					noiseSignalforFence[NoiseSignalJ + 1] = noiseSignalforFence[NoiseSignalJ];
					NoiseSignalJ = NoiseSignalJ - 1;
				}
				noiseSignalforFence[NoiseSignalJ + 1] = NoiseSignalKey;

				while (SNRJ >= 0 && SNRforFence[SNRJ] > SNRKey)
				{
					SNRforFence[SNRJ + 1] = SNRforFence[SNRJ];
					SNRJ = SNRJ - 1;
				}
				SNRforFence[SNRJ + 1] = SNRKey;

				while (RSSIJ >= 0 && RSSIforFence[RSSIJ] > RSSIKey)
				{
					RSSIforFence[RSSIJ + 1] = RSSIforFence[RSSIJ];
					RSSIJ = RSSIJ - 1;
				}
				RSSIforFence[RSSIJ + 1] = RSSIKey;
			}

			int ninetyFivePercentileForRSSI = round(countTotal * 0.95);
			int ninetyFivePercentileForSNR = round(countTotal * 0.05);

			//hal_printf("NS:%d, SNR:%d, TR:%d, Count:%d\r\n", noiseSignalforFence[ninetyFivePercentileForSNR], SNRforFence[ninetyFivePercentileForSNR], RSSIforFence[ninetyFivePercentileForRSSI], countTotal);

			int arrayCalculation = ((ListenChannel % 5))*5;

			allInfoforFence[arrayCalculation] = ListenChannel;
			allInfoforFence[arrayCalculation+1] = 200 + noiseSignalforFence[ninetyFivePercentileForRSSI];
			allInfoforFence[arrayCalculation+2] = 200 + SNRforFence[ninetyFivePercentileForSNR];
			allInfoforFence[arrayCalculation+3] = 200 + RSSIforFence[ninetyFivePercentileForSNR] ;

			for (UINT8 i = 0; i < 100; i++) {
				if (noiseSignalforFence[i] != 999) countTotal++;
				noiseSignalforFence[i] = 999;
				SNRforFence[i] = 999;
				RSSIforFence[i] = 999;
			}
			allInfoforFence[arrayCalculation+4] = countTotal;

			hal_printf("NS:%d, SNR:%d, TR:%d, Count:%d\r\n", allInfoforFence[arrayCalculation+1]-200, allInfoforFence[arrayCalculation+2]-200, allInfoforFence[arrayCalculation+3]-200, allInfoforFence[arrayCalculation+4]);

			ListenChannel = ListenChannel + 1;
			if (ListenChannel >= 52) ListenChannel = 2;
			if (ListenChannel % 5 == 2) stateType = 3;
			else stateType = 1;

			rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
			rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 1, TRUE, LOW_DRIFT_TIMER );
			rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
		}
		else if (stateType == 3) {
			if (countForStateThree == 0) {
				SX1276SetSleep();
				hal_printf("Sending channel: %d\r\n", ListenChannel);
				sx1276_interop_change_channel(ListenChannel);
			}

			if (countForStateThree < 3) {
				info_pkt_t ip_pkt;
				ip_pkt.packet_id = 22;
				ip_pkt.node_id = nodeID;

				if (checkLossInfoPacket == 1) {
					for (UINT8 i = 0; i < 25; i++) {
						ip_pkt.info[i] = oldAllInfoforFence[i];
					}
					checkLossInfoPacket = 2;
				}
				else if(countForStateThree < 3) {
					for (UINT8 i = 0; i < 25; i++) {
						ip_pkt.info[i] = allInfoforFence [i];
					}
				}
				countForStateThree++;
				SX1276Send( (uint8_t *)&ip_pkt, sizeof(ip_pkt), 0 );
			}

			SX1276SetSleep();
			SX1276SetRx(0);

			if (countForStateThree >= 3) {
				rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
				rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 10000000 - (1000000*countForStateThree), TRUE, LOW_DRIFT_TIMER );
				rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
				for (UINT8 i = 0; i < 25; i++) {
					oldAllInfoforFence[i] = allInfoforFence [i];
					allInfoforFence[i] = 0;
				}
				if (countForStateThree == 5) checkLossInfoPacket = 0;
				else checkLossInfoPacket = 1;

				stateType = 1;
				countForStateThree = 0;
			}
			else {
				rm = VirtTimer_Stop(LocalClockMonitor_TIMER1);
				rm = VirtTimer_Change(LocalClockMonitor_TIMER1, 0, 1000000, TRUE, LOW_DRIFT_TIMER );
				rm = VirtTimer_Start(LocalClockMonitor_TIMER1);
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

////////////////////////////////////////////////////////////////////////////////
void ApplicationEntryPoint()
{
    CLR_SETTINGS clrSettings;
#ifdef MEL_USE_SERIAL_FRAMES
	framed_serial_init();
#endif

	// Initial delay to allow UART terminals to start and catch startup messages
    //HAL_Delay(5000);

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
	I2S_Internal_Initialize();
	//I2S_Test();
    //hal_printf(" CLR 30 ");
	native_link_test();


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
