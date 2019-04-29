/*
 * SamrakshSX1276hal.h
 *
 *  Created on: Aug 18, 2017
 *      Author: Bora
 */

#ifndef MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_SAMRAKSHSX1276HAL_NETMFADAPTER_H_
#define MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_SAMRAKSHSX1276HAL_NETMFADAPTER_H_

#include "SamrakshSX1276hal.h"
#include <Samraksh/Radio_decl.h>

namespace EMOTE_SX1276_LORA {
/*!
 * @brief Samraksh_SX1276_hal: 	The class that converts calls from netmf_radio to SamrakshRadio_I interface
 */
class Samraksh_SX1276_hal_netmfadapter {
protected:
	UINT64 received_ts_ticks;
	UINT16 radio_address;
	INT8 radioName;
	bool DataStatusCallback_success;
	UINT16 DataStatusCallback_number_of_bytes_in_buffer;

public:
	bool Is_CAD_Running;
	bool CAD_Status;

	RadioEventHandler Radio_event_handler; //Radop events from netmf
	SamrakshRadio_I::RadioEvents_t radio_events; //Radio events to SamrakshRadio_I
public:
	static void TxDone (bool success);

    static void  PacketDetected( void );
    /*!
     * @brief Rx Done callback prototype. In case of rx error payload is null and the size is zero
     *
     * @param [IN] payload Received buffer pointer
     * @param [IN] size    Received buffer size
     */
    static void    RxDone( uint8_t *payload, uint16_t size );

    /*!
     * @brief CAD Done callback prototype.
     *
     * @param [IN] channelDetected    Whether Channel Activity detected during the CAD
     */
    static void CadDone( bool channelActivityDetected );

    /*!
     * @brief Callback prototype for sending. This is generated when the data gets accepted to be sent
     *
     * @param [IN] success	Wheter
     * @param [IN] number_of_bytes_in_buffer
     *
     */
    static void  DataStatusCallback ( bool success, UINT16 number_of_bytes_in_buffer );
public:

	DeviceStatus CPU_Radio_Initialize(RadioEventHandler* event_handler); //Initializes Return the ID of the Radio layer that was initialized
	void* Send(void* msg, UINT16 size);
	void* SendTS(void* msg, UINT16 size,  UINT32 eventTime);
	DeviceStatus CPU_Radio_ClearChannelAssesment();
	DeviceStatus TurnOnRx();
	DeviceStatus Sleep();

	UINT16 GetAddress(){
		return radio_address;
	}
	BOOL SetAddress(UINT16 address){
		radio_address = address;
		return true;
	}

	INT8 GetRadioName(){
		return radioName;
	}
	void SetRadioName(INT8 rn){
		radioName = rn;
	}
	/*	BOOL CPU_Radio_UnInitialize(UINT8 radioIDs);
	UINT8 CPU_Radio_GetRadioIDs(UINT8* radioIDs);
	void* CPU_Radio_Preload(UINT8 radioID,void * msg, UINT16 size);
	void* CPU_Radio_SendRetry(UINT8 radioID);
	void* CPU_Radio_SendStrobe(UINT8 radioID, UINT16 size);
	void* CPU_Radio_Send(UINT8 radioID,void * msg, UINT16 size);

	void* CPU_Radio_Send_TimeStamped(UINT8 radioID,void * msg, UINT16 size, UINT32 eventTime);
	NetOpStatus CPU_Radio_PreloadMessage(UINT8* msg, UINT16 size);
	DeviceStatus CPU_Radio_Send_Strobe(UINT8 radioID);	//Send preloaded message
	DeviceStatus CPU_Radio_ClearChannelAssesment (UINT8 radioID);
	DeviceStatus CPU_Radio_ClearChannelAssesment(UINT8 radioID, UINT32 numberMicroSecond);
	//DeviceStatus CPU_Radio_EnableCSMA(UINT8 radioID);
	//DeviceStatus CPU_Radio_DisableCSMA(UINT8 radioID);
	DeviceStatus CPU_Radio_TurnOnRx(UINT8 radioID);
	DeviceStatus CPU_Radio_TurnOffRx(UINT8 radioID);
	DeviceStatus CPU_Radio_TurnOnPLL(UINT8 radioID);
	DeviceStatus CPU_Radio_Sleep(UINT8 radioID, UINT8 level);
	BOOL CPU_Radio_SetTimeStamp(UINT8 radioID);
	BOOL CPU_Radio_SetTimeStamp2(UINT8 radioID, UINT64 timeStamp);
	UINT32 CPU_Radio_GetSNR(UINT8 radioID);
	UINT32 CPU_Radio_GetRSSI(UINT8 radioID);
	UINT32 CPU_Radio_SetDefaultRxState(UINT8 state); // default state is sleep, so CSMA needs to call this to keep RX always on

	//Aggregate apis
	BOOL CPU_Radio_SwitchType(UINT8 radioID);
	//Radio* CPU_Radio_GetRadio(UINT8 radioID);
	BOOL CPU_Radio_Intialize(UINT8  radioID);
	BOOL CPU_Radio_UnInitialize(UINT8 radioID);
	UINT8 CPU_Radio_NumberRadiosSupported();

	BOOL CPU_Radio_Reset(UINT8 radioID);
	UINT16 CPU_Radio_GetAddress(UINT8 radioID);
	BOOL CPU_Radio_SetAddress(UINT8 radioID, UINT16 address);

	RadioType CPU_Radio_GetRadioType();
	DeviceStatus CPU_Radio_SetRadioType(RadioType radioType);

	INT8 CPU_Radio_GetRadioAckType();
	INT8 CPU_Radio_GetRadioName();
	DeviceStatus CPU_Radio_SetRadioName(INT8 radioName);

	DeviceStatus CPU_Radio_ChangeTxPower(UINT8 radioID, int power);

	DeviceStatus CPU_Radio_ChangeChannel(UINT8 radioID, int channel);

	UINT32 CPU_Radio_GetTxPower(UINT8 radioID);

	UINT32 CPU_Radio_GetChannel(UINT8 radioID);*/
};
} /* namespace Samraksh_SX1276 */
#endif /* MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_SAMRAKSHSX1276HAL_NETMFADAPTER_H_ */
