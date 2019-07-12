/*
 *
 * 	File Name   :		RF231.h
 *
 *  Author Name :   	Nived.Sivadas@samraksh.com
						Nathan.Stohs@samraksh.com
 *
 *  Description :    	Contains the core class implementation of the rf231 radio derived from the radio base class
 *
 *
 *
 *
 */

#ifndef _RF231_H_
#define _RF231_H_

/* === INCLUDES ============================================================ */
#include <Tinyhal_types.h>
#include <tinyhal.h>
#include <Samraksh\Message.h>
#include <Samraksh\Radio.h>
//#include <spi\netmf_spi.h>
//#include <gpio\netmf_gpio.h>
#include <CPU_SPI_decl.h>
#include <CPU_GPIO_decl.h>

/* === VARS ================================================================ */

class RF231Radio;
// Declared in RF231.cpp
extern RF231Radio grf231Radio;
extern RF231Radio grf231RadioLR;

/* === FUNCS =============================================================== */

BOOL GetCPUSerial(UINT8 * ptr, UINT16 num_of_bytes);
void Radio_Handler(GPIO_PIN Pin, BOOL PinState, void* Param);
void Radio_Handler_LR(GPIO_PIN Pin,BOOL PinState, void* Param);

/* === MACROS ============================================================== */

#define  SELN_PIN      102 // Rev 1 :  26
#define	 RSTN_PIN      66  // Rev 1 :  27
#define  SLP_TR_PIN    104 // Rev 1 :  18
#define  INTERRUPT_PIN 65  // Rev 1 :  17

#define SELN_PIN_LR		89
#define RSTN_PIN_LR		90
#define SLP_TR_PIN_LR   27
#define AMP_PIN_LR		(GPIO_PIN) 26
#define INTERRUPT_PIN_LR 55

#define	 RF231_TRX_CTRL_0_VALUE		 0x0
//Bit 7 - PA_EXT_EN - 0
//Bit 6 - IRQ_2_EXT_EN - 0
//Bit 5 - TX_AUTO_CRC_ON - 1
//Bit 4 - RX_BL_CTRL - 0
//Bit [3:2] - SPI_CMD_MODE - 00
//Bit 1 - IRQ_MASK_MODE - 1
//Bit 0 - IRQ_POLARITY - 0
//Register TRX_CTRL_1 is 0x04
//0010 0000
#define	 RF231_TRX_CTRL_1_VALUE		 0x20
#define  RF230_CCA_THRES_VALUE 	 	 0xC7
#define	 RF230_CCA_MODE_VALUE  		 (1 << 5)

//Configuration based on page 54
//
//Page 154 in RF231 datasheet
//Bit 7 - RX_SAFE_MODE	- If set, dynamic frame buffer protection is enabled (1)
//Bit [6:2] - Reserved 	- 00000
//Bit [1:0] - OQPSK_DATA_RATE - Page 137, 140 - 00 is for 250 kb/s
//Register TRX_CTRL_2 is 0x0C
//1000 0000
#define RF231_TRX_CTRL_2_VALUE	0x80
//Page 71-72 in RF231 datasheet
//Bits 0,3,6,7 - Reserved
//Bit 0 - Reserved (0)
//Bit 1 - AACK_PROM_MODE 	- Promiscuous mode (enabled since we cannot filter on addresses) (0)
//Bit 2 - AACK_ACK_TIME 	- Auto ack time (ack set to transmit 12 symbols after reception of last symbol of a frame)
//								if set, then ack is sent 2 symbol periods later. (1)
//Bit 3 - Reserved (0)
//Bit 4 - AACK_UPLD_RES_FT 	- If set, received frames indicated as a reserved frame are further processed (0)
//								An IRQ_3 (TRX_END) interrupt is generated if the FCS is valid
//Bit 5 - AACK_FLTR_RES_FT 	- If set, reserved frame types are filtered similar to data frames as specified in IEEE 802.15.4-2006. (0)
//		   Can be set only if bit 4 is set.
//Bit [7:6] - Reserved (00)
//Register XAH_CTRL_1 is 0x17
//0000 0100
#define	 RF231_XAH_CTRL_1_VALUE		 0x04
//Page 73 in RF231 datasheet
//Bits 7-4 - MAX_FRAME_RETRIES - being set to 0 (0000)
//Bits 3-1 - MAX_CSMA_RETRIES - being set to 7 (111)
//Bit 0 - SLOTTED_OPERATION - set to 0
//Register XAH_CTRL_1 is 0x2C
//0000 1110
#define	 RF231_XAH_CTRL_0_VALUE		 0x0E
//Page 74-75 in the RF231 datasheet
//Bit [7:6] - AACK_FVN_MODE		- Mode is set to 3 (Acknowledge independent of frame version number) (11) (to be changed to 00 later)
//									RX_AACK register bit AACK_FVN_MODE (register 0x2E, CSMA_SEED_1) controls the behavior
//									of frame acknowledgements. This register determines if, depending on the Frame Version
//									Number, a frame is acknowledged or not. This is necessary for backward compatibility to IEEE
//									802.15.4-2003 and for future use. Even if frame version numbers 2 and 3 are reserved, it can be
//									handled by the radio transceiver.
//								  If AACK_FVN_MODE is 2 and frame version in FCF is 2 - No AMI is generated
//								  If AACK_FVN_MODE is 2 and frame version in FCF is 3 - AMI is generated
//
//Bit 5 	- AACK_SET_PD 		- Content is copied into the frame pending subfield of the acknowledgement frame if ack
//									is the answer to a data request MAC command frame (0)
//Bit 4 	- AACK_DIS_ACK 		- If this bit is set no ack frames are transmitted (0)
//Bit 3 	- AACK_I_AM_COORD 	- This has to be set if the node is a PAN coordinator (0)
//Bit [2:0]	- CSMA_SEED_1 		- Higher 3 bit of the CSMA_SEED (010)
//Register CSMA_SEED_1 is 0x2E
//0100 0010 (0x82, 0xC2 work)
#define RF231_CSMA_SEED_1_VALUE		0x42
//Page 73-74
//The seed for random value for CSMA-CA backoff is 010 1010 1010
//The higher 3 bits (010) is set in bits 2:0 in CSMA_SEED_1
//The lower 8 bits are set in CSMA_SEED_0
//Register CSMA_SEED_0 is 0x2D
//1010 1010
#define RF231_CSMA_SEED_0_VALUE		0xAA
//Page 75 (default value)
//Register CSMA_BE is 0x2F
//0101 0011
#define RF231_CSMA_BE_VALUE		0x53

#define TRAC_STATUS_MASK	0xE0


#define RF230_DEF_CHANNEL 26

#define NODE_ID 0x1
#define WRITE_ACCESS_COMMAND            (0xC0)//Write access command to the tranceiver
#define READ_ACCESS_COMMAND             (0x80)//Read access command to the tranceiver

#define INIT_STATE_CHECK()				volatile UINT16 poll_counter, trx_status;

#define VERIFY_STATE_CHANGE				ReadRegister(RF230_TRX_STATUS) & RF230_TRX_STATUS_MASK

// Simple assert function that prints a message and infinite whiles
#if !defined(NDEBUG)
#define ASSERT_RADIO(x)  if(!(x)){ SOFT_BREAKPOINT(); }
#else
#define ASSERT_RADIO(x)
#endif

#define DID_STATE_CHANGE(x)				poll_counter = 0;               \
										do{ 							\
											trx_status = (radio_hal_trx_status_t)(ReadRegister(RF230_TRX_STATUS) & RF230_TRX_STATUS_MASK);		\
											if(poll_counter == 0xfff)    \
											{  								\
												hal_printf(RADIOERROR03);  \
												return DS_Fail; 				\
											} 								\
											poll_counter++; 				\
										  }while(trx_status != x);							\


#define DID_STATE_CHANGE_NULL(x)		poll_counter = 0;               \
										do{ 							\
											trx_status = (radio_hal_trx_status_t)(ReadRegister(RF230_TRX_STATUS) & RF230_TRX_STATUS_MASK);		\
											if(poll_counter == 0xfff)    \
											{  								\
												hal_printf(RADIOERROR03);  \
												return NULL; 				\
											} 								\
											poll_counter++; 				\
										  }while(trx_status != x);							\


#define DID_STATE_CHANGE_ASSERT(x)		poll_counter = 0;               \
										do{ 							\
											trx_status = (radio_hal_trx_status_t)(ReadRegister(RF230_TRX_STATUS) & RF230_TRX_STATUS_MASK);		\
											if(poll_counter == 0xfff)    \
											{  								\
												SOFT_BREAKPOINT();          \
												break;                      \
											} 								\
											poll_counter++; 				\
										  }while(trx_status != x);

#define RF231_REG_TX_CTRL_1		0x04
#define RF231_REG_ANT_DIV		0x0D

//for PA_BRD
#define RF231_REG_TX_CTRL_1		0x04

#define RF231_SPI SPI2
#define RF231_SEL(x) { if(x) GPIO_WriteBit(GPIOF, GPIO_Pin_9, Bit_SET); else GPIO_WriteBit(GPIOF, GPIO_Pin_9, Bit_RESET); }
#define RF231_RST(x) { if(x) GPIO_WriteBit(GPIOF, GPIO_Pin_10, Bit_SET); else GPIO_WriteBit(GPIOF, GPIO_Pin_10, Bit_RESET); }
#define RF231_SLP(x) { if(x) GPIO_WriteBit(GPIOB, GPIO_Pin_11, Bit_SET); else GPIO_WriteBit(GPIOB, GPIO_Pin_11, Bit_RESET); }
#define RF231_AMP(x) { if(x) GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_SET); else GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_RESET); }

#define ENABLE_LRR(X) if(RF231RADIOLR == this->GetRadioName()) \
			{	\
				this->Amp(X); \
				this->PARXTX(X); \
				this->AntDiversity(X); \
			}

typedef UINT8 RadioStateType;
typedef UINT8 RadioCommandType;

/** Transceiver interrupt reasons */
typedef enum radio_irq_reason
{
	TRX_NO_IRQ                      = (0x00),
	TRX_IRQ_4                       = (0x10),
	TRX_IRQ_5                       = (0x20),
	TRX_IRQ_PLL_LOCK                = (0x01),
	TRX_IRQ_PLL_UNLOCK              = (0x02),
	TRX_IRQ_RX_START                = (0x04),
	TRX_IRQ_TRX_END                 = (0x08),
	TRX_IRQ_CCA_ED_DONE             = (0x10),	//16
	TRX_IRQ_AMI             		= (0x20),	//32
	TRX_IRQ_TRX_UR                  = (0x40),	//64
	TRX_IRQ_BAT_LOW                 = (0x80)	//128
} radio_irq_reason_t;


/** Transceiver states */
typedef enum radio_hal_trx_status
{
    P_ON                            = 0,
    BUSY_RX                         = 1,
    BUSY_TX                         = 2,
	FORCE_PLL_ON					= 4, // should not be used
	FORCE_TRX_OFF					= 5,
    RX_ON                           = 6,
    TRX_OFF                         = 8,
    PLL_ON                          = 9,
    TRX_SLEEP                       = 15,
    BUSY_RX_AACK                    = 17,
    BUSY_TX_ARET                    = 18,
    RX_AACK_ON                      = 22,
    TX_ARET_ON                      = 25,
    RX_ON_NOCLK                     = 28,
    RX_AACK_ON_NOCLK                = 29,
    BUSY_RX_AACK_NOCLK              = 30,
    STATE_TRANSITION_IN_PROGRESS    = 31
} radio_hal_trx_status_t;

/*****************************************************************************/

enum RadioStateEnum
{
	STATE_P_ON = 0,
	STATE_SLEEP = 1,
	STATE_SLEEP_2_TRX_OFF = 2,
	STATE_TRX_OFF = 3,
	STATE_TRX_OFF_2_RX_ON = 4,
	STATE_RX_ON = 5,
	STATE_BUSY_TX_2_RX_ON = 6,
	STATE_PLL_ON_2_RX_ON = 7,
	STATE_BUSY_TX = 8,
	STATE_PLL_ON = 9,
	STATE_SLEEP_PENDING = 10,
	STATE_BUSY_RX = 11,
	STATE_RX_AACK_ON = 12,
	STATE_BUSY_RX_AACK = 13,
	STATE_TX_ARET_ON = 14,
	STATE_BUSY_TX_ARET = 15,
	STATE_RX_AACK_ON_NOCLK = 16,
	STATE_BUSY_RX_AACK_NOCLK = 17
};

enum RadioCommandEnum
{
	CMD_NONE = 0,
	CMD_TURNOFF = 1,
	CMD_STANDBY = 2,
	CMD_TURNON = 3,
	CMD_TRANSMIT = 4,
	CMD_RECEIVE = 5,
	CMD_CCA = 6,
	CMD_CHANNEL = 7,
	CMD_SIGNAL_DONE = 8,
	CMD_DOWNLOAD = 9,
	CMD_LISTEN = 10,
	CMD_TX_ARET = 11,
	CMD_RX_AACK = 12
};

enum Rf230RegistersEnum {

  RF230_TRX_STATUS = 0x01,
  RF230_TRX_STATE = 0x02,
  RF230_TRX_CTRL_0 = 0x03,
  RF230_TRX_CTRL_1 = 0x04,
  RF230_PHY_TX_PWR = 0x05,
  RF230_PHY_RSSI = 0x06,
  RF230_PHY_ED_LEVEL = 0x07,
  RF230_PHY_CC_CCA = 0x08,
  RF230_CCA_THRES = 0x09,
  RF230_TRX_CTRL_2 = 0x0C,
  RF230_IRQ_MASK = 0x0E,
  RF230_IRQ_STATUS = 0x0F,
  RF230_VREG_CTRL = 0x10,
  RF230_BATMON = 0x11,
  RF230_XOSC_CTRL = 0x12,
  RF230_XAH_CTRL_1 = 0x17,
  RF230_PLL_CF = 0x1A,
  RF230_PLL_DCU = 0x1B,
  RF230_PART_NUM = 0x1C,
  RF230_VERSION_NUM = 0x1D,
  RF230_MAN_ID_0 = 0x1E,
  RF230_MAN_ID_1 = 0x1F,
  RF230_SHORT_ADDR_0 = 0x20,
  RF230_SHORT_ADDR_1 = 0x21,
  RF230_PAN_ID_0 = 0x22,
  RF230_PAN_ID_1 = 0x23,
  RF230_IEEE_ADDR_0 = 0x24,
  RF230_IEEE_ADDR_1 = 0x25,
  RF230_IEEE_ADDR_2 = 0x26,
  RF230_IEEE_ADDR_3 = 0x27,
  RF230_IEEE_ADDR_4 = 0x28,
  RF230_IEEE_ADDR_5 = 0x29,
  RF230_IEEE_ADDR_6 = 0x2A,
  RF230_IEEE_ADDR_7 = 0x2B,
  RF230_XAH_CTRL_0 = 0x2C,
  RF230_CSMA_SEED_0 = 0x2D,
  RF230_CSMA_SEED_1 = 0x2E,
  RF230_CSMA_BE = 0x2F
};


enum Rf230TrxRegisterEnum {

  RF230_CCA_DONE = 1 << 7,
  RF230_CCA_STATUS = 1 << 6,
  RF230_TRX_STATUS_MASK = 0x1F,
  RF230_P_ON = 0,
  RF230_BUSY_RX = 1,
  RF230_BUSY_TX = 2,
  RF230_RX_ON = 6,
  RF230_TRX_OFF = 8,
  RF230_PLL_ON = 9,
  RF230_SLEEP = 15,
  RF230_BUSY_RX_AACK = 16,
  RF230_BUSR_TX_ARET = 17,
  RF230_RX_AACK_ON = 22,
  RF230_TX_ARET_ON = 25,
  RF230_RX_ON_NOCLK = 28,
  RF230_AACK_ON_NOCLK = 29,
  RF230_BUSY_RX_AACK_NOCLK = 30,
  RF230_STATE_TRANSITION_IN_PROGRESS = 31,
  RF230_TRAC_STATUS_MASK = 0xE0,
  RF230_TRAC_SUCCESS = 0,
  RF230_TRAC_CHANNEL_ACCESS_FAILURE = 3 << 5,
  RF230_TRAC_NO_ACK = 5 << 5,
  RF230_TRX_CMD_MASK = 0x1F,
  RF230_NOP = 0,
  RF230_TX_START = 2,
  RF230_FORCE_TRX_OFF = 3
};

enum Rf230IrqRegisterEnum {

  RF230_IRQ_BAT_LOW = 1 << 7,
  RF230_IRQ_TRX_UR = 1 << 6,
  RF230_IRQ_AMI = 1 << 5,
  RF230_IRQ_CCA_ED_DONE = 1 << 4,
  RF230_IRQ_TRX_END = 1 << 3,
  RF230_IRQ_RX_START = 1 << 2,
  RF230_IRQ_PLL_UNLOCK = 1 << 1,
  RF230_IRQ_PLL_LOCK = 1 << 0
};

enum Rf230PhyRegisterEnum {

  RF230_TX_AUTO_CRC_ON = 1 << 7,
  RF230_TX_PWR_MASK = 0x0F,
  RF230_RSSI_MASK = 0x1F,
  RF230_CCA_REQUEST = 1 << 7,
  RF230_CCA_MODE_0 = 0 << 5,
  RF230_CCA_MODE_1 = 1 << 5,
  RF230_CCA_MODE_2 = 2 << 5,
  RF230_CCA_MODE_3 = 3 << 5,
  RF230_CHANNEL_DEFAULT = 11,
  RF230_CHANNEL_MASK = 0x1F,
  RF230_CCA_CS_THRES_SHIFT = 4,
  RF230_CCA_ED_THRES_SHIFT = 0
};

enum Rf230SpiCommandEnums {

  RF230_CMD_REGISTER_READ = 0x80,
  RF230_CMD_REGISTER_WRITE = 0xC0,
  RF230_CMD_REGISTER_MASK = 0x3F,
  RF230_CMD_FRAME_READ = 0x20,
  RF230_CMD_FRAME_WRITE = 0x60,
  RF230_CMD_SRAM_READ = 0x00,
  RF230_CMD_SRAM_WRITE = 0x40
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RF230_CHANNEL_OFFSET 11

enum Channels
{
	Channel_11,
	channel_12,
	channel_13,
	channel_14,
	channel_15,
	channel_16,
	channel_17,
	channel_18,
	channel_19,
	channel_20,
	channel_21,
	channel_22,
	channel_23,
	channel_24,
	channel_25,
	channel_26,
};

//page 70 in RF231 datasheet
enum TRAC_STATUS
{
	TRAC_STATUS_SUCCESS = 0x00,
	TRAC_STATUS_SUCCESS_DATA_PENDING = 0x20,	//0010 0000 (32)
	TRAC_STATUS_SUCCESS_WAIT_FOR_ACK = 0x40,	//0100 0000 (64)
	TRAC_STATUS_CHANNEL_ACCESS_FAILURE = 0x60,	//0110 0000 (96)
	TRAC_STATUS_FAIL_TO_SEND = 0x80,			//1000 0000 (128)
	TRAC_STATUS_NO_ACK = 0xA0,					//1010 0000 (160)
	TRAC_STATUS_INVALID	= 0xE0,					//1110 0000 (224)
};

class RF231Radio : public Radio<Message_15_4_t>
{
	// Pin Definitions for the state control pins declared by macros
	GPIO_PIN kslpTr;
	GPIO_PIN krstn;
	GPIO_PIN kseln;

	GPIO_PIN kinterrupt;

	// Stores the configuration of the spi
	SPI_CONFIGURATION config;

	// Variables to control the software state machine
	volatile RadioCommandType cmd;
	volatile RadioStateType state;

	// Control transmission power and channel of transmission
	UINT8 tx_power;
	UINT8 channel;

	// Record rssi values
	volatile UINT8 rssi_busy;

	// Contains the length of the message that will be transmitted
	UINT8 length;

	// Receive length and data, could have used the same as above, but trying to avoid unnecessary confusion
	volatile UINT8 rx_length;
	volatile INT64 receive_timestamp;

	UINT16 active_mac_index;

	// Pointer to the outgoing message
	Message_15_4_t* tx_msg_ptr;

	// Pointer to the incoming message
	Message_15_4_t* rx_msg_ptr;

	volatile BOOL sleep_pending;

	volatile UINT8 tx_length;

	// Initialize the rstn, seln and slptr pins
	BOOL GpioPinInitialize();
	BOOL GpioPinUnInitialize();

	// Initialize the spi module
    BOOL SpiInitialize();
    BOOL SpiUnInitialize();

    // All the below functions can be made inline, but this may increase the executable size, so i am deciding against this now
    // Am assuming the speed is not significant enough to warrant the change to inline
    // This remains to be profiled
    void SelnClear()
    {
    	CPU_GPIO_SetPinState(kseln, FALSE);
    }

    void SelnSet()
    {
    	CPU_GPIO_SetPinState(kseln, TRUE);
    }

    void RstnClear()
    {
    	CPU_GPIO_SetPinState(krstn, FALSE);
    }

    void RstnSet()
    {
    	CPU_GPIO_SetPinState(krstn, TRUE);
    }

    void SlptrClear()
    {
    	CPU_GPIO_SetPinState(kslpTr, FALSE);
    }

    void SlptrSet()
    {
    	CPU_GPIO_SetPinState(kslpTr, TRUE);
    }

private:
	void Wakeup();
	BOOL Interrupt_Pending();
	BOOL Careful_State_Change(radio_hal_trx_status_t target);
	BOOL Careful_State_Change(uint32_t target); // Our register enums are a mess. Until I fix. --NPS
	BOOL Careful_State_Change_Extended(radio_hal_trx_status_t target);
	BOOL Careful_State_Change_Extended(uint32_t target);
	void* Send_Ack(void *msg, UINT16 size, NetOpStatus status, UINT8 tracStatus);

	RadioEventHandler Radio_event_handler;

	UINT32 sequenceNumberSender;
	UINT32 sequenceNumberReceiver;
	UINT8 RF231_extended_mode;

public:
    DeviceStatus ChangeState();
    // Indicates whether the message has been loaded into the frame buffer or not
    volatile BOOL message_status;

    UINT32 GetChannel();

    UINT32 GetTxPower();

    void SetRadioName(INT32 radio)
    {
    	SetRadioID(radio);
    	//this->radioType = radio;
    }

    INT32 GetRadioName()
    {
    	return GetRadioID();
    	//return this->radioType;
    }

    // Functions added to support the long range radio
    DeviceStatus AntDiversity(BOOL enable);

    DeviceStatus PARXTX(BOOL enable);

    void Amp(BOOL TurnOn);

    // This function has been moved from the cpp file because the linker is unable to call the function
    // otherwise, something to do with instantiation of the template
    // Calls the gpio initialize and spi initialize modules and asserts if the spi initialization failed
    // Once SPI initialization is complete, radio initialization including setting radio state, channel, tx power etc carried out here
    // The radio is sleeping at the end of initialization
    DeviceStatus Initialize(RadioEventHandler *event_handler, UINT8 radio, UINT8 mac_id);

    DeviceStatus UnInitialize();

    DeviceStatus EnableCSMA();
    DeviceStatus DisableCSMA();
    void* SendRetry();
    void* SendStrobe(UINT16 size);
    void* Send(void* msg, UINT16 size);

    void* Send_TimeStamped(void* msg, UINT16 size, UINT32 eventTime);

    Message_15_4_t* Preload(Message_15_4_t* msg, UINT16 size);


    DeviceStatus ClearChannelAssesment();
    DeviceStatus ClearChannelAssesment(UINT32 numberMicroSecond);
    DeviceStatus SetTimeStamp();
    DeviceStatus SetTimeStamp(UINT64 timeStamp);
    INT32 GetSNR();
    INT32 GetRSSI();
    UINT16 GetAverageOrMaxRSSI_countN(const UINT8 rssiCount);
    BOOL CheckForRSSI();

    DeviceStatus Sleep(int level);

    //UINT8 GetRadioID();

	void WriteRegister(UINT8 reg, UINT8 value);

	DeviceStatus Reset();

	__IO UINT8 ReadRegister(UINT8 reg);

	// May have to toggle slp_tr, check this during testing of this interface
	void setChannel(UINT8 channel)
	{
		WriteRegister(RF230_PHY_CC_CCA, RF230_CCA_MODE_VALUE | channel);
	}

	RadioCommandType GetCommand()
	{
		return cmd;
	}

	RadioStateType GetState()
	{
		return state;
	}

	void SetCommand(RadioCommandType cmd)
	{
		this->cmd = cmd;
	}

	void SetState(RadioStateType state)
	{
		this->state = state;
	}

	DeviceStatus ChangeTxPower(int power);

	DeviceStatus ChangeChannel(int channel);

	DeviceStatus PreLoadFrame();

	DeviceStatus TurnOnRx();
	DeviceStatus TurnOffRx();

	DeviceStatus TurnOff();

	DeviceStatus StandBy();

	DeviceStatus DownloadMessage();
	DeviceStatus DownloadMessage(UINT16 length);

	void HandleInterrupt();
};
#endif /* RADIO_H */














