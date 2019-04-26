#ifndef RADIO_H
#define RADIO_H

/* === INCLUDES ============================================================ */
#include <Tinyhal_types.h>
#include <tinyhal.h>
#include "Radio_decl.h"

//////////////////////////// Radio Errors/////////////////////////////////////////

#define RADIOERROR01  "Frame Buffer Overrun\n"
#define RADIOERROR02  "Battery Low\n"
#define RADIOERROR03  "State Change Failed\n"

//////////////////////////////////////////////////////////////////////////////////////

#define MAX_RADIOS_SUPPORTED 4
#define MAX_MACS_SUPPORTED 4

//////////////////////////// Default Event Handler Definitions //////////////////////////////////////////


extern "C"
{
	void* DefaultReceiveHandler(void *msg, UINT16 Size)
	{
		return NULL;
	}

	void DefaultSendAckHandler(void *msg, UINT16 Size, NetOpStatus status, UINT8 radioAckStatus)
	{

	}

	BOOL DefaultRadioInterruptHandler(RadioInterrupt Interrupt, void *param)
	{
		return FALSE;
	}
}


// The radio id being a static in the radio class has been removed

#if 0
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//All Radio objects needs to share the ID class to get a unique number
class RadioID {
private:
	static UINT8 UniqueRadioId;
public:
	static UINT8 GetUniqueRadioId(){
		return ++UniqueRadioId;
	}
};

UINT8 RadioID::UniqueRadioId=0;
#endif

// Base class definition for the radio driver interface for all radio device implementations
template<class T>
class Radio
{
	// Contains a unique id for each radio registered

	UINT8 RadioID;
	RadioAddress_t MyAddress;

	// Keeps track of all the mac ids that are currently supported
	static UINT8 MacIDs[MAX_MACS_SUPPORTED];
	static UINT8 MacIDIndex;

	RadioEventHandler defaultHandler;

	// Keeps a record of all the handlers registered by the MACs
	static RadioEventHandler* MacHandlers[MAX_MACS_SUPPORTED];

	// Type of radio (802.15.4 or 802.11 or ByteRadio or SoftwareDefined)
	RadioType Radio_Type;

	// Is the radio active
	BOOL Initialized;

public:
	// Set the mac handlers
	BOOL SetMacHandlers(RadioEventHandler* handler)
	{
		if(handler == NULL)
			return FALSE;

		MacHandlers[MacIDIndex++] = handler;
		return TRUE;
	}

	RadioEventHandler* GetMacHandler(UINT8 MacIndex)
	{
		return MacHandlers[MacIndex];
	}

	BOOL SetDefaultHandlers()
	{
		defaultHandler.SetReceiveHandler(DefaultReceiveHandler);
		defaultHandler.SetSendAckHandler(DefaultSendAckHandler);
		defaultHandler.SetRadioInterruptHandler(DefaultRadioInterruptHandler);
		MacHandlers[MacIDIndex] = &defaultHandler;
		return TRUE;
	}
	UINT16 GetAddress(){
		return (UINT16) MyAddress;
	}

	BOOL SetAddress(UINT16 address){
		MyAddress = (RadioAddress_t)address;
		return TRUE;
	}

	//virtual DeviceStatus Initialize(RadioEventHandler *event_handler, UINT8 mac_id)=0;

	// Responsible for initializing the radio
	//virtual DeviceStatus Initialize(RadioEventHandler *event_handler, UINT8 mac_id)
	DeviceStatus Initialize(RadioEventHandler *event_handler, UINT8 mac_id)
	{
		Radio<T>::MacIDs[MacIDIndex] = mac_id;
		if(!event_handler){
			SetDefaultHandlers();
		}
		else{
			MacHandlers[MacIDIndex] = event_handler;
		}

		// Increment the mac id index
		MacIDIndex++;

		return DS_Success;

	}

	// Responsible for uninitializing the radio
	//virtual DeviceStatus UnInitialize()
	DeviceStatus UnInitialize(UINT8 mac_id)
	{
	    DeviceStatus ret = DS_Fail;
	    // remove from list of IDs. what a PITA.
	    for(int itr=0; itr < MAX_MACS_SUPPORTED; ++itr) {
	        if(Radio<T>::MacIDs[itr] == mac_id) {

	            Radio<T>::MacIDs[itr] = 0;
	            for(int itr_inner=itr; itr_inner < MAX_MACS_SUPPORTED; ++itr_inner) {
	                Radio<T>::MacIDs[itr_inner] = Radio<T>::MacIDs[itr_inner + 1];
	            }
	            Radio<T>::MacIDs[MAX_MACS_SUPPORTED - 1] = 0;

	            MacHandlers[itr] = NULL;
	            for(int itr_inner=itr; itr_inner < MAX_MACS_SUPPORTED; ++itr_inner) {
	                MacHandlers[itr_inner] = MacHandlers[itr_inner + 1];
	            }
	            MacHandlers[MAX_MACS_SUPPORTED - 1] = NULL;

	            MacIDIndex--;

	            ret = DS_Success;
	            break;
	        }
	    }
	    return ret;
	}


	// Defines the send interface recieves an empty pointer from the radio layer
	//virtual T* Send(T* msg, UINT16 size)
	void* Send(void* msg, UINT16 size);



	// Defines the send interface recieves an empty pointer from the radio layer
	//virtual T* Send(T* msg, UINT16 size)
	void* Send_TimeStamped(void* msg, UINT16 size, UINT32 eventTime);


	// Preload the message on to the radio frame buffer/ RAM and dont send
	//virtual T* Preload(T* msg, UINT16 size)
	T* Preload(T* msg, UINT16 size);

	//virtual DeviceStatus ClearChannelAssesment()
	DeviceStatus ClearChannelAssesment();


	//virtual DeviceStatus ClearChannelAssesment(UINT32 numberMicroSecond)
	DeviceStatus ClearChannelAssesment(UINT32 numberMicroSecond);


	//virtual DeviceStatus SetTimeStamp()
	DeviceStatus SetTimeStamp();


	//virtual DeviceStatus SetTimeStamp(UINT64 timeStamp)
	DeviceStatus SetTimeStamp(UINT64 timeStamp);


	//virtual INT32 GetSNR()
	INT32 GetSNR();


	//virtual INT32 GetRSSI()
	INT32 GetRSSI();


	//virtual UINT8 GetRadioID()
	UINT8 GetRadioID(){
		return RadioID;
	}

	void SetRadioID(UINT8 id)
	{
		RadioID=id;
	}

	// Mutators for radio_type and is_active variables
	RadioType GetRadioType() const
	{
		return Radio_Type;
	}

	void SetRadioType(RadioType radio_type)
	{
		Radio_Type = radio_type;
	}

	BOOL IsInitialized() const
	{
		return Initialized;
	}

	void SetInitialized(BOOL initialize)
	{
		Initialized = initialize;
	}

	UINT8 GetMacIdIndex()
	{
		return MacIDIndex;
	}

};


class SamrakshRadio_I{
public:
	SamrakshRadio_I();
	virtual ~SamrakshRadio_I();
	/*!
	 * @brief Radio modes
	 */
	enum RadioMode_t{
		Uninitialized,
		SLEEP, 		//Low-power mode
		STANDBY,	//both Crystal oscillator and Lora baseband blocks are turned on.RF part and PLLs are disabled
		TX,			// TX
		RX,			// Listenning. When activated the SX1276/77/78/79 powers all remaining blocks required for reception, processing all received data until a new user request is made to change operating mode.
	};



	/*!
	 * @brief RadioProperties
	 */
	struct RadioProperties_t{
		UINT16 CUR_PHY_MODE; 			//A set of PHY layer settings forms a transmission mode. A number of such modes are defined at the radio driver and is settable by the above layer
		UINT16 NUMBER_OF_PHY_MODE;		//

		UINT16 RadioAddress; 				//Radio address. Defaults to some address during initialization. Can be settable via upper layers

		BOOL IS_HARDWARE_ACKS_SUPPORTED;	//Indicates whether or not radio supports hardware ACKS

		UINT16 MAXPacketSize; //Maximum packet size



		UINT16 RADIO_TURN_ON_DELAY_RX_MICRO;	//Delay transitioning into the RX from SLEEP mode

		UINT16 DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX; //Delay in detecting a packet in microseconds. Preamble + Header + Checking the address
		UINT16 DELAY_FROM_DTH_TX_TO_RADIO_DRIVER_TX; //Delay in transmitting a packet when tx is issued




		UINT8 TX_TIME_PER_BIT_IN_MICROSEC;
		UINT16 DELAY_IN_RECEIVING_HW_ACK;
		UINT16 CAD_DURATION_MICRO;
	};

	/*!
	 * @brief Radio driver callback functions
	 */
	struct RadioEvents_t
	{
	    /*!
	     * @brief  Tx Done callback prototype. for Send and InitiateTXPreloadedMsg
	     *
	     * @param [IN] success Whether tx was successfull
	     */
	    void    ( *TxDone )( bool success);
	    /*!
	     * @brief Packet detection callback prototype. A packet is detected with a valid preamble and a valid header.
	     */
	    void    ( *PacketDetected )( void );
	    /*!
	     * @brief Rx Done callback prototype. In case of rx error payload is null and the size is zero
	     *
	     * @param [IN] payload Received buffer pointer
	     * @param [IN] size    Received buffer size
	     */
	    void    ( *RxDone )( uint8_t *payload, uint16_t size );

	    /*!
	     * @brief CAD Done callback prototype.
	     *
	     * @param [IN] channelDetected    Whether Channel Activity detected during the CAD
	     */
	    void 	( *CadDone ) ( bool channelActivityDetected );


	    /*!
	     * @brief Callback prototype for sending. This is generated when the data gets accepted to be sent
	     *
	     * @param [IN] success	Wheter
	     * @param [IN] number_of_bytes_in_buffer
	     *
	     */
	    void    ( *DataStatusCallback )( bool success, UINT16 number_of_bytes_in_buffer );
//	    /*!
//	     * @brief Callback prototype for StartListenning, Sleep , Standby and GetRadioState
//	     *
//	     * @param [IN] cur_mode	Current radio mode
//	     *
//	     */
//		void    ( *RadioStateCallback )( RadioMode_t cur_mode );
//	    /*!
//	     * @brief Callback prototype for returning radio properties
//	     *
//	     * @param [IN] _rp	Radio properties
//	     *
//	     */
//		void  	( *RadioPropertiesCallback) (RadioProperties_t _rp);
	};

    /*!
     * @brief Initialization controls both chip initialization and the radio event handler.
     *
     */
	virtual DeviceStatus Initialize(RadioEvents_t re) = 0;
	virtual DeviceStatus UnInitialize() = 0;
    /*!
     * @brief Returns the status of the initialization
     *
     */
	virtual DeviceStatus IsInitialized();

	/*!
     * @brief Sets the radio address. Triggers a RadioPropertiesCallback.
     * 		If successful, returned value has the new address
     * 		otherwise, returned RadioProperties_t has the old address
     */
	virtual DeviceStatus SetAddress() = 0;
	/*!
     * @brief Request for
     */
	virtual RadioProperties_t GetRadioProperties() = 0;


    /*!
     * @brief Request Sending data as soon as possible
     *
     */
	virtual void Send(void* msg, UINT16 size, bool request_ack = false, bool saveCopyOfPacket=false);
    /*!
     * @brief Request Sending data at a specified time instance
     *
     */
	typedef UINT64 TimeVariable_t; //BK: These should be defined in the clock module
	typedef UINT8 ClockIdentifier_t;
	virtual void RequestSendAtTimeInstanst(void* msg, UINT16 size, TimeVariable_t PacketTransmissionTime, ClockIdentifier_t ClockIdentifier);
    /*!
     * @brief Cancels previous send request
     *
     */
	virtual void RequestCancelSend() = 0;


	/*!
     * @brief Performs a clear channel assesment
     */
	virtual void ChannelActivityDetection() = 0;

	/*!
     * @brief State change request
     */
	virtual RadioMode_t StartListenning() = 0;
	virtual RadioMode_t Sleep() = 0;
	virtual RadioMode_t Standby()= 0;
	virtual RadioMode_t GetRadioState()= 0;

};



template<class T>
UINT8 Radio<T>::MacIDIndex = 0;

template<class T>
UINT8 Radio<T>::MacIDs[MAX_MACS_SUPPORTED] = {0,0,0,0};

template<class T>
RadioEventHandler* Radio<T>::MacHandlers[MAX_MACS_SUPPORTED] = {NULL,NULL,NULL,NULL};
#endif /* RADIO_H */














