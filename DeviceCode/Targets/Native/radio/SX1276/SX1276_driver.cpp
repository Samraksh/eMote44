#include "SX1276.h"
#include "SX1276_driver.h"
#include <Samraksh/VirtualTimer.h>
#include <Samraksh/Message.h>

#define UNSET_TS 0xFFFFFFFFFFFFFFFF
#define LOW_PRECISION_CLOCK_ID 4
#define HIGH_PRECISION_CLOCK_ID 1

RadioEventHandler Radio_event_handler;
RadioEvents_t events;
InternalRadioProperties_t internal_radio_properties;
RadioMode_t m_rm;
msgToBeTransmitted_t m_packet;
bool DataStatusCallback_success;
UINT16 DataStatusCallback_number_of_bytes_in_buffer;
bool Is_CAD_Running;
bool CAD_Status;
UINT64 received_ts_ticks;
UINT16 radio_address;
INT8 radioName;
UINT16 preloadedMsgSize;

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
	
void SX1276_HAL_ValidHeaderDetected(){	
	void* dummy_ptr = NULL;
	received_ts_ticks = HAL_Time_CurrentTicks();
	Radio_event_handler.RadioInterruptHandler(StartOfReception, dummy_ptr);
}

void SX1276_HAL_TxDone(){	
	NetOpStatus ns;
	UINT8 radioAckStatus;

	ns = NetworkOperations_Success;
	radioAckStatus = NetworkOperations_Success;
	
	SX1276_HAL_TurnOnRx();

	(Radio_event_handler.GetSendAckHandler())(static_cast<void*>(SX1276_Packet_GetPayload()), SX1276_Packet_GetSize(), ns, radioAckStatus);

	SX1276_Packet_ClearPayload();
}

void SX1276_HAL_TxTimeout(){	
	NetOpStatus ns;
	UINT8 radioAckStatus;
	
	SX1276_Packet_ClearPayload();
	SX1276_HAL_ChooseRadioConfig();

	ns = NetworkOperations_Fail;
	radioAckStatus = NetworkOperations_Success;
	SX1276_HAL_TurnOnRx();
	(Radio_event_handler.GetSendAckHandler())(static_cast<void*>(SX1276_Packet_GetPayload()), SX1276_Packet_GetSize(), ns, radioAckStatus);

}

void SX1276_HAL_RxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr){	
	if(payload == NULL) {
		hal_printf("RX timeout with no payload\r\n");
		SX1276_HAL_TurnOnRx();
		  // Reset the radio
		return;
	}
	//hal_printf("RX Done \r\n");
	Message_15_4_t* pckt_ptr = reinterpret_cast<Message_15_4_t*>(payload);
	if(received_ts_ticks == UNSET_TS)
		received_ts_ticks = HAL_Time_CurrentTicks();
	pckt_ptr->GetMetaData()->SetReceiveTimeStamp(received_ts_ticks);
	(Radio_event_handler.GetReceiveHandler())(payload, size);
}

void SX1276_HAL_RxTimeout(){
	hal_printf("RX timeout\r\n");
/*	SX1276Reset();
	//g_SX1276M1BxASWrapper.IoInit( );
	RxChainCalibration();
	SX1276SetOpMode( RF_OPMODE_SLEEP );
	SX1276BoardIoIrqInit();
	
	SX1276SetRadioRegistersInit();
	
	SX1276SetModem(MODEM_LORA);
	SX1276_HAL_ChooseRadioConfig();*/
	SX1276_HAL_TurnOnRx();
}

void SX1276_HAL_RxError(){	
	hal_printf("RX CRC Error\r\n");
	SX1276_HAL_TurnOnRx();
}

void SX1276_HAL_FhssChangeChannel(uint8_t currentChannel ){

}
void SX1276_HAL_CadDone(bool channelActivityDetected){	
/*	m_rm = SLEEP;
	CAD_Status = channelActivityDetected;
	Is_CAD_Running = false;*/
}



/*!
 * @brief Callback prototype for sending. This is generated when the data gets accepted to be sent
 *
 * @param [IN] success	Wheter
 * @param [IN] number_of_bytes_in_buffer
 *
 */
void SX1276_HAL_DataStatusCallback ( bool success, UINT16 number_of_bytes_in_buffer ){
	DataStatusCallback_success = success;
	DataStatusCallback_number_of_bytes_in_buffer = number_of_bytes_in_buffer;
}


DeviceStatus SX1276_HAL_Initialize(RadioEventHandler *event_handler){

	internal_radio_properties.SetDefaults(10, 10, 1000, 1000, MODEM_LORA);
	
	SX1276_Packet_ClearPayload();

	Radio_event_handler.SetReceiveHandler(event_handler->GetReceiveHandler());
	Radio_event_handler.SetSendAckHandler(event_handler->GetSendAckHandler());
	Radio_event_handler.SetRadioInterruptHandler(event_handler->GetRadioInterruptHandler());
	
	events.TxDone 				= SX1276_HAL_TxDone;
	events.TxTimeout 			= SX1276_HAL_TxTimeout;
	events.RxDone 				= SX1276_HAL_RxDone;
	events.RxTimeout 			= SX1276_HAL_RxTimeout;
	events.RxError 				= SX1276_HAL_RxError;
	events.FhssChangeChannel 	= SX1276_HAL_FhssChangeChannel;
	events.CadDone 				= SX1276_HAL_CadDone; 
	events.DataStatusCallback 	= SX1276_HAL_DataStatusCallback;
	events.ValidHeaderDetected  = SX1276_HAL_ValidHeaderDetected;

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
		SX1276_HAL_SetAddress(tempNum);
		//SX1276_HAL_SetAddress(25084);

		//hal_printf("Address: %d\r\n", tempNum);
	}
	
	received_ts_ticks = UNSET_TS;
	
	CPU_SPI_Init(SPI_TYPE_RADIO);
	
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(VIRT_TIMER_SX1276_PacketLoadTimerName, 0, 1000, TRUE, FALSE, SX1276_HAL_PacketLoadTimerHandler);
	rm = VirtTimer_SetTimer(VIRT_TIMER_SX1276_PacketTxTimerName, 0, 1000, TRUE, FALSE, SX1276_HAL_PacketTxTimerHandler);
	//rm = VirtTimer_SetTimer(VIRT_TIMER_SX1276_CADTimer, 0,  1000, TRUE, FALSE, SX1276_HAL_CADTimerHandler, LOW_DRIFT_TIMER);
	rm = VirtTimer_SetTimer(VIRT_TIMER_SX1276_CADTimer, 0,  1000, TRUE, FALSE, SX1276_HAL_CADTimerHandler);

	SX1276Init(&events);
	
	SX1276_HAL_ChooseRadioConfig();
	
	return DS_Success;
}

DeviceStatus SX1276_HAL_UnInitialize(){

	events.TxDone 				= NULL;
	events.TxTimeout 			= NULL;
	events.RxDone 				= NULL;
	events.RxTimeout 			= NULL;
	events.RxError 				= NULL;
	events.FhssChangeChannel 	= NULL;
	events.CadDone 				= NULL; 
	events.DataStatusCallback 	= NULL;
	
	m_rm = SLEEP;
	SX1276_Packet_ClearPayload();
	
	CPU_SPI_DeInit(SPI_TYPE_RADIO);
	
	return DS_Success;
}

void* SX1276_HAL_Send(void* msg, UINT16 size, UINT32 eventTime, bool request_ack, bool saveCopyOfPacket) {
	DataStatusCallback_success = false;
	
	if(!SX1276_HAL_IsPacketTransmittable(msg, size)) {
		SX1276_HAL_DataStatusCallback(false,size);
		return NULL;
	}
	if(saveCopyOfPacket){
		bool rv = SX1276_Packet_PreparePayload(msg, size, 0, 0);
		if(!rv){
			SX1276_HAL_DataStatusCallback(false, size);
			return NULL;
		}
	}
	
	//if radio layer accepted pkt return true, else return false.
	//SX1276Send(SX1276_Packet_GetPayload(), size, eventTime);
	SX1276Send(static_cast<uint8_t *>(msg), size, eventTime);
	SX1276_HAL_DataStatusCallback(true,size);	
	return msg;
}

void SX1276_HAL_RequestSendAtTimeInstants(void* msg, UINT16 size, TimeVariable_t PacketTransmissionTime, ClockIdentifier_t ClockIdentifier){ //TODO:
	if(!SX1276_HAL_IsPacketTransmittable(msg, size)) { //Reject if the incoming packet is not transferrable
		SX1276_HAL_DataStatusCallback(false, size);
		return;
	}
	UINT64 curtime = VirtTimer_GetTicks(ClockIdentifier);
	if(PacketTransmissionTime < curtime){
		SX1276_HAL_DataStatusCallback(false, size);
		return;
	}
	UINT64 delay = VirtTimer_TicksToTime(ClockIdentifier, PacketTransmissionTime - curtime);

	if(delay < internal_radio_properties.TIME_ADVANCE_FOR_SCHEDULING_A_PACKET_MICRO){
		SX1276_HAL_DataStatusCallback(false, size);
		return;
	}
	delay = delay - internal_radio_properties.TIME_ADVANCE_FOR_SCHEDULING_A_PACKET_MICRO;


	bool rv = SX1276_HAL_SetTimer(VIRT_TIMER_SX1276_PacketLoadTimerName, 0, delay, TRUE, LOW_PRECISION_CLOCK_ID);
	if(!rv){
		SX1276_HAL_DataStatusCallback(false, size);
		return;
	}
	rv = SX1276_Packet_PreparePayload(msg, size, PacketTransmissionTime, ClockIdentifier);
	if(!rv){
		SX1276_HAL_DataStatusCallback(false, size);
		return;
	}
	SX1276_HAL_DataStatusCallback(true, size);
	return;
}


void SX1276_HAL_RequestCancelSend(){
	// Initializes the payload size
	SX1276Write( REG_LR_PAYLOADLENGTH, 0 );

	// Full buffer used for Tx
	SX1276Write( REG_LR_FIFOTXBASEADDR, 0 );
	SX1276Write( REG_LR_FIFOADDRPTR, 0 );

	preloadedMsgSize = 0;
	SX1276_HAL_DataStatusCallback(true, 0);
}

DeviceStatus SX1276_HAL_AddToTxBuffer(void* msg, UINT16 size){
	// FIFO operations can not take place in Sleep mode
	if(size + preloadedMsgSize > MAX_PACKET_SIZE) return DS_Fail;

	if( ( SX1276Read( REG_OPMODE ) & ~RF_OPMODE_MASK ) == RF_OPMODE_SLEEP )
	{
		SX1276SetStby( );
		return DS_Fail;
	}
	preloadedMsgSize += size;
	SX1276WriteFifo(static_cast<uint8_t*>(msg),size);

//	g_SX1276M1BxASWrapper.SetOpMode( RFLR_OPMODE_SYNTHESIZER_TX );
	SX1276SetTx(1000 + SX1276GetTimeOnAir(internal_radio_properties.radio_modem, SX1276_Packet_GetSize()));

	return DS_Success;
}


DeviceStatus SX1276_HAL_ChannelActivityDetection(){
	Is_CAD_Running = true;
	CAD_Status = true;
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Start(VIRT_TIMER_SX1276_CADTimer);

	m_rm = RX;
	//SX1276StartCad();

	UINT32 i = 1;
	//while(Is_CAD_Running && i < 20000){
	//	i++;
		//hal_printf("CAD Running");
	//};
	
	return DS_Success;
	//if(CAD_Status) return DS_Success;
	//else return DS_Fail;
}

void SX1276_HAL_PacketLoadTimerHandler(void* param) {
	UINT64 delay;
	SX1276SetStby( );
	m_rm = STANDBY;
	UINT64 curtime = VirtTimer_GetTicks(SX1276_Packet_GetClockId());
	SX1276WriteFifo(SX1276_Packet_GetPayload(), SX1276_Packet_GetSize());
 	SX1276_Packet_MarkUploaded();
	if(curtime >= SX1276_Packet_GetDueTime()){ //Failed to load and send correctly
		delay = 10000;
	}
	else{
		delay = VirtTimer_TicksToTime(SX1276_Packet_GetClockId(), SX1276_Packet_GetDueTime() - curtime);
	}
	SX1276_HAL_SetTimer(VIRT_TIMER_SX1276_PacketTxTimerName, 0, delay, TRUE, HIGH_PRECISION_CLOCK_ID ); //Schedule PacketTxTimerHandler
}

void SX1276_HAL_PacketTxTimerHandler(void* param) {
	if(!SX1276_Packet_IsMsgUploaded()) {
		m_rm = TX;
		SX1276SetTx(1000 + SX1276GetTimeOnAir(internal_radio_properties.radio_modem, SX1276_Packet_GetSize()));
	}
}

void SX1276_HAL_CADTimerHandler(void * param){
	SX1276_HAL_CadDone(true);
}

bool SX1276_HAL_SetTimer(UINT8 timer_id, UINT32 start_delay, UINT32 period, BOOL is_one_shot, UINT8 hardwareTimerId) {
 	VirtualTimerReturnMessage rm = TimerSupported;
	if(rm != TimerSupported) return false;  rm = VirtTimer_Stop(timer_id);
	if(rm != TimerSupported) return false;  rm = VirtTimer_Change(timer_id, start_delay, period, is_one_shot, hardwareTimerId);
	if(rm != TimerSupported) return false;  rm = VirtTimer_Start(timer_id);
	return true;
}

bool SX1276_HAL_IsPacketTransmittable(void* msg, UINT16 size) {
	if(SX1276_Packet_GetSize() > 0){ //Reject the request if there is a packet scheduled
		return false;
	}
	return (size <= MAX_PACKET_SIZE);
}

DeviceStatus SX1276_HAL_TurnOnRx(){
	if(!SX1276_Packet_IsMsgUploaded()) {
		m_rm = RX;
		SX1276SetRx(0);
	}
	if (SX1276_HAL_GetRadioState() == RX) return DS_Success;
	return DS_Fail;	
}

DeviceStatus SX1276_HAL_Sleep(){
	if(!SX1276_Packet_IsMsgUploaded()) {
		m_rm = SLEEP;
		SX1276SetSleep();
	}
	if (SX1276_HAL_GetRadioState() == SLEEP) return DS_Success;
	return DS_Fail;
}

DeviceStatus SX1276_HAL_Standby(){
	if(!SX1276_Packet_IsMsgUploaded()) {
		m_rm = STANDBY;
		SX1276SetStby();
	}
	if (SX1276_HAL_GetRadioState() == STANDBY) return DS_Success;
	return DS_Fail;
	
}

DeviceStatus SX1276_HAL_Reset() {
	hal_printf("Reset\n\r");
	m_rm = SLEEP;
	SX1276_Packet_ClearPayload();
	
	SX1276Reset( );

	// Calibrate Rx chain
	RxChainCalibration( );

	// Initialize radio default values
	SX1276SetOpMode( RF_OPMODE_SLEEP );

	SX1276SetRadioRegistersInit();
	
	SX1276SetModem( MODEM_FSK );

	// Restore previous network type setting.
	SX1276SetPublicNetwork( SX1276.Settings.LoRa.PublicNetwork );
	// END WORKAROUND

	SX1276.Settings.State = RF_IDLE;
	
	return DS_Success;
}

void SX1276_HAL_ChooseRadioConfig() {

    SX1276SetChannel( RF_FREQUENCY );

#if defined( USE_MODEM_LORA )
	SX1276SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    SX1276SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
#elif defined( USE_MODEM_FSK )

    SX1276SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );

    SX1276SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

#endif

}

UINT32 SX1276_HAL_ReadRssi() {
	UINT32 value;	
#if defined( USE_MODEM_LORA )
	value = (UINT32)SX1276ReadRssi(MODEM_LORA);
#elif defined( USE_MODEM_FSK )
	value = (UINT32)SX1276ReadRssi(MODEM_FSK);
#endif
	return value;
}

RadioMode_t SX1276_HAL_GetRadioState() {
	return m_rm;
}

UINT16 SX1276_HAL_GetAddress(){
	return radio_address;
}
BOOL SX1276_HAL_SetAddress(UINT16 address){
	radio_address = address;
	return true;
}

INT8 SX1276_HAL_GetRadioName(){
	return radioName;
}
void SX1276_HAL_SetRadioName(INT8 rn){
	radioName = rn;
}

bool SX1276_Packet_PreparePayload(void* msg, UINT16 size, const UINT64& t, ClockIdentifier_t c) {
	if(SX1276_Packet_IsMsgSaved()) return false;
	SX1276_Packet_ClearPayload();
	m_packet.msg_size = size;
	memcpy(m_packet.msg_payload, msg, m_packet.msg_size );
	m_packet.due_time = t;
	m_packet.clock_id = c;
	return true;
}

void SX1276_Packet_ClearPayload() {
	m_packet.msg_size = 0;
	m_packet.isUploaded = false;
	m_packet.due_time = 0;
}

void SX1276_Packet_MarkUploaded() {
	m_packet.isUploaded = true;
}

bool SX1276_Packet_IsMsgSaved() {
	if(m_packet.msg_size > 0 ) return true;
	return false;
}

bool SX1276_Packet_IsMsgUploaded() {
	return m_packet.isUploaded;
}

UINT16 SX1276_Packet_GetSize() {
	return m_packet.msg_size;
}

UINT8* SX1276_Packet_GetPayload() {
	return m_packet.msg_payload;
}

UINT64 SX1276_Packet_GetDueTime() {
	return m_packet.due_time;
}

ClockIdentifier_t SX1276_Packet_GetClockId() {
	return m_packet.clock_id;
}

