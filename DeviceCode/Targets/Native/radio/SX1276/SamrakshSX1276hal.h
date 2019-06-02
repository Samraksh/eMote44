/*
 * SamrakshSX1276hal.h
 *
 *  Created on: Aug 18, 2017
 *      Author: Bora
 */

#ifndef MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_SAMRAKSHSX1276HAL_H_
#define MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_SAMRAKSHSX1276HAL_H_

#include <Samraksh/Radio.h>
#include "SX1276wrapper.h"


namespace EMOTE_SX1276_LORA {

/*!
 * @brief Samraksh_SX1276_hal: 	The class that implements the SamrakshRadio_I interface
								which is used by the above layers within Emote like OMAC
 */
class Samraksh_SX1276_hal : public  SamrakshRadio_I {
protected:
	SX1276RadioEvents_t sx1276_re;
public: //Public class definitions
//	SX1276M1BxASWrapper radio;


public: //SX1276_s internal callbacks as defined in the driver
	static void ValidHeaderDetected();
	static void TxDone();
	static void TxTimeout();
	static void RxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
	static void RxTimeout();
	static void RxError();
	static void FhssChangeChannel(uint8_t currentChannel );
	static void CadDone(bool channelActivityDetected);
public:
	static void PacketLoadTimerHandler (void* param);
	static void PacketTxTimerHandler (void* param);


	struct InternalRadioProperties_t {
		UINT64 RADIO_BUS_SPEED; //Number of bytes written in a second to the radio buffer
		UINT16 RADIO_TURN_ON_DELAY_TX_MICRO;	//Tx initiation delay
		uint32_t MAX_PACKET_TX_DURATION;

		UINT64 TIME_ADVANCE_FOR_SCHEDULING_A_PACKET_MICRO;


		RadioModems_t radio_modem;
		void SetDefaults(UINT64 i , UINT16 i2, uint32_t i3, UINT64 i4, RadioModems_t i5){
			RADIO_BUS_SPEED = i;
			RADIO_TURN_ON_DELAY_TX_MICRO = i2;
			MAX_PACKET_TX_DURATION = i3;
			TIME_ADVANCE_FOR_SCHEDULING_A_PACKET_MICRO = i4;
			radio_modem = i5;

		};
	};
	InternalRadioProperties_t SX1276_hal_wrapper_internal_radio_properties;
protected:
	static const UINT16 SX1276_hal_wrapper_max_packetsize = 255;

	static const ClockIdentifier_t low_precision_clock_id = 4; // low precision  clock id used when schedling  time to load
	static const ClockIdentifier_t high_precision_clock_id = 1; //high precision  clock id used when schedling exact time instant to tx
	static const UINT8 PacketLoadTimerName = VIRT_TIMER_SX1276_PacketLoadTimerName;
	static const UINT8 PacketTxTimerName = VIRT_TIMER_SX1276_PacketTxTimerName;

	class msgToBeTransmitted_t{
		UINT8 msg_payload[SX1276_hal_wrapper_max_packetsize];
		UINT16 msg_size;
		UINT64 due_time;
		ClockIdentifier_t clock_id;
		bool isUploaded;
	public:
		msgToBeTransmitted_t();
		bool PreparePayload(void* msg, UINT16 size, const UINT64& t, ClockIdentifier_t c);
		UINT16 GetSize();
		UINT8* GetPayload();
		UINT64 GetDueTime();
		ClockIdentifier_t GetClockId();
		void ClearPaylod();
		void MarkUploaded();
		bool IsMsgSaved();
		bool IsMsgUploaded();
	};
public:
	msgToBeTransmitted_t m_packet;
	SamrakshRadio_I::RadioEvents_t m_re;
	RadioMode_t m_rm;
private:

	bool isCallbackIssued;


	UINT16 preloadedMsgSize;


	RadioProperties_t m_rp;
	bool isRadioInitialized;


	static bool SetTimer(UINT8 timer_id, UINT32 start_delay, UINT32 period, BOOL is_one_shot, UINT8 hardwareTimerId);
	bool SanityCheckOnConstants(){
		//if (SX1276_hal_wrapper_max_packetsize > std::numeric_limits<uint8_t>::max()) return false;
		if (SX1276_hal_wrapper_max_packetsize > 256) return false;

		return true;
	}

	DeviceStatus AddToTxBuffer(void* msg, UINT16 size);
protected:
	bool IsPacketTransmittable(void* msg, UINT16 size);
public:
	void ChooseRadioConfig();
public:
	Samraksh_SX1276_hal();
	virtual ~Samraksh_SX1276_hal();

	DeviceStatus Initialize(SamrakshRadio_I::RadioEvents_t re);
	DeviceStatus UnInitialize();
	DeviceStatus IsInitialized();

	DeviceStatus SetAddress();
	RadioProperties_t GetRadioProperties();

	void Send(void* msg, UINT16 size, bool request_ack = false, bool saveCopyOfPacket = false);
	void SendTS(void* msg, UINT16 size, UINT32 eventTime, bool request_ack, bool saveCopyOfPacket);

	void RequestSendAtTimeInstanst(void* msg, UINT16 size, TimeVariable_t PacketTransmissionTime, ClockIdentifier_t ClockIdentifier);
	void RequestCancelSend();

	void ChannelActivityDetection();

	RadioMode_t StartListenning();
	RadioMode_t Sleep();
	RadioMode_t Standby();
	RadioMode_t GetRadioState();
};

} /* namespace Samraksh_SX1276 */

#endif /* MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_SAMRAKSHSX1276HAL_H_ */
