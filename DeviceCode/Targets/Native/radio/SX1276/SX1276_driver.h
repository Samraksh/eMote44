#include <Samraksh/Radio.h>
#include <Samraksh/VirtualTimer.h>
#include "parameters.h"

#define MAX_PACKET_SIZE 255

typedef enum RadioMode_t{
	Uninitialized,
	SLEEP, 		//Low-power mode
	STANDBY,	//both Crystal oscillator and Lora baseband blocks are turned on.RF part and PLLs are disabled
	TX,			// TX
	RX,			// Listenning. When activated the SX1276/77/78/79 powers all remaining blocks required for reception, processing all received data until a new user request is made to change operating mode.
};

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

typedef UINT64 TimeVariable_t;
typedef UINT8 ClockIdentifier_t;

struct msgToBeTransmitted_t {
	UINT8 msg_payload[MAX_PACKET_SIZE];
	UINT16 msg_size;
	UINT64 due_time;
	ClockIdentifier_t clock_id;
	bool isUploaded;
};


	
void SX1276_HAL_ValidHeaderDetected();
void SX1276_HAL_TxDone();
void SX1276_HAL_TxTimeout();
void SX1276_HAL_RxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void SX1276_HAL_RxTimeout();
void SX1276_HAL_RxError();
void SX1276_HAL_FhssChangeChannel(uint8_t currentChannel );
void SX1276_HAL_CadDone(bool channelActivityDetected);
DeviceStatus SX1276_HAL_Initialize(SamrakshRadio_I::RadioEvents_t re);
DeviceStatus SX1276_HAL_UnInitialize();

void* SX1276_HAL_Send(void* msg, UINT16 size, UINT32 eventTime, bool request_ack, bool saveCopyOfPacket);
void SX1276_HAL_RequestSendAtTimeInstants(void* msg, UINT16 size, TimeVariable_t PacketTransmissionTime, ClockIdentifier_t ClockIdentifier);
void SX1276_HAL_RequestCancelSend();
DeviceStatus SX1276_HAL_AddToTxBuffer(void* msg, UINT16 size);
DeviceStatus SX1276_HAL_ChannelActivityDetection();

void SX1276_HAL_PacketLoadTimerHandler(void* param);
void SX1276_HAL_PacketTxTimerHandler(void* param);
void SX1276_HAL_CADTimerHandler(void * param);

bool SX1276_HAL_SetTimer(UINT8 timer_id, UINT32 start_delay, UINT32 period, BOOL is_one_shot, UINT8 hardwareTimerId);
bool SX1276_HAL_IsPacketTransmittable(void* msg, UINT16 size);
DeviceStatus SX1276_HAL_TurnOnRx();
DeviceStatus SX1276_HAL_Sleep();
DeviceStatus SX1276_HAL_Standby();
void SX1276_HAL_ChooseRadioConfig();
RadioMode_t SX1276_HAL_GetRadioState();

UINT16 SX1276_HAL_GetAddress();
BOOL SX1276_HAL_SetAddress(UINT16 address);
INT8 SX1276_HAL_GetRadioName();
void SX1276_HAL_SetRadioName(INT8 rn);

bool SX1276_Packet_PreparePayload(void* msg, UINT16 size, const UINT64& t, ClockIdentifier_t c);
void SX1276_Packet_ClearPayload();
void SX1276_Packet_MarkUploaded();
bool SX1276_Packet_IsMsgSaved();
bool SX1276_Packet_IsMsgUploaded();
UINT16 SX1276_Packet_GetSize();
UINT8* SX1276_Packet_GetPayload();
UINT64 SX1276_Packet_GetDueTime();
ClockIdentifier_t SX1276_Packet_GetClockId();

