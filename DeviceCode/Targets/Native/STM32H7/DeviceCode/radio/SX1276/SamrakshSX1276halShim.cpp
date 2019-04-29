/*
 * SamrakshSX1276hal.cpp
 *
 *  Created on: Aug 18, 2017
 *      Author: Bora
 */

#include "SamrakshSX1276halShim.h"
#include <Samraksh/VirtualTimer.h>
#include "SamrakshSX1276Parameters.h"
#include <Samraksh/Message.h>

extern EMOTE_SX1276_LORA::Samraksh_SX1276_hal gsx1276radio;
EMOTE_SX1276_LORA::Samraksh_SX1276_hal_netmfadapter gsx1276radio_netmf_adapter;

#define UNSET_TS 0xFFFFFFFFFFFFFFFF
namespace EMOTE_SX1276_LORA {

// This somehow gets put in the radio function. Out of scope for now, but fix me later.
static void GetCPUSerial(uint8_t * ptr, unsigned num_of_bytes ){
	unsigned Device_Serial0;unsigned Device_Serial1; unsigned Device_Serial2;
	Device_Serial0 = *(unsigned*)(0x1FFFF7E8);
	Device_Serial1 = *(unsigned*)(0x1FFFF7EC);
	Device_Serial2 = *(unsigned*)(0x1FFFF7F0);
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

void CADTimerHandler(void * param){
	//gsx1276radio_netmf_adapter.CAD_Status = true;
	//gsx1276radio_netmf_adapter.Is_CAD_Running = false;
	gsx1276radio_netmf_adapter.CadDone(true);
}

DeviceStatus Samraksh_SX1276_hal_netmfadapter::CPU_Radio_Initialize(RadioEventHandler* event_handler){

	Radio_event_handler.SetReceiveHandler(event_handler->GetReceiveHandler());
	Radio_event_handler.SetSendAckHandler(event_handler->GetSendAckHandler());
	Radio_event_handler.SetRadioInterruptHandler(event_handler->GetRadioInterruptHandler());

	radio_events.TxDone = TxDone;
	radio_events.PacketDetected = PacketDetected;
	radio_events.RxDone = RxDone;
	radio_events.CadDone = CadDone;
	radio_events.DataStatusCallback = DataStatusCallback;

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
		SetAddress(tempNum);
	}

	received_ts_ticks = UNSET_TS;

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(VIRT_TIMER_SX1276_CADTimer, 0,  1000, TRUE, FALSE, CADTimerHandler, LOW_DRIFT_TIMER);

	//Samraksh_SX1276_hal* base_ptr = this;
	DeviceStatus ds = gsx1276radio.Initialize(radio_events);
	//DeviceStatus ds = gsx1276radio.Initialize(radio_events);

	return ds;
}

void Samraksh_SX1276_hal_netmfadapter::TxDone (bool success){
	NetOpStatus ns;
	UINT8 radioAckStatus;
	if(success) {
		ns = NetworkOperations_Success;
		radioAckStatus = NetworkOperations_Success;
	}
	else {
		ns = NetworkOperations_Fail;
		radioAckStatus = NetworkOperations_Success;
	}
	gsx1276radio_netmf_adapter.TurnOnRx();
	(gsx1276radio_netmf_adapter.Radio_event_handler.GetSendAckHandler())(static_cast<void*>(gsx1276radio.m_packet.GetPayload()), gsx1276radio.m_packet.GetSize(), ns, radioAckStatus);
}

void  Samraksh_SX1276_hal_netmfadapter::PacketDetected( void ){
	void* dummy_ptr=NULL;
	gsx1276radio_netmf_adapter.received_ts_ticks = HAL_Time_CurrentTicks();
	gsx1276radio_netmf_adapter.Radio_event_handler.RadioInterruptHandler(StartOfReception, dummy_ptr);
}

/*!
 * @brief Rx Done callback prototype. In case of rx error payload is null and the size is zero
 *
 * @param [IN] payload Received buffer pointer
 * @param [IN] size    Received buffer size
 */
void    Samraksh_SX1276_hal_netmfadapter::RxDone( uint8_t *payload, uint16_t size ){
	if(payload == NULL) {
		//hal_printf("RX timeout");
		gsx1276radio_netmf_adapter.TurnOnRx();
		  // Reset the radio
		return;
	}
	Message_15_4_t* pckt_ptr = reinterpret_cast<Message_15_4_t*>(payload);
	if(gsx1276radio_netmf_adapter.received_ts_ticks == UNSET_TS)
		gsx1276radio_netmf_adapter.received_ts_ticks = HAL_Time_CurrentTicks();
	pckt_ptr->GetMetaData()->SetReceiveTimeStamp(gsx1276radio_netmf_adapter.received_ts_ticks);
	(gsx1276radio_netmf_adapter.Radio_event_handler.GetReceiveHandler())(payload, size);
}

/*!
 * @brief CAD Done callback prototype.
 *
 * @param [IN] channelDetected    Whether Channel Activity detected during the CAD
 */
void Samraksh_SX1276_hal_netmfadapter::CadDone( bool channelActivityDetected ){
	gsx1276radio_netmf_adapter.CAD_Status = channelActivityDetected;
	gsx1276radio_netmf_adapter.Is_CAD_Running = false;
}

DeviceStatus Samraksh_SX1276_hal_netmfadapter::CPU_Radio_ClearChannelAssesment(){
	return DS_Success;
	Is_CAD_Running = true;
	CAD_Status = true;
	VirtualTimerReturnMessage rm;
	rm = VirtTimer_Start(VIRT_TIMER_SX1276_CADTimer);

	gsx1276radio.ChannelActivityDetection();

	UINT32 i = 1;
	while(Is_CAD_Running && i < 20000){
		i++;
		//hal_printf("CAD Running");
	};

	if(CAD_Status) return DS_Success;
	else DS_Fail;
}

/*!
 * @brief Callback prototype for sending. This is generated when the data gets accepted to be sent
 *
 * @param [IN] success	Wheter
 * @param [IN] number_of_bytes_in_buffer
 *
 */
void Samraksh_SX1276_hal_netmfadapter::DataStatusCallback ( bool success, UINT16 number_of_bytes_in_buffer ){
	gsx1276radio_netmf_adapter.DataStatusCallback_success = success;
	gsx1276radio_netmf_adapter.DataStatusCallback_number_of_bytes_in_buffer = number_of_bytes_in_buffer;
}

void* Samraksh_SX1276_hal_netmfadapter::Send(void* msg, UINT16 size){
	gsx1276radio_netmf_adapter.DataStatusCallback_success = false;
	gsx1276radio.Send(msg,size,true,true);
	if(gsx1276radio_netmf_adapter.DataStatusCallback_success){
		return msg; //gsx1276radio.m_packet.GetPayload();
	}
		return NULL;
}

void* Samraksh_SX1276_hal_netmfadapter::SendTS(void* msg, UINT16 size, UINT32 eventTime){
	gsx1276radio_netmf_adapter.DataStatusCallback_success = false;
	gsx1276radio.SendTS(msg,size,eventTime, true,true);
	if(gsx1276radio_netmf_adapter.DataStatusCallback_success){
		return msg; //gsx1276radio.m_packet.GetPayload();
	}
	return NULL;
}

DeviceStatus Samraksh_SX1276_hal_netmfadapter::TurnOnRx(){
	if(gsx1276radio.StartListenning() == SamrakshRadio_I::RX)
		return DS_Success;
	else return DS_Fail;
}
DeviceStatus Samraksh_SX1276_hal_netmfadapter::Sleep(){
	if(gsx1276radio.Sleep() == SamrakshRadio_I::SLEEP)
		return DS_Success;
	else return DS_Fail;
}

} /* namespace Samraksh_SX1276 */
