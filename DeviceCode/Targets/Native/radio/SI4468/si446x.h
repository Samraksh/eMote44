#pragma once
#include <stdint.h>
#include <tinyhal.h>

#include <Samraksh\Radio.h>

enum { PH_STATUS_MASK_FILTER_MATCH = 0x80, PH_STATUS_MASK_FILTER_MISS = 0x40, PH_STATUS_MASK_PACKET_SENT=0x20, PH_STATUS_MASK_PACKET_RX=0x10, \
PH_STATUS_MASK_CRC_ERROR=0x08, PH_STATUS_MASK_ALT_CRC_ERROR=0x04, PH_STATUS_MASK_TX_FIFO_ALMOST_EMPTY=0x02, PH_STATUS_MASK_RX_FIFO_ALMOST_EMPTY=0x01 };

enum { MODEM_MASK_RSSI_LATCH = 0x80, MODEM_MASK_POSTAMBLE_DETECT = 0x40, MODEM_MASK_INVALID_SYNC=0x20, MODEM_MASK_RSSI_JUMP=0x10, \
MODEM_MASK_RSSI=0x08, MODEM_MASK_INVALID_PREAMBLE=0x04, MODEM_MASK_PREAMBLE_DETECT=0x02, MODEM_MASK_SYNC_DETECT=0x01 };

enum radio_lock_id_t {
	radio_lock_none			=0,
	radio_lock_tx			=1,
	radio_lock_tx_power		=2,
	radio_lock_set_channel	=3,
	radio_lock_cca			=4,
	radio_lock_cca_ms		=5,
	radio_lock_rx			=6,
	radio_lock_init			=7,
	radio_lock_uninit		=8,
	radio_lock_reset		=9,
	radio_lock_sleep		=10,
	radio_lock_crc			=11, // Not used
	radio_lock_interrupt	=12,
	radio_lock_rx_setup		=13,
	radio_lock_all			=0xFF,
};

enum {
	NONE00=0,
	DEBUG01=1,
	DEBUG02=2,
	DEBUG03=3,
	ERR99=99,		// For things that are bad unless you know what you are doing.
	ERR100=100,
};

enum {
	NO_TIMESTAMP=0,
	YES_TIMESTAMP=1,
};

// These are registers that are changed AFTER automatic generation.
// Check to make sure they are set.
enum {
	RF_GLOBAL_CONFIG_1_1 = 0x70, // For the FIFO size change, prev 0x30
	PKT_LEN = 0x22,
	PKT_FIELD_2_LENGTH = 0x90,
};

// Driver Constants
enum {
	si446x_lock_max_attempts=2,
	si446x_timestamp_size=4,
	si446x_packet_size=128, // max payload accounting for 1 byte size for variable packet
	si446x_payload_ts=124, // max with timestamp of 4 bytes
	si446x_default_power=0x7f,
	si446x_default_channel=0,
	si446x_rssi_cca_thresh=0x7F,
	si446x_tx_timeout=200,
	si446x_rx_timeout_ms=100,
	serial_size=12,
};

typedef uint8_t U8;
typedef uint16_t U16;

typedef void (*my_debug_print_t)(int, const char *, ...);
typedef void (*si446x_tx_callback_t)();
typedef void (*si446x_rx_callback_t)(UINT64, unsigned, const __restrict__ uint8_t *);

typedef enum { 	SI_STATE_BOOT=0, SI_STATE_SLEEP=1, SI_STATE_SPI_ACTIVE=2, \
				SI_STATE_READY=3, SI_STATE_READY2=4, SI_STATE_TX_TUNE=5, \
				SI_STATE_RX_TUNE=6, SI_STATE_TX=7, SI_STATE_RX=8, SI_STATE_ERROR=9, \
				SI_STATE_UNKNOWN=10
} si_state_t;

typedef struct {
	SPI_TypeDef 	*spi_base;
	GPIO_TypeDef 	*spi_port;
	uint32_t		spi_rcc;
	GPIO_TypeDef 	*nirq_port;
	uint16_t		nirq_pin;
	GPIO_PIN		nirq_mf_pin;
	GPIO_TypeDef 	*gpio0_port;
	GPIO_TypeDef 	*gpio1_port;
	uint16_t		gpio0_pin;
	uint16_t		gpio1_pin;
	GPIO_TypeDef 	*cs_port;
	uint16_t		cs_pin;
	uint16_t		sclk_pin;
	uint16_t		miso_pin;
	uint16_t		mosi_pin;
	GPIO_TypeDef 	*sdn_port;
	uint16_t		sdn_pin;
} SI446X_pin_setup_t;

// MF HAL FUNCTIONS
DeviceStatus 	si446x_hal_init(RadioEventHandler *event_handler, UINT8 radio, UINT8 mac_id);
DeviceStatus 	si446x_hal_uninitialize(UINT8 radio);
DeviceStatus 	si446x_hal_reset(UINT8 radio);
UINT16 			si446x_hal_get_address(UINT8 radio);
BOOL			si446x_hal_set_address(UINT8 radio, UINT16 address);
void *			si446x_hal_send(UINT8 radioID, void *msg, UINT16 size);
void *			si446x_hal_send_ts(UINT8 radioID, void *msg, UINT16 size, UINT32 eventTime);
DeviceStatus 	si446x_hal_rx(UINT8 radioID);
DeviceStatus 	si446x_hal_sleep(UINT8 radioID);
DeviceStatus 	si446x_hal_tx_power(UINT8 radioID, int pwr);
DeviceStatus 	si446x_hal_set_channel(UINT8 radioID, int channel);
INT8 			si446x_hal_get_RadioType();
void 			si446x_hal_set_RadioType(INT8 radio);
DeviceStatus 	si446x_hal_cca_ms(UINT8 radioID, UINT32 ms);
DeviceStatus 	si446x_packet_send(uint8_t chan, uint8_t *pkt, uint8_t len, UINT32 eventTime, int doTS, uint8_t after_state);
UINT32 			si446x_hal_get_chan(UINT8 radioID);
UINT32 			si446x_hal_get_power(UINT8 radioID);
UINT32			si446x_hal_get_rssi(UINT8 radioID);
void			si446x_hal_set_default_state(si_state_t defaultState);

void			si446x_hal_register_tx_callback(si446x_tx_callback_t handler);
void			si446x_hal_unregister_tx_callback();

void			si446x_hal_register_rx_callback(si446x_rx_callback_t handler);
void			si446x_hal_unregister_rx_callback();

// END MF HAL FUNCTIONS

void si446x_set_debug_print(my_debug_print_t f, unsigned level);
void si446x_reset(void);
int si446x_part_info();
void si446x_get_int_status(uint8_t PH_CLR_PEND, uint8_t MODEM_CLR_PEND, uint8_t CHIP_CLR_PEND);
int si446x_func_info();
si_state_t si446x_request_device_state(void);
void si446x_write_tx_fifo(uint8_t numBytes, uint8_t* pTxData);
void si446x_read_rx_fifo(uint8_t numBytes, uint8_t* pRxData);
void si446x_change_state(si_state_t NEXT_STATE1);
void si446x_start_tx_fast(void);
void si446x_start_tx(uint8_t CHANNEL, uint8_t CONDITION, uint16_t TX_LEN);
void si446x_fifo_info(uint8_t FIFO);
void si446x_start_rx(uint8_t CHANNEL, uint8_t CONDITION, uint16_t RX_LEN, uint8_t NEXT_STATE1, uint8_t NEXT_STATE2, uint8_t NEXT_STATE3);
void si446x_start_rx_fast(void);
void si446x_get_modem_status( uint8_t MODEM_CLR_PEND );
void si446x_set_property( U8 GROUP, U8 NUM_PROPS, int START_PROP, ... );
void si446x_frr_a_read(U8 respByteCount);
void si446x_frr_b_read(U8 respByteCount);
void si446x_frr_c_read(U8 respByteCount);
void si446x_frr_d_read(U8 respByteCount);
void si446x_get_adc_reading(U8 ADC_EN);
U8 si446x_get_property(U8 GROUP, U8 NUM_PROPS, U8 START_PROP);
U8 si446x_get_property_multi(U8 GROUP, U8 NUM_PROPS, U8 START_PROP, U8 *out);
void si446x_ircal(U8 SEARCHING_STEP_SIZE, U8 SEARCHING_RSSI_AVG, U8 RX_CHAIN_SETTING1, U8 RX_CHAIN_SETTING2);
int si446x_get_packet_info(U8 FIELD_NUMBER_MASK, U16 LEN, int16_t DIFF_LEN );

// custom
uint8_t si446x_get_latched_rssi();
uint8_t si446x_get_current_rssi();
uint8_t si446x_get_ph_status();
uint8_t si446x_get_ph_pend();
uint8_t si446x_get_modem_pend();
uint8_t si446x_get_chip_pend();
uint8_t si446x_get_modem_status();
uint8_t si446x_get_chip_status();
int 	si446x_get_afc_info(void);
void si446x_start_rx_fast_channel(uint8_t CHANNEL);
void si446x_start_tx_fast_channel(uint8_t CHANNEL);
si_state_t si446x_request_device_state_shadow(void);

extern uint8_t radio_spi_go(uint8_t data);
extern void radio_shutdown(int go);
//extern unsigned radio_get_gpio0();
//extern unsigned radio_get_gpio1();
extern void radio_spi_sel_assert(void);
extern void radio_spi_sel_no_assert(void);
extern bool radio_get_assert_irq(void);

//extern void radio_debug_print(int level, const char * restrict fmt, ...);

void radio_comm_ClearCTS();
unsigned int radio_comm_PollCTS();
void radio_comm_SendCmd(unsigned byteCount, const uint8_t* pData);
void radio_comm_ReadData(uint8_t cmd, unsigned pollCts, uint8_t byteCount, uint8_t* pData);
void radio_comm_WriteData(uint8_t cmd, unsigned pollCts, uint8_t byteCount, uint8_t* pData);
uint8_t radio_comm_GetResp(uint8_t byteCount, uint8_t* pData);
uint8_t radio_comm_SendCmdGetResp(uint8_t cmdByteCount, uint8_t* pCmdData, uint8_t respByteCount, uint8_t* pRespData);
