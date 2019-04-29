/*
 * SamrakshSX1276hal.cpp
 *
 *  Created on: Aug 18, 2017
 *      Author: Bora
 */

#include "SamrakshSX1276hal.h"
#include <Samraksh/VirtualTimer.h>
#include "SamrakshSX1276Parameters.h"

EMOTE_SX1276_LORA::Samraksh_SX1276_hal gsx1276radio;

//extern SX1276_Semtech::SX1276M1BxASWrapper g_SX1276M1BxASWrapper;

extern SX1276M1BxASWrapper g_SX1276M1BxASWrapper;

namespace EMOTE_SX1276_LORA {




//const Samraksh_SX1276_hal::InternalRadioProperties_t Samraksh_SX1276_hal::SX1276_hal_wrapper_internal_radio_properties(10, 10, 1000, 100, MODEM_LORA);

//Samraksh_SX1276_hal grfsx1276Radio;

void Samraksh_SX1276_hal::ValidHeaderDetected(){
	if(gsx1276radio.m_re.PacketDetected) gsx1276radio.m_re.PacketDetected();
}
void Samraksh_SX1276_hal::TxDone(){
	if(gsx1276radio.m_re.TxDone) gsx1276radio.m_re.TxDone(true);
	gsx1276radio.m_packet.ClearPaylod();
}
void Samraksh_SX1276_hal::TxTimeout(){
	gsx1276radio.m_packet.ClearPaylod();
	gsx1276radio.ChooseRadioConfig();
	if(gsx1276radio.m_re.TxDone) gsx1276radio.m_re.TxDone(false);
};
void Samraksh_SX1276_hal::RxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr){
	//hal_printf("size: %u rssi: %d dBm snr: %d dB\r\n", size, rssi, snr);
	if(gsx1276radio.m_re.RxDone) gsx1276radio.m_re.RxDone(payload, size);
}
void Samraksh_SX1276_hal::RxTimeout(){
	g_SX1276M1BxASWrapper.Reset();
	g_SX1276M1BxASWrapper.IoInit( );
	g_SX1276M1BxASWrapper.RxChainCalibration();
	g_SX1276M1BxASWrapper.SetOpMode( RF_OPMODE_SLEEP );
	g_SX1276M1BxASWrapper.IoIrqInit();
	g_SX1276M1BxASWrapper.RadioRegistersInit();
	g_SX1276M1BxASWrapper.SetModem(MODEM_LORA);
	gsx1276radio.ChooseRadioConfig();
	gsx1276radio.StartListenning();

//	if(gsx1276radio.m_re.RxDone) gsx1276radio.m_re.RxDone(NULL, 0);
}
void Samraksh_SX1276_hal::RxError(){
	if(gsx1276radio.m_re.RxDone) gsx1276radio.m_re.RxDone(NULL, 0);
}
void Samraksh_SX1276_hal::FhssChangeChannel(uint8_t currentChannel ){

}
void Samraksh_SX1276_hal::CadDone(bool channelActivityDetected){
	gsx1276radio.m_rm = SLEEP;
	if(gsx1276radio.m_re.CadDone) gsx1276radio.m_re.CadDone(channelActivityDetected);
}



Samraksh_SX1276_hal::Samraksh_SX1276_hal()
: isRadioInitialized(false)
, m_rp() 	//TODO: BK: We need to initialize these parameters
, isCallbackIssued(false)
{

}


Samraksh_SX1276_hal::~Samraksh_SX1276_hal() {
}


DeviceStatus Samraksh_SX1276_hal::Initialize(SamrakshRadio_I::RadioEvents_t re){
	if(isRadioInitialized) return DS_Fail;

	isCallbackIssued = false;

	SX1276_hal_wrapper_internal_radio_properties.SetDefaults(10, 10, 1000, 1000, MODEM_LORA);


	sx1276_re.ValidHeaderDetected = Samraksh_SX1276_hal::ValidHeaderDetected;
	sx1276_re.TxDone = Samraksh_SX1276_hal::TxDone;
	sx1276_re.TxTimeout = Samraksh_SX1276_hal::TxTimeout;
	sx1276_re.RxDone = Samraksh_SX1276_hal::RxDone;
	sx1276_re.RxTimeout = Samraksh_SX1276_hal::RxTimeout;
	sx1276_re.RxError = Samraksh_SX1276_hal::RxError;
	sx1276_re.FhssChangeChannel = Samraksh_SX1276_hal::FhssChangeChannel;
	sx1276_re.CadDone = Samraksh_SX1276_hal::CadDone;


	m_re.CadDone = re.CadDone;
	m_re.DataStatusCallback = re.DataStatusCallback;
	m_re.PacketDetected = re.PacketDetected;
	m_re.RxDone = re.RxDone;
	m_re.TxDone = re.TxDone;


	SanityCheckOnConstants();

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(PacketLoadTimerName, 0, 1000, TRUE, FALSE, Samraksh_SX1276_hal::PacketLoadTimerHandler);
	rm = VirtTimer_SetTimer(PacketTxTimerName, 0, 1000, TRUE, FALSE, Samraksh_SX1276_hal::PacketTxTimerHandler);

	g_SX1276M1BxASWrapper.Initialize(&sx1276_re);

	ChooseRadioConfig();

	isRadioInitialized = true;
	return DS_Success;
}
DeviceStatus Samraksh_SX1276_hal::UnInitialize(){
	bool t = isCallbackIssued;
	isCallbackIssued = false; Sleep(); while(!isCallbackIssued){} isCallbackIssued = t;
	m_re.CadDone = NULL;
	m_re.DataStatusCallback = NULL;
	m_re.PacketDetected = NULL;
	m_re.RxDone = NULL;
	m_re.TxDone = NULL;

//	m_re = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	isRadioInitialized  = false;
	return DS_Success;
}
DeviceStatus Samraksh_SX1276_hal::IsInitialized(){
	if(isRadioInitialized) return DS_Success;
	return DS_Fail;

}

DeviceStatus Samraksh_SX1276_hal::SetAddress(){
	//BK: Currently refuse and return the current properties;
	return DS_Fail;
}

SamrakshRadio_I::RadioProperties_t Samraksh_SX1276_hal::GetRadioProperties(){
	return m_rp;
}

void Samraksh_SX1276_hal::Send(void* msg, UINT16 size, bool request_ack, bool saveCopyOfPacket) {
	if(!IsPacketTransmittable(msg, size)) {
		m_re.DataStatusCallback(false,size);
		return;
	}
	if(saveCopyOfPacket){
		bool rv = m_packet.PreparePayload(msg, size, 0, 0);
		if(!rv){
			m_re.DataStatusCallback(false, size);
			return;
		}
	}

	//g_SX1276M1BxASWrapper.Send(static_cast<uint8_t *>(msg), size);
	if(g_SX1276M1BxASWrapper.Send(m_packet.GetPayload(), size)){
		m_re.DataStatusCallback(true,size);
	}
	m_re.DataStatusCallback(false,size);
}
void Samraksh_SX1276_hal::SendTS(void* msg, UINT16 size, UINT32 eventTime, bool request_ack, bool saveCopyOfPacket) {
	if(!IsPacketTransmittable(msg, size)) {
		m_re.DataStatusCallback(false,size);
		return;
	}
	if(saveCopyOfPacket){
		bool rv = m_packet.PreparePayload(msg, size, 0, 0);
		if(!rv){
			m_re.DataStatusCallback(false, size);
			return;
		}
	}
	m_re.DataStatusCallback(true,size);

	//if radio layer accepted pkt return true, else return false.
	if(g_SX1276M1BxASWrapper.SendTS(m_packet.GetPayload(), size, eventTime)){
		m_re.DataStatusCallback(true,size);
	}
	m_re.DataStatusCallback(false,size);
}


void Samraksh_SX1276_hal::RequestSendAtTimeInstanst(void* msg, UINT16 size, TimeVariable_t PacketTransmissionTime, ClockIdentifier_t ClockIdentifier){ //TODO:
	//	m_re.DataStatusCallback(false, 0); //

	if(!IsPacketTransmittable(msg, size)) { //Reject if the incoming packet is not transferrable
		m_re.DataStatusCallback(false, size);
		return;
	}
	UINT64 curtime = VirtTimer_GetTicks(ClockIdentifier);
	if(PacketTransmissionTime < curtime){
		m_re.DataStatusCallback(false, size);
		return;
	}
	UINT64 delay = VirtTimer_TicksToTime(ClockIdentifier, PacketTransmissionTime - curtime);

	if(delay < SX1276_hal_wrapper_internal_radio_properties.TIME_ADVANCE_FOR_SCHEDULING_A_PACKET_MICRO){
		m_re.DataStatusCallback(false, size);
		return;
	}
	delay = delay - SX1276_hal_wrapper_internal_radio_properties.TIME_ADVANCE_FOR_SCHEDULING_A_PACKET_MICRO;


	bool rv = SetTimer(PacketLoadTimerName,0, delay, TRUE, low_precision_clock_id);
	if(!rv){
		m_re.DataStatusCallback(false, size);
		return;
	}
	rv = m_packet.PreparePayload(msg, size, PacketTransmissionTime, ClockIdentifier);
	if(!rv){
		m_re.DataStatusCallback(false, size);
		return;
	}
	m_re.DataStatusCallback(true, size);
	return;


}


void Samraksh_SX1276_hal::RequestCancelSend(){
	// Initializes the payload size
	g_SX1276M1BxASWrapper.Write( REG_LR_PAYLOADLENGTH, 0 );

	// Full buffer used for Tx
	g_SX1276M1BxASWrapper.Write( REG_LR_FIFOTXBASEADDR, 0 );
	g_SX1276M1BxASWrapper.Write( REG_LR_FIFOADDRPTR, 0 );

	preloadedMsgSize = 0;
	m_re.DataStatusCallback(true, 0);
}

DeviceStatus Samraksh_SX1276_hal::AddToTxBuffer(void* msg, UINT16 size){
	// FIFO operations can not take place in Sleep mode
	if(size + preloadedMsgSize > SX1276_hal_wrapper_max_packetsize) return DS_Fail;

	if( ( g_SX1276M1BxASWrapper.Read( REG_OPMODE ) & ~RF_OPMODE_MASK ) == RF_OPMODE_SLEEP )
	{
		g_SX1276M1BxASWrapper.Standby( );
		return DS_Fail;
	}
	preloadedMsgSize += size;
	g_SX1276M1BxASWrapper.WriteFifo(static_cast<uint8_t*>(msg),size);

//	g_SX1276M1BxASWrapper.SetOpMode( RFLR_OPMODE_SYNTHESIZER_TX );
	g_SX1276M1BxASWrapper.Tx(1000 + g_SX1276M1BxASWrapper.TimeOnAir(SX1276_hal_wrapper_internal_radio_properties.radio_modem, m_packet.GetSize()));

	return DS_Success;
}


void Samraksh_SX1276_hal::ChannelActivityDetection(){
	m_rm = RX;
	g_SX1276M1BxASWrapper.StartCad();
}

void Samraksh_SX1276_hal::PacketLoadTimerHandler(void* param) {
	UINT64 delay;
	g_SX1276M1BxASWrapper.Standby( );
	gsx1276radio.m_rm = STANDBY;
	UINT64 curtime = VirtTimer_GetTicks(gsx1276radio.m_packet.GetClockId());
	g_SX1276M1BxASWrapper.WriteFifo(gsx1276radio.m_packet.GetPayload(),gsx1276radio.m_packet.GetSize());
	gsx1276radio.m_packet.MarkUploaded();
	if(curtime >= gsx1276radio.m_packet.GetDueTime()){ //Failed to load and send correctly
//		gsx1276radio.m_re.DataStatusCallback(false, gsx1276radio.m_packet.GetSize());
//		return;
		delay = 10000;
	}
	else{
		delay = VirtTimer_TicksToTime(gsx1276radio.m_packet.GetClockId(), gsx1276radio.m_packet.GetDueTime() - curtime);
	}
	SetTimer(PacketTxTimerName, 0 , delay, TRUE, high_precision_clock_id ); //Schedule PacketTxTimerHandler
}

void Samraksh_SX1276_hal::PacketTxTimerHandler(void* param) {
	if(gsx1276radio.m_packet.IsMsgUploaded()){ gsx1276radio.m_rm = TX;
		g_SX1276M1BxASWrapper.Tx( 1000 +
				g_SX1276M1BxASWrapper.TimeOnAir(gsx1276radio.SX1276_hal_wrapper_internal_radio_properties.radio_modem, gsx1276radio.m_packet.GetSize())
				);
	}
}

bool Samraksh_SX1276_hal::SetTimer(UINT8 timer_id, UINT32 start_delay,
		UINT32 period, BOOL is_one_shot, UINT8 hardwareTimerId) {
	VirtualTimerReturnMessage rm = TimerSupported;
	if(rm != TimerSupported) return false;  rm = VirtTimer_Stop(timer_id);
	if(rm != TimerSupported) return false;  rm = VirtTimer_Change(timer_id,start_delay,period, is_one_shot, hardwareTimerId);
	if(rm != TimerSupported) return false;  rm = VirtTimer_Start(timer_id);
	return true;
}

bool Samraksh_SX1276_hal::IsPacketTransmittable(void* msg, UINT16 size) {

	if(m_packet.GetSize() > 0){ //Reject the request if there is a packet scheduled
		return false;
	}
	return (size <= SX1276_hal_wrapper_max_packetsize);
}




SamrakshRadio_I::RadioMode_t Samraksh_SX1276_hal::StartListenning(){
	if(m_packet.IsMsgUploaded()) return Samraksh_SX1276_hal::GetRadioState();
	g_SX1276M1BxASWrapper.Rx(0);
	m_rm = RX;
	return Samraksh_SX1276_hal::GetRadioState();
}

SamrakshRadio_I::RadioMode_t Samraksh_SX1276_hal::Sleep(){
	if(m_packet.IsMsgUploaded()) return Samraksh_SX1276_hal::GetRadioState();
	m_rm = SLEEP;
	g_SX1276M1BxASWrapper.Sleep();
	return Samraksh_SX1276_hal::GetRadioState();
}

SamrakshRadio_I::RadioMode_t Samraksh_SX1276_hal::Standby(){
	m_rm = STANDBY;
	g_SX1276M1BxASWrapper.Standby();
	return Samraksh_SX1276_hal::GetRadioState();
}

void Samraksh_SX1276_hal::ChooseRadioConfig() {



    g_SX1276M1BxASWrapper.SetChannel( RF_FREQUENCY );

#if defined( USE_MODEM_LORA )

	g_SX1276M1BxASWrapper.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    g_SX1276M1BxASWrapper.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
	//Change to single packet mode for receiving
	/*g_SX1276M1BxASWrapper.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
	                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
	                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
	                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, false );*/


#elif defined( USE_MODEM_FSK )

    g_SX1276M1BxASWrapper.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );

    g_SX1276M1BxASWrapper.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

#else
    #error "Please define a frequency band in the compiler options."
#endif

}

SamrakshRadio_I::RadioMode_t Samraksh_SX1276_hal::GetRadioState() {
	return m_rm;
}


bool Samraksh_SX1276_hal::msgToBeTransmitted_t::PreparePayload(void* msg,
	UINT16 size, const UINT64& t, ClockIdentifier_t c) {
	if(IsMsgSaved()) return false;
	ClearPaylod();
	msg_size = size;
	memcpy(msg_payload, msg, msg_size );
	due_time = t;
	clock_id = c;
	return true;
}

Samraksh_SX1276_hal::msgToBeTransmitted_t::msgToBeTransmitted_t(){
	ClearPaylod();

}
void Samraksh_SX1276_hal::msgToBeTransmitted_t::ClearPaylod() {
	msg_size = 0;
	isUploaded = false;
	due_time = 0;

}

void Samraksh_SX1276_hal::msgToBeTransmitted_t::MarkUploaded() {
	isUploaded = true;
}

bool Samraksh_SX1276_hal::msgToBeTransmitted_t::IsMsgSaved() {
	if(msg_size > 0 ) return true;
	return false;
}

bool Samraksh_SX1276_hal::msgToBeTransmitted_t::IsMsgUploaded() {
	return isUploaded;
}

UINT16 Samraksh_SX1276_hal::msgToBeTransmitted_t::GetSize() {
	return msg_size;
}

UINT8* Samraksh_SX1276_hal::msgToBeTransmitted_t::GetPayload() {
	return msg_payload;
}

UINT64 Samraksh_SX1276_hal::msgToBeTransmitted_t::GetDueTime() {
	return due_time;
}

SamrakshRadio_I::ClockIdentifier_t Samraksh_SX1276_hal::msgToBeTransmitted_t::GetClockId() {
	return clock_id;
}



} /* namespace Samraksh_SX1276 */

