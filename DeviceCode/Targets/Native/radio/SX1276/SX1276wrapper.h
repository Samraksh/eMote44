/*
 * SX1276wrapper.h
 *
 *  Created on: Aug 10, 2017
 *      Author: Bora
 */

#ifndef MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_SX1276WRAPPER_H_
#define MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_SX1276WRAPPER_H_


//#include "EmoteLoraHat.h"
#include <tinyhal.h>
#include <stm32f10x.h>
#include <spi\netmf_spi.h>

//#include "sx1276driver/sx1276/sx1276.h"
//#include "sx1276driver/radio/radio.h"
#include "sx1276driver/registers/sx1276Regs-Fsk.h"
#include "sx1276driver/registers/sx1276Regs-LoRa.h"
#include "sx1276driver/typedefs/typedefs.h"

#include "sx1276wrapper_definitions.h"


//namespace SX1276_Semtech{
/*!
 * @brief Radio driver callback functions
 */
struct SX1276RadioEvents_t{
	 void    ( *ValidHeaderDetected )( void );
    /*!
     * @brief  Tx Done callback prototype.
     */
    void    ( *TxDone )( void );
    /*!
     * @brief  Tx Timeout callback prototype.
     */
    void    ( *TxTimeout )( void );
    /*!
     * @brief Rx Done callback prototype.
     *
     * @param [IN] payload Received buffer pointer
     * @param [IN] size    Received buffer size
     * @param [IN] rssi    RSSI value computed while receiving the frame [dBm]
     * @param [IN] snr     Raw SNR value given by the radio hardware
     *                     FSK : N/A ( set to 0 )
     *                     LoRa: SNR value in dB
     */
    void    ( *RxDone )( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
    /*!
     * @brief  Rx Timeout callback prototype.
     */
    void    ( *RxTimeout )( void );
    /*!
     * @brief Rx Error callback prototype.
     */
    void    ( *RxError )( void );
    /*!
     * \brief  FHSS Change Channel callback prototype.
     *
     * \param [IN] currentChannel   Index number of the current channel
     */
    void ( *FhssChangeChannel )( uint8_t currentChannel );
    /*!
     * @brief CAD Done callback prototype.
     *
     * @param [IN] channelDetected    Channel Activity detected during the CAD
     */
    void ( *CadDone ) ( bool channelActivityDetected );
};

//namespace SX1276_Semtech {
class SX1276M1BxASWrapper;

typedef void ( SX1276M1BxASWrapper::*DioIrqHandler )( void );

/*!
 * Radio wake-up time from sleep
 */
#define RADIO_WAKEUP_TIME                           1 // [ms]

/*!
 * Sync word for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x12

/*!
 * Sync word for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x34


/*!
 * SX1276 definitions
 */
#define XTAL_FREQ                                   32000000
#define FREQ_STEP                                   61.03515625

#define RX_BUFFER_SIZE                              256

/*!
 * Constant values need to compute the RSSI value
 */
#define RSSI_OFFSET_LF                              -164.0
#define RSSI_OFFSET_HF                              -157.0

#define RF_MID_BAND_THRESH                          525000000



#define SX1276_CMD_ID_READ_RX_FIFO 0x77
#define SX1276_CMD_ID_WRITE_TX_FIFO 0x66


static int LORA_ASSERT(int x, const char *err) {
//	if (!x) {
//		si446x_debug_print(SX1276ERR100, "%s", err);
//		radio_error();
//		return 1;
//	}
//	else return 0;
	if(!x) return 1;
	else return 0;
}


//namespace SX1276_Semtech {

enum {
	SX1276NONE00=0,
	SX1276DEBUG01=1,
	SX1276DEBUG02=2,
	SX1276DEBUG03=3,
	SX1276ERR99=99,		// For things that are bad unless you know what you are doing.
	SX1276ERR100=100,
};
enum { CTS_TIMEOUT = 262143*2, CTS_VAL_GOOD=0xFF, CTS_WAIT=255 };
enum { VERB0=0, VERB1=1, ERR0=128, ERR1=256 };
enum { ROMC2A=6, ROMB1B=3 };

const unsigned si4468x_debug_level = SX1276ERR100; // CHANGE ME.



class SX1276M1BxASWrapper {
public:
	enum LoraDaugterBoards{
		LORA_GPS_HAT,
	};

	struct SX1276_pin_setup_t{
//        PinName mosi;		//mosi_pin
//		PinName miso;  		//miso_pin
//		PinName sclk;		//sclk_pin
//		PinName nss;
//		PinName reset;
//        PinName dio0;
//		PinName dio1;
//		PinName dio2;
//		PinName dio3;
//		PinName dio4;
//		PinName dio5;


		GPIO_TypeDef 	*nirq_port0;
		uint16_t		nirq_pin0;
		GPIO_PIN		nirq_mf_pin0;
		GPIO_TypeDef 	*nirq_port1;
		uint16_t		nirq_pin1;
		GPIO_PIN		nirq_mf_pin1;
		GPIO_TypeDef 	*nirq_port2;
		uint16_t		nirq_pin2;
		GPIO_PIN		nirq_mf_pin2;
		GPIO_TypeDef 	*nirq_port3;
		uint16_t		nirq_pin3;
		GPIO_PIN		nirq_mf_pin3;
		GPIO_TypeDef 	*nirq_port4;
		uint16_t		nirq_pin4;
		GPIO_PIN		nirq_mf_pin4;
		GPIO_TypeDef 	*nirq_port5;
		uint16_t		nirq_pin5;
		GPIO_PIN		nirq_mf_pin5;


		GPIO_TypeDef 	*reset_port;
		uint16_t		reset_pin;
		GPIO_PIN		reset_mf_pin;

		SPI_TypeDef 	*spi_base;
		GPIO_TypeDef 	*spi_port;
		uint32_t		spi_rcc;
		GPIO_TypeDef 	*cs_port;
		uint16_t		cs_pin;
		uint16_t		sclk_pin;
		uint16_t		miso_pin;
		uint16_t		mosi_pin;

	};

	struct InterruptPins_t{
		UINT8 DIO0;
		UINT8 DIO1;
		UINT8 DIO2;
		UINT8 DIO3;
		UINT8 DIO4;
		UINT8 DIO5;
	};

//protected:
	// Stores the configuration of the spi
//	SPI_CONFIGURATION config;
	SPI_CONFIGURATION m_spi_config;
	SX1276_pin_setup_t SX1276_pin_setup;
	static unsigned ctsWentHigh;

	uint8_t radio_spi_go(uint8_t data);
	void radio_spi_sel_no_assert();

//private:
	void initSPI2(); //Initialize a SPI structure
	void init_pins();
	void reset();
	void init_interrupts();
	SPI_XACTION_8 PrepareAddrSPI_XACTION(const SPI_CONFIGURATION& config, UINT8* addr_ptr);
public:
	InterruptPins_t SX1276_interupt_pins;



//	LoraHardwareConfig();

	void LoraHardwareConfigInitialize();

	void spi_write_bytes(unsigned count, const uint8_t *buf);

	void spi_read_bytes(unsigned count, uint8_t *buf);

	unsigned int radio_comm_PollCTS();

	void radio_spi_sel_assert();

	uint8_t radio_comm_GetResp(uint8_t byteCount, uint8_t* pData);

	//check and read packet in lora modem
	bool LoraRcvPkt();
public:
//	Emote_Lora_Hat();
//	~Emote_Lora_Hat();
////
//	void write_tx_fifo(uint8_t numBytes, uint8_t* pTxData) {
//	  radio_comm_WriteData( SX1276_CMD_ID_WRITE_TX_FIFO, 0, numBytes, pTxData );
//	}
//
//	void read_rx_fifo(uint8_t numBytes, uint8_t* pRxData) {
//	  radio_comm_ReadData( SX1276_CMD_ID_READ_RX_FIFO, 0, numBytes, pRxData );
//	}



	bool SpiInitialize();

	void radio_comm_WriteData(uint8_t cmd, unsigned pollCts, uint8_t byteCount, uint8_t* pData);
	void radio_comm_ReadData(uint8_t cmd, unsigned pollCts, uint8_t byteCount, uint8_t* pData);



//protected:
    SX1276RadioEvents_t* RadioEvents;

//protected:
    /*!
    * SPI Interface
    */
//    SPI spi; // mosi, miso, sclk
//    DigitalOut nss;
//
//    /*!
//     * SX1276 Reset pin
//     */
//    DigitalInOut reset;

    /*!
     * SX1276 DIO pins
     */
//    InterruptIn dio0;
//    InterruptIn dio1;
//    InterruptIn dio2;
//    InterruptIn dio3;
//    InterruptIn dio4;
//    DigitalIn dio5;

    bool isRadioActive;

    uint8_t boardConnected; //1 = SX1276MB1LAS; 0 = SX1276MB1MAS

    const uint8_t rxtxBufferSize = 256;
    uint8_t rxtxBufferstorage[256];
    uint8_t *rxtxBuffer;

    /*!
     * Hardware DIO IRQ functions
     */
//    DioIrqHandler *dioIrq;
//    DioIrqHandler dioIrq[6];

    /*!
     * Tx and Rx timers
     */
//    Timeout txTimeoutTimer;
//    Timeout rxTimeoutTimer;
//    Timeout rxTimeoutSyncWord;
//BK: Use SetTimer The regex for find replace
//    (..TimeoutTimer)(.attach_us[^,]*,[^,]*,)([^)]*)([^;];)      //\1\2\3\4;\R SetTimeoutTimer(\1,\3);
//    (..Timeout[^;\.]*)(.detach[^;]*;)              //\1\2 \R CancelTimeoutTimer(\1);
    enum TimeoutName_t{
    	txTimeoutTimer,
		rxTimeoutTimer,
		rxTimeoutSyncWord,
    };

    RadioSettings_t settings;

    static const FskBandwidth_t FskBandwidths[];
//protected:

    /*!
    * Performs the Rx chain calibration for LF and HF bands
    * \remark Must be called just after the reset so all registers are at their
    *         default values
    */
    void RxChainCalibration( );

public:

    //-------------------------------------------------------------------------
    //                        Redefined Radio functions
    //-------------------------------------------------------------------------
    /*!
     * @brief Initializes the radio
     *
     * @param [IN] events Structure containing the driver callback functions
     */
    void Init( SX1276RadioEvents_t* events );
    /*!
     * Return current radio status
     *
     * @param status Radio status. [RF_IDLE, RX_RUNNING, TX_RUNNING]
     */
    RadioState GetStatus( );
    /*!
     * @brief Configures the SX1276 with the given modem
     *
     * @param [IN] modem Modem to be used [0: FSK, 1: LoRa]
     */
    void SetModem( RadioModems_t modem );
    /*!
     * @brief Sets the channel frequency
     *
     * @param [IN] freq         Channel RF frequency
     */
    void SetChannel( uint32_t freq );
    /*!
     * @brief Sets the channels configuration
     *
     * @param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] freq       Channel RF frequency
     * @param [IN] rssiThresh RSSI threshold
     *
     * @retval isFree         [true: Channel is free, false: Channel is not free]
     */
    bool IsChannelFree( RadioModems_t modem, uint32_t freq, int16_t rssiThresh );
    /*!
     * @brief Generates a 32 bits random value based on the RSSI readings
     *
     * \remark This function sets the radio in LoRa modem mode and disables
     *         all interrupts.
     *         After calling this function either Radio.SetRxConfig or
     *         Radio.SetTxConfig functions must be called.
     *
     * @retval randomValue    32 bits random value
     */
    uint32_t Random(  );
    /*!
     * @brief Sets the reception parameters
     *
     * @param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] bandwidth    Sets the bandwidth
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     * @param [IN] datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * @param [IN] coderate     Sets the coding rate ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * @param [IN] bandwidthAfc Sets the AFC Bandwidth ( FSK only )
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: N/A ( set to 0 )
     * @param [IN] preambleLen  Sets the Preamble length ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: Length in symbols ( the hardware adds 4 more symbols )
     * @param [IN] symbTimeout  Sets the RxSingle timeout value
     *                          FSK : timeout number of bytes
     *                          LoRa: timeout in symbols
     * @param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
     * @param [IN] payloadLen   Sets payload length when fixed lenght is used
     * @param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
     * @param [IN] freqHopOn    Enables disables the intra-packet frequency hopping  [0: OFF, 1: ON] (LoRa only)
     * @param [IN] hopPeriod    Number of symbols bewteen each hop (LoRa only)
     * @param [IN] iqInverted   Inverts IQ signals ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     * @param [IN] rxContinuous Sets the reception in continuous mode
     *                          [false: single mode, true: continuous mode]
     */
    void SetRxConfig ( RadioModems_t modem, uint32_t bandwidth,
                               uint32_t datarate, uint8_t coderate,
                               uint32_t bandwidthAfc, uint16_t preambleLen,
                               uint16_t symbTimeout, bool fixLen,
                               uint8_t payloadLen,
                               bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                               bool iqInverted, bool rxContinuous );
    /*!
     * @brief Sets the transmission parameters
     *
     * @param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] power        Sets the output power [dBm]
     * @param [IN] fdev         Sets the frequency deviation ( FSK only )
     *                          FSK : [Hz]
     *                          LoRa: 0
     * @param [IN] bandwidth    Sets the bandwidth ( LoRa only )
     *                          FSK : 0
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     * @param [IN] datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * @param [IN] coderate     Sets the coding rate ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * @param [IN] preambleLen  Sets the preamble length
     * @param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
     * @param [IN] crcOn        Enables disables the CRC [0: OFF, 1: ON]
     * @param [IN] freqHopOn    Enables disables the intra-packet frequency hopping  [0: OFF, 1: ON] (LoRa only)
     * @param [IN] hopPeriod    Number of symbols bewteen each hop (LoRa only)
     * @param [IN] iqInverted   Inverts IQ signals ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     * @param [IN] timeout      Transmission timeout [ms]
     */
    void SetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
                              uint32_t bandwidth, uint32_t datarate,
                              uint8_t coderate, uint16_t preambleLen,
                              bool fixLen, bool crcOn, bool freqHopOn,
                              uint8_t hopPeriod, bool iqInverted, uint32_t timeout );
    /*!
     * @brief Computes the packet time on air for the given payload
     *
     * \Remark Can only be called once SetRxConfig or SetTxConfig have been called
     *
     * @param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] pktLen     Packet payload length
     *
     * @retval airTime        Computed airTime for the given packet payload length
     */
    uint32_t TimeOnAir ( RadioModems_t modem, uint8_t pktLen );
    /*!
     * @brief Sends the buffer of size. Prepares the packet to be sent and sets
     *        the radio in transmission
     *
     * @param [IN]: buffer     Buffer pointer
     * @param [IN]: size       Buffer size
     */
    bool Send( uint8_t *buffer, uint8_t size );
    bool SendTS( uint8_t *buffer, uint8_t size ,  UINT32 eventTime);//BK:Artificial injection due to compliance with mf_radio. TODO:The interface should be changed as it breaks layering.
    /*!
     * @brief Sets the radio in sleep mode
     */
    void Sleep(  );
    /*!
     * @brief Sets the radio in standby mode
     */
    void Standby(  );
    /*!
     * @brief Sets the radio in CAD mode
     */
    void StartCad(  );
    /*!
     * @brief Sets the radio in reception mode for the given time
     * @param [IN] timeout Reception timeout [ms]
     *                     [0: continuous, others timeout]
     */
    void Rx( uint32_t timeout );
    /*!
     * @brief Sets the radio in transmission mode for the given time
     * @param [IN] timeout Transmission timeout [ms]
     *                     [0: continuous, others timeout]
     */
    void Tx( uint32_t timeout );
    /*!
     * @brief Sets the radio in continuous wave transmission mode
     *
     * @param [IN]: freq       Channel RF frequency
     * @param [IN]: power      Sets the output power [dBm]
     * @param [IN]: time       Transmission mode timeout [s]
     */
    void SetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time );
    /*!
     * @brief Reads the current RSSI value
     *
     * @retval rssiValue Current RSSI value in [dBm]
     */
    int16_t GetRssi ( RadioModems_t modem );

    /*!
     * @brief Sets the maximum payload length.
     *
     * @param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] max        Maximum payload length in bytes
     */
    void SetMaxPayloadLength( RadioModems_t modem, uint8_t max );

    /*!
     * \brief Sets the network to public or //private. Updates the sync byte.
     *
     * \remark Applies to LoRa modem only
     *
     * \param [IN] enable if true, it enables a public network
     */
    void SetPublicNetwork( bool enable );

    //-------------------------------------------------------------------------
    //                        Board relative functions
    //-------------------------------------------------------------------------

//protected:
    //wait for milliseconds
    void wait_ms(UINT32 x);
//protected:

    /*!
     * @brief Sets the SX1276 operating mode
     *
     * @param [IN] opMode New operating mode
     */
    void SetOpMode( uint8_t opMode );

    /*
     * SX1276 DIO IRQ callback functions prototype
     */

    /*!
     * @brief DIO 0 IRQ callback
     */
    void OnDio0Irq(  );

    /*!
     * @brief DIO 1 IRQ callback
     */
    void OnDio1Irq(  );

    /*!
     * @brief DIO 2 IRQ callback
     */
    void OnDio2Irq(  );

    /*!
     * @brief DIO 3 IRQ callback
     */
    void OnDio3Irq(  );

    /*!
     * @brief DIO 4 IRQ callback
     */
    void OnDio4Irq(  );

    /*!
     * @brief DIO 5 IRQ callback
     */
    void OnDio5Irq(  );

    /*!
     * @brief Tx & Rx timeout timer callback
     */
    void OnTimeoutIrq(  );

    /*!
     * Returns the known FSK bandwidth registers value
     *
     * \param [IN] bandwidth Bandwidth value in Hz
     * \retval regValue Bandwidth register value.
     */
    static uint8_t GetFskBandwidthRegValue( uint32_t bandwidth );

public :
	static void SX1276_Radio_Interrupt_Handler0(GPIO_PIN Pin, BOOL PinState, void* Param);
	static void SX1276_Radio_Interrupt_Handler1(GPIO_PIN Pin, BOOL PinState, void* Param);
	static void SX1276_Radio_Interrupt_Handler2(GPIO_PIN Pin, BOOL PinState, void* Param);
	static void SX1276_Radio_Interrupt_Handler3(GPIO_PIN Pin, BOOL PinState, void* Param);
	static void SX1276_Radio_Interrupt_Handler4(GPIO_PIN Pin, BOOL PinState, void* Param);
	static void SX1276_Radio_Interrupt_Handler5(GPIO_PIN Pin, BOOL PinState, void* Param);

	static void SX1276_Radio_OnTimeoutIrq(void* param);

	static void SX1276_Reset_Pin_Interrupt_Handler(GPIO_PIN Pin, BOOL PinState, void* Param);

//private:
	RadioRegisters_t RadioRegsInit[16];
public:
	bool reset_intiated;
	SX1276M1BxASWrapper();
	void Initialize(SX1276RadioEvents_t *events);
//	SX1276M1BxASWrapper(SX1276RadioEvents_t *events );
	~SX1276M1BxASWrapper();

	void SetOpMode_public( uint8_t opMode );

    /*!
     * @brief Checks if the given RF frequency is supported by the hardware
     *
     * @param [IN] frequency RF frequency to be checked
     * @retval isSupported [true: supported, false: unsupported]
     */
	bool CheckRfFrequency( uint32_t frequency );
    /*!
     * @brief Writes the radio register at the specified address
     *
     * @param [IN]: addr Register address
     * @param [IN]: data New register value
     */
    void Write ( uint8_t addr, uint8_t data );
    /*!
     * @brief Reads the radio register at the specified address
     *
     * @param [IN]: addr Register address
     * @retval data Register value
     */
    uint8_t Read ( uint8_t addr );
    /*!
     * @brief Writes multiple radio registers starting at address
     *
     * @param [IN] addr   First Radio register address
     * @param [IN] buffer Buffer containing the new register's values
     * @param [IN] size   Number of registers to be written
     */
    void Write( uint8_t addr, uint8_t *buffer, uint8_t size );
    /*!
     * @brief Reads multiple radio registers starting at address
     *
     * @param [IN] addr First Radio register address
     * @param [OUT] buffer Buffer where to copy the registers data
     * @param [IN] size Number of registers to be read
     */
    void Read ( uint8_t addr, uint8_t *buffer, uint8_t size );
    /*!
     * @brief Writes the buffer contents to the SX1276 FIFO
     *
     * @param [IN] buffer Buffer containing data to be put on the FIFO.
     * @param [IN] size Number of bytes to be written to the FIFO
     */
    void WriteFifo( uint8_t *buffer, uint8_t size );
    /*!
     * @brief Reads the contents of the SX1276 FIFO
     *
     * @param [OUT] buffer Buffer where to copy the FIFO read data.
     * @param [IN] size Number of bytes to be read from the FIFO
     */
    void ReadFifo( uint8_t *buffer, uint8_t size );
    /*!
     * @brief Resets the SX1276
     */
    void Reset(  );

    friend class Samraksh_SX1276_hal;

//protected:
    /*!
     * @brief Initializes the radio I/Os pins interface
     */
    void IoInit( );

    void InitializeTimers();

    /*!
     *    @brief Initializes the radio registers
     */
    void RadioRegistersInit( );

    /*!
     * @brief Initializes the radio SPI
     */
    void SpiInit(  );

    /*!
     * @brief Initializes DIO IRQ handlers
     *
     * @param [IN] irqHandlers Array containing the IRQ callback functions
     */
    void IoIrqInit( );

    /*!
     * @brief De-initializes the radio I/Os pins interface.
     *
     * \remark Useful when going in MCU lowpower modes
     */
    void IoDeInit(  );

    /*!
     * @brief Sets the radio output power.
     *
     * @param [IN] power Sets the RF output power
     */
    void SetRfTxPower( int8_t power );

    /*!
     * @brief Gets the board PA selection configuration
     *
     * @param [IN] channel Channel frequency in Hz
     * @retval PaSelect RegPaConfig PaSelect value
     */
    uint8_t GetPaSelect( uint32_t channel );

    /*!
     * @brief Set the RF Switch I/Os pins in Low Power mode
     *
     * @param [IN] status enable or disable
     */
    void SetAntSwLowPower( bool status );

    /*!
     * @brief Initializes the RF Switch I/Os pins interface
     */
    void AntSwInit(  );

    /*!
     * @brief De-initializes the RF Switch I/Os pins interface
     *
     * \remark Needed to decrease the power consumption in MCU lowpower modes
     */
    void AntSwDeInit(  );

    /*!
     * @brief Controls the antenna switch if necessary.
     *
     * \remark see errata note
     *
     * @param [IN] opMode Current radio operating mode
     */
    void SetAntSw( uint8_t opMode );


    void SetTimeoutTimer(TimeoutName_t ton, float delay) ;
    void CancelTimeoutTimer(TimeoutName_t ton);


//private:
    uint8_t GetTimerID(TimeoutName_t ton) ;




};

//} //namespace SX1276_Semtech


#endif /* MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_SX1276WRAPPER_H_ */
