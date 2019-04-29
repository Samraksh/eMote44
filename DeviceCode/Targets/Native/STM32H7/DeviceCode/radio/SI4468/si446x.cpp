#include <stdint.h>
#include <stdarg.h>
#include "si446x.h"

// TODO allow selection of other 446x chips
#include "radio_config_Si4468.h"

#include "si446x_cmd.h"
#include <stm32f10x.h>

// Unknown reasons, modem_pend does not mask INVALID_PREAMBLE as expected.
// Masking manually for now. --NPS
#define SI446X_MODEM_PEND_MASK 0x01 // for SYNC_DET only
#define SI446X_PH_PEND_MASK 0x38 	// 00111000 == PSNT/PRX/CRCE

//#define SI446X_AGGRESSIVE_CTS

// Constants and states
enum { CTS_TIMEOUT = 262143*2, CTS_VAL_GOOD=0xFF, CTS_WAIT=255 };
enum { VERB0=0, VERB1=1, ERR0=128, ERR1=256 };
enum { ROMC2A=6, ROMB1B=3 };

//#ifdef SI4468
enum { PART_SI446X=0x4468, PATCHID=0 };
//#endif

// State Vars
static unsigned ctsWentHigh;
static volatile si_state_t current_state;

static uint8_t latched_rssi;
static uint8_t current_rssi;

static uint8_t ph_status;
static uint8_t modem_status;
static uint8_t chip_status;

static uint8_t ph_pend;
static uint8_t modem_pend;
static uint8_t chip_pend;

static int16_t freq_offset; // frequency offset info from AFC, if on.

// Buffers
static union si446x_cmd_reply_union Si446xCmd; // Reply Buffer
static uint8_t Pro2Cmd[16]; // Tx Command Buffer


// Static Helper Functions

static void spi_read_bytes(unsigned count, uint8_t *buf) {
	for(unsigned i=0; i<count; i++) {
		buf[i] = radio_spi_go(0);
	}
}

static void spi_write_bytes(unsigned count, const uint8_t *buf) {
	for(unsigned i=0; i<count; i++) {
		radio_spi_go(buf[i]);
	}
}

// Hopefully would allow to optimize release case? Check it.
static void blank_debug(int x, const char *fmt, ...) {
	return;
}


static my_debug_print_t si446x_debug_print;
void si446x_set_debug_print(my_debug_print_t f, unsigned level) {
	if (level != 0)
		si446x_debug_print = f;
	else
		si446x_debug_print = blank_debug;
}

static void radio_error() {
	//__ASM volatile ("bkpt"); // TURN ME OFF FOR RELEASE !!!!!!
	__enable_irq(); // So any remaining messages can spit out
	while(1);
}

static int SI_ASSERT(int x, const char *err) {
	if (!x) {
		si446x_debug_print(ERR100, "%s", err);
		radio_error();
		return 1;
	}
	else return 0;
}

// SI446x

// Returns measured offset error reported by AFC.
// Refer to AN734 section 4.7
int si446x_get_afc_info(void) {
	const int unit = 16000000 / 262144 / 8;
	return freq_offset * unit;
}

uint8_t si446x_get_latched_rssi() {
	return latched_rssi;
}

uint8_t si446x_get_ph_status() {
	return ph_status;
}

uint8_t si446x_get_ph_pend() {
#ifndef SI446X_PH_PEND_MASK
	return ph_pend;
#else
	return (ph_pend & SI446X_PH_PEND_MASK);
#endif
}

uint8_t si446x_get_modem_status() {
	return modem_status;
}

uint8_t si446x_get_modem_pend() {
#ifndef SI446X_MODEM_PEND_MASK
	return modem_pend;
#else
	return (modem_pend & SI446X_MODEM_PEND_MASK);
#endif
}

uint8_t si446x_get_chip_status() {
	return chip_status;
}

uint8_t si446x_get_chip_pend() {
	return chip_pend;
}

uint8_t si446x_get_current_rssi(){
	return current_rssi;
}

void si446x_reset(void)
{
	const uint8_t init_commands[] = RADIO_CONFIGURATION_DATA_ARRAY; // From radio_config.h
	
	// state cleanup
	ctsWentHigh = 0;
	current_state = SI_STATE_BOOT;
	// End state cleanup

	radio_shutdown(1);
	for(volatile int i=0; i<CTS_TIMEOUT; i++) ; // spin, replace with real delay, 10us+
	radio_shutdown(0);
	for(volatile int i=0; i<CTS_TIMEOUT; i++) ; // spin, replace with real delay, 10us+
	
	radio_comm_PollCTS();
	
	// WALK THROUGH INIT FROM RADIO_CONFIG.H
	// All commands are single array. Format is (size) (data of length size) (size) ...
	{
		unsigned index = 1;
		unsigned num_commands=0;
		uint8_t size = init_commands[0];
		unsigned bytes=size;
		while(size != 0) {
			radio_comm_SendCmd(size, &init_commands[index]);
			index += size;
			bytes += size;
			size = init_commands[index++];
			num_commands++;
		}
		si446x_debug_print(DEBUG01, "\tSI446x config sequence complete, %d commands %d bytes\r\n", num_commands, bytes);
	}
	si446x_change_state(SI_STATE_READY); // Move us to READY state, otherwise depends on init.

	// Verify that state transition passed. Discovered this as a failure mode. Perhaps hardware damaged?
	for(volatile int i=0; i<CTS_TIMEOUT; i++) ; // spin, replace with real delay, 10us+
	SI_ASSERT(si446x_request_device_state() == SI_STATE_READY, "Si446x FATAL: No response to state change request.\r\n");

	// END INIT
}

void si446x_set_property( U8 GROUP, U8 NUM_PROPS, int START_PROP, ... )
{
    va_list argList;
    U8 cmdIndex;

    Pro2Cmd[0] = SI446X_CMD_ID_SET_PROPERTY;
    Pro2Cmd[1] = GROUP;
    Pro2Cmd[2] = NUM_PROPS;
    Pro2Cmd[3] = START_PROP;

    va_start (argList, START_PROP);
    cmdIndex = 4;
    while(NUM_PROPS--)
    {
        Pro2Cmd[cmdIndex] = (U8) va_arg (argList, int);
        cmdIndex++;
    }
    va_end(argList);

    radio_comm_SendCmd( cmdIndex, Pro2Cmd );
}

int si446x_part_info()
{
	int ret=0;
    Pro2Cmd[0] = SI446X_CMD_ID_PART_INFO;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_PART_INFO,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_PART_INFO,
                              Pro2Cmd );

    Si446xCmd.PART_INFO.CHIPREV         = Pro2Cmd[0];
    Si446xCmd.PART_INFO.PART            = ((U16)Pro2Cmd[1] << 8) & 0xFF00;
    Si446xCmd.PART_INFO.PART           |= (U16)Pro2Cmd[2] & 0x00FF;
    Si446xCmd.PART_INFO.PBUILD          = Pro2Cmd[3];
    Si446xCmd.PART_INFO.ID              = ((U16)Pro2Cmd[4] << 8) & 0xFF00;
    Si446xCmd.PART_INFO.ID             |= (U16)Pro2Cmd[5] & 0x00FF;
    Si446xCmd.PART_INFO.CUSTOMER        = Pro2Cmd[6];
    Si446xCmd.PART_INFO.ROMID           = Pro2Cmd[7];
	
	si446x_debug_print(DEBUG01, "\tCHIPREV %d\r\n",	Si446xCmd.PART_INFO.CHIPREV);
	si446x_debug_print(DEBUG01, "\tPART 0x%.4X\r\n",	Si446xCmd.PART_INFO.PART); // This is best shown in hex
	si446x_debug_print(DEBUG01, "\tPBUILD %d\r\n",	Si446xCmd.PART_INFO.PBUILD);
	si446x_debug_print(DEBUG01, "\tID %d\r\n",		Si446xCmd.PART_INFO.ID);
	si446x_debug_print(DEBUG01, "\tCUSTOMER %d\r\n",	Si446xCmd.PART_INFO.CUSTOMER);
	si446x_debug_print(DEBUG01, "\tROMID %d\r\n",	Si446xCmd.PART_INFO.ROMID); // ROMID=03 means you have a revB1B chip. ROMID of revC2A is 06.
	
	// RF4463PRO board from niceRF has ROMID 6 == revC2A
	// Unfortunately it looks like different chip revs need different treatment. So must verify ROMID
	si446x_debug_print(DEBUG01, "\nROMID = %d\n", Si446xCmd.PART_INFO.ROMID);
	ret += SI_ASSERT(Si446xCmd.PART_INFO.ROMID == ROMC2A, "Fatal: Bad ROMID\r\n");
	ret += SI_ASSERT(Si446xCmd.PART_INFO.PART  == PART_SI446X, "Fatal: Bad Part\r\n");
	
	return ret;
}

/*!
 * Sends the FUNC_INFO command to the radio, then reads the resonse into @Si446xCmd union.
 */
int si446x_func_info()
{
	int ret = 0;
    Pro2Cmd[0] = SI446X_CMD_ID_FUNC_INFO;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_FUNC_INFO,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_FUNC_INFO,
                              Pro2Cmd );

    Si446xCmd.FUNC_INFO.REVEXT          = Pro2Cmd[0];
    Si446xCmd.FUNC_INFO.REVBRANCH       = Pro2Cmd[1];
    Si446xCmd.FUNC_INFO.REVINT          = Pro2Cmd[2];
    Si446xCmd.FUNC_INFO.PATCH           = ((U16)Pro2Cmd[3] << 8) & 0xFF00;
    Si446xCmd.FUNC_INFO.PATCH          |= (U16)Pro2Cmd[4] & 0x00FF;
    Si446xCmd.FUNC_INFO.FUNC            = Pro2Cmd[5];
	
	si446x_debug_print(DEBUG01, "\tREVEXT %d\r\n",	Si446xCmd.FUNC_INFO.REVEXT);
	si446x_debug_print(DEBUG01, "\tREVBRANCH %d\r\n",	Si446xCmd.FUNC_INFO.REVBRANCH);
	si446x_debug_print(DEBUG01, "\tREVINT %d\r\n",	Si446xCmd.FUNC_INFO.REVINT);
	si446x_debug_print(DEBUG01, "\tPATCH 0x%.4X\r\n",	Si446xCmd.FUNC_INFO.PATCH);
	si446x_debug_print(DEBUG01, "\tFUNC %d\r\n",		Si446xCmd.FUNC_INFO.FUNC);
	
	ret += SI_ASSERT(Si446xCmd.FUNC_INFO.PATCH  == PATCHID, "Fatal: Patch Failed\r\n");
	
	return ret;
}

/*!
 * Get the Interrupt status/pending flags form the radio and clear flags if requested.
 *
 * @param PH_CLR_PEND     Packet Handler pending flags clear.
 * @param MODEM_CLR_PEND  Modem Status pending flags clear.
 * @param CHIP_CLR_PEND   Chip State pending flags clear.
 */
void si446x_get_int_status(uint8_t PH_CLR_PEND, uint8_t MODEM_CLR_PEND, uint8_t CHIP_CLR_PEND)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_INT_STATUS;
    Pro2Cmd[1] = PH_CLR_PEND;
    Pro2Cmd[2] = MODEM_CLR_PEND;
    Pro2Cmd[3] = CHIP_CLR_PEND;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_INT_STATUS,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_INT_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_INT_STATUS.INT_PEND       = Pro2Cmd[0];
    Si446xCmd.GET_INT_STATUS.INT_STATUS     = Pro2Cmd[1];
    Si446xCmd.GET_INT_STATUS.PH_PEND        = Pro2Cmd[2];
    Si446xCmd.GET_INT_STATUS.PH_STATUS      = Pro2Cmd[3];
    Si446xCmd.GET_INT_STATUS.MODEM_PEND     = Pro2Cmd[4];
    Si446xCmd.GET_INT_STATUS.MODEM_STATUS   = Pro2Cmd[5];
    Si446xCmd.GET_INT_STATUS.CHIP_PEND      = Pro2Cmd[6];
    Si446xCmd.GET_INT_STATUS.CHIP_STATUS    = Pro2Cmd[7];
	
	ph_pend    = 	Si446xCmd.GET_INT_STATUS.PH_PEND;
	ph_status = 	Si446xCmd.GET_INT_STATUS.PH_STATUS;
	modem_pend = 	Si446xCmd.GET_INT_STATUS.MODEM_PEND;
	modem_status = 	Si446xCmd.GET_INT_STATUS.MODEM_STATUS;
	chip_pend 	=	Si446xCmd.GET_INT_STATUS.CHIP_PEND;
	chip_status =	Si446xCmd.GET_INT_STATUS.CHIP_STATUS;

	// A strategic point to note that we are no longer sleeping.
	if (current_state == SI_STATE_SLEEP) { current_state = SI_STATE_UNKNOWN; }
}

// Requests the cached state instead of directly asking the chip.
// As a result this isn't by any means guaranteed accurate.
// Mostly for a sleep check.
si_state_t si446x_request_device_state_shadow()
{
	return current_state;
}

// Will actually ask the radio as opposed to above.
si_state_t si446x_request_device_state()
{
    Pro2Cmd[0] = SI446X_CMD_ID_REQUEST_DEVICE_STATE;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd );

    Si446xCmd.REQUEST_DEVICE_STATE.CURR_STATE       = Pro2Cmd[0];
    Si446xCmd.REQUEST_DEVICE_STATE.CURRENT_CHANNEL  = Pro2Cmd[1];
	
	//si446x_debug_print(DEBUG01, "\tSTATE %d\r\n",	Si446xCmd.REQUEST_DEVICE_STATE.CURR_STATE);
	//si446x_debug_print(DEBUG01, "\tCHANNEL %d\r\n",Si446xCmd.REQUEST_DEVICE_STATE.CURRENT_CHANNEL);
	
	current_state = (si_state_t) Si446xCmd.REQUEST_DEVICE_STATE.CURR_STATE;
	return current_state;
}

/*!
 * Issue a change state command to the radio.
 *
 * @param NEXT_STATE1 Next state.
 */
void si446x_change_state(si_state_t NEXT_STATE1)
{
    Pro2Cmd[0] = SI446X_CMD_ID_CHANGE_STATE;
    Pro2Cmd[1] = (U8) NEXT_STATE1;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_CHANGE_STATE, Pro2Cmd );
	
	// Just assuming this works... maybe should check...
	current_state = (si_state_t) NEXT_STATE1;
}

/*!
 * The function can be used to load data into TX FIFO.
 *
 * @param numBytes  Data length to be load.
 * @param pTxData   Pointer to the data (U8*).
 */
void si446x_write_tx_fifo(uint8_t numBytes, uint8_t* pTxData)
{
  radio_comm_WriteData( SI446X_CMD_ID_WRITE_TX_FIFO, 0, numBytes, pTxData );
}

/*!
 * Reads the RX FIFO content from the radio.
 *
 * @param numBytes  Data length to be read.
 * @param pRxData   Pointer to the buffer location.
 */
void si446x_read_rx_fifo(uint8_t numBytes, uint8_t* pRxData)
{
  radio_comm_ReadData( SI446X_CMD_ID_READ_RX_FIFO, 0, numBytes, pRxData );
}

void si446x_get_modem_status( uint8_t MODEM_CLR_PEND )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_MODEM_STATUS;
    Pro2Cmd[1] = MODEM_CLR_PEND;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_MODEM_STATUS,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_MODEM_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_MODEM_STATUS.MODEM_PEND   = Pro2Cmd[0];
    Si446xCmd.GET_MODEM_STATUS.MODEM_STATUS = Pro2Cmd[1];
    Si446xCmd.GET_MODEM_STATUS.CURR_RSSI    = Pro2Cmd[2];
    Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI   = Pro2Cmd[3];
    Si446xCmd.GET_MODEM_STATUS.ANT1_RSSI    = Pro2Cmd[4];
    Si446xCmd.GET_MODEM_STATUS.ANT2_RSSI    = Pro2Cmd[5];
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET =  ((U16)Pro2Cmd[6] << 8) & 0xFF00;
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET |= (U16)Pro2Cmd[7] & 0x00FF;

	freq_offset = (int16_t) Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET;
	latched_rssi = Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI;
	current_rssi = Si446xCmd.GET_MODEM_STATUS.CURR_RSSI;
}

/*! Sends START_TX command ID to the radio with no input parameters
 *
 */
void si446x_start_tx_fast(void)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_TX;

    radio_comm_SendCmd( 1, Pro2Cmd );
}

/*! Sends START_TX command ID to the radio with just a channel
 * @param CHANNEL   Channel number.
 *  SAMRAKSH CUSTOM
 */
void si446x_start_tx_fast_channel(uint8_t CHANNEL)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_TX;
	Pro2Cmd[1] = CHANNEL;

    radio_comm_SendCmd( 2, Pro2Cmd );
}

/*! Sends START_TX command to the radio.
 *
 * @param CHANNEL   Channel number.
 * @param CONDITION Start TX condition.
 * @param TX_LEN    Payload length (exclude the PH generated CRC).
 */
void si446x_start_tx(uint8_t CHANNEL, uint8_t CONDITION, uint16_t TX_LEN)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_TX;
    Pro2Cmd[1] = CHANNEL;
    Pro2Cmd[2] = CONDITION;
    Pro2Cmd[3] = (uint8_t)(TX_LEN >> 8);
    Pro2Cmd[4] = (uint8_t)(TX_LEN);
    Pro2Cmd[5] = 0x00;

    // Don't repeat the packet, 
    // ie. transmit the packet only once
    Pro2Cmd[6] = 0x00;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_START_TX, Pro2Cmd );

	current_state = SI_STATE_TX;
}

/*!
 * Sends START_RX command to the radio.
 *
 * @param CHANNEL     Channel number.
 * @param CONDITION   Start RX condition.
 * @param RX_LEN      Payload length (exclude the PH generated CRC).
 * @param NEXT_STATE1 Next state when Preamble Timeout occurs.
 * @param NEXT_STATE2 Next state when a valid packet received.
 * @param NEXT_STATE3 Next state when invalid packet received (e.g. CRC error).
 */
void si446x_start_rx(uint8_t CHANNEL, uint8_t CONDITION, uint16_t RX_LEN, uint8_t NEXT_STATE1, uint8_t NEXT_STATE2, uint8_t NEXT_STATE3)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_RX;
    Pro2Cmd[1] = CHANNEL;
    Pro2Cmd[2] = CONDITION;
    Pro2Cmd[3] = (uint8_t)(RX_LEN >> 8);
    Pro2Cmd[4] = (uint8_t)(RX_LEN);
    Pro2Cmd[5] = NEXT_STATE1;
    Pro2Cmd[6] = NEXT_STATE2;
    Pro2Cmd[7] = NEXT_STATE3;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_START_RX, Pro2Cmd );
	current_state = SI_STATE_RX;
}

/*!
 * Sends START_RX command ID to the radio with no input parameters
 *
 */
void si446x_start_rx_fast(void)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_RX;

    radio_comm_SendCmd( 1, Pro2Cmd );
}

/*!
 * Sends START_RX command ID to the radio with just a channel
 * SAMRAKSH CUSTOM
 */
void si446x_start_rx_fast_channel(uint8_t CHANNEL)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_RX;
	Pro2Cmd[1] = CHANNEL;

    radio_comm_SendCmd( 2, Pro2Cmd );
}

/*!
 * Send the FIFO_INFO command to the radio. Optionally resets the TX/RX FIFO. Reads the radio response back
 * into @Si446xCmd.
 *
 * @param FIFO  RX/TX FIFO reset flags.
 */
void si446x_fifo_info(uint8_t FIFO)
{
    Pro2Cmd[0] = SI446X_CMD_ID_FIFO_INFO;
    Pro2Cmd[1] = FIFO;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_FIFO_INFO,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_FIFO_INFO,
                              Pro2Cmd );

    Si446xCmd.FIFO_INFO.RX_FIFO_COUNT   = Pro2Cmd[0];
    Si446xCmd.FIFO_INFO.TX_FIFO_SPACE   = Pro2Cmd[1];
}

/*!
 * Reads the Fast Response Registers starting with A register into @Si446xCmd union.
 *
 * @param respByteCount Number of Fast Response Registers to be read.
 */
void si446x_frr_a_read(U8 respByteCount)
{
    radio_comm_ReadData(SI446X_CMD_ID_FRR_A_READ,
                            0,
                        respByteCount,
                        Pro2Cmd);

    Si446xCmd.FRR_A_READ.FRR_A_VALUE = Pro2Cmd[0];
    Si446xCmd.FRR_A_READ.FRR_B_VALUE = Pro2Cmd[1];
    Si446xCmd.FRR_A_READ.FRR_C_VALUE = Pro2Cmd[2];
    Si446xCmd.FRR_A_READ.FRR_D_VALUE = Pro2Cmd[3];
}

/*!
 * Reads the Fast Response Registers starting with B register into @Si446xCmd union.
 *
 * @param respByteCount Number of Fast Response Registers to be read.
 */
void si446x_frr_b_read(U8 respByteCount)
{
    radio_comm_ReadData(SI446X_CMD_ID_FRR_B_READ,
                            0,
                        respByteCount,
                        Pro2Cmd);

    Si446xCmd.FRR_B_READ.FRR_B_VALUE = Pro2Cmd[0];
    Si446xCmd.FRR_B_READ.FRR_C_VALUE = Pro2Cmd[1];
    Si446xCmd.FRR_B_READ.FRR_D_VALUE = Pro2Cmd[2];
    Si446xCmd.FRR_B_READ.FRR_A_VALUE = Pro2Cmd[3];
}

/*!
 * Reads the Fast Response Registers starting with C register into @Si446xCmd union.
 *
 * @param respByteCount Number of Fast Response Registers to be read.
 */
void si446x_frr_c_read(U8 respByteCount)
{
    radio_comm_ReadData(SI446X_CMD_ID_FRR_C_READ,
                            0,
                        respByteCount,
                        Pro2Cmd);

    Si446xCmd.FRR_C_READ.FRR_C_VALUE = Pro2Cmd[0];
    Si446xCmd.FRR_C_READ.FRR_D_VALUE = Pro2Cmd[1];
    Si446xCmd.FRR_C_READ.FRR_A_VALUE = Pro2Cmd[2];
    Si446xCmd.FRR_C_READ.FRR_B_VALUE = Pro2Cmd[3];
}

/*!
 * Reads the Fast Response Registers starting with D register into @Si446xCmd union.
 *
 * @param respByteCount Number of Fast Response Registers to be read.
 */
void si446x_frr_d_read(U8 respByteCount)
{
    radio_comm_ReadData(SI446X_CMD_ID_FRR_D_READ,
                            0,
                        respByteCount,
                        Pro2Cmd);

    Si446xCmd.FRR_D_READ.FRR_D_VALUE = Pro2Cmd[0];
    Si446xCmd.FRR_D_READ.FRR_A_VALUE = Pro2Cmd[1];
    Si446xCmd.FRR_D_READ.FRR_B_VALUE = Pro2Cmd[2];
    Si446xCmd.FRR_D_READ.FRR_C_VALUE = Pro2Cmd[3];
}

/*!
 * Reads the ADC values from the radio into @Si446xCmd union.
 *
 * @param ADC_EN  ADC enable parameter.
 */
void si446x_get_adc_reading(U8 ADC_EN)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_ADC_READING;
    Pro2Cmd[1] = ADC_EN;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_ADC_READING,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_ADC_READING,
                              Pro2Cmd );

    Si446xCmd.GET_ADC_READING.GPIO_ADC         = ((U16)Pro2Cmd[0] << 8) & 0xFF00;
    Si446xCmd.GET_ADC_READING.GPIO_ADC        |=  (U16)Pro2Cmd[1] & 0x00FF;
    Si446xCmd.GET_ADC_READING.BATTERY_ADC      = ((U16)Pro2Cmd[2] << 8) & 0xFF00;
    Si446xCmd.GET_ADC_READING.BATTERY_ADC     |=  (U16)Pro2Cmd[3] & 0x00FF;
    Si446xCmd.GET_ADC_READING.TEMP_ADC         = ((U16)Pro2Cmd[4] << 8) & 0xFF00;
    Si446xCmd.GET_ADC_READING.TEMP_ADC        |=  (U16)Pro2Cmd[5] & 0x00FF;
}

/*!
 * Get property values from the radio. Reads them into Si446xCmd union.
 *
 * @param GROUP       Property group number.
 * @param NUM_PROPS   Number of properties to be read.
 * @param START_PROP  Starting sub-property number.
 */
U8 si446x_get_property(U8 GROUP, U8 NUM_PROPS, U8 START_PROP)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_PROPERTY;
    Pro2Cmd[1] = GROUP;
    Pro2Cmd[2] = NUM_PROPS;
    Pro2Cmd[3] = START_PROP;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_PROPERTY,
                              Pro2Cmd,
                              Pro2Cmd[2],
                              Pro2Cmd );

    //Si446xCmd.GET_PROPERTY.DATA[0 ]   = Pro2Cmd[0];
    //Si446xCmd.GET_PROPERTY.DATA[1 ]   = Pro2Cmd[1];
    //Si446xCmd.GET_PROPERTY.DATA[2 ]   = Pro2Cmd[2];
    //Si446xCmd.GET_PROPERTY.DATA[3 ]   = Pro2Cmd[3];
    //Si446xCmd.GET_PROPERTY.DATA[4 ]   = Pro2Cmd[4];
    //Si446xCmd.GET_PROPERTY.DATA[5 ]   = Pro2Cmd[5];
    //Si446xCmd.GET_PROPERTY.DATA[6 ]   = Pro2Cmd[6];
    //Si446xCmd.GET_PROPERTY.DATA[7 ]   = Pro2Cmd[7];
    //Si446xCmd.GET_PROPERTY.DATA[8 ]   = Pro2Cmd[8];
    //Si446xCmd.GET_PROPERTY.DATA[9 ]   = Pro2Cmd[9];
    //Si446xCmd.GET_PROPERTY.DATA[10]   = Pro2Cmd[10];
    //Si446xCmd.GET_PROPERTY.DATA[11]   = Pro2Cmd[11];
    //Si446xCmd.GET_PROPERTY.DATA[12]   = Pro2Cmd[12];
    //Si446xCmd.GET_PROPERTY.DATA[13]   = Pro2Cmd[13];
    //Si446xCmd.GET_PROPERTY.DATA[14]   = Pro2Cmd[14];
    //Si446xCmd.GET_PROPERTY.DATA[15]   = Pro2Cmd[15];

	return Pro2Cmd[0];
}

/*!
 * Get property values from the radio. Reads them into Si446xCmd union.
 *
 * @param GROUP       Property group number.
 * @param NUM_PROPS   Number of properties to be read.
 * @param START_PROP  Starting sub-property number.
 */
U8 si446x_get_property_multi(U8 GROUP, U8 NUM_PROPS, U8 START_PROP, U8 *out)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_PROPERTY;
    Pro2Cmd[1] = GROUP;
    Pro2Cmd[2] = NUM_PROPS;
    Pro2Cmd[3] = START_PROP;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_PROPERTY,
                              Pro2Cmd,
                              Pro2Cmd[2],
                              Pro2Cmd );

    //Si446xCmd.GET_PROPERTY.DATA[0 ]   = Pro2Cmd[0];
    //Si446xCmd.GET_PROPERTY.DATA[1 ]   = Pro2Cmd[1];
    //Si446xCmd.GET_PROPERTY.DATA[2 ]   = Pro2Cmd[2];
    //Si446xCmd.GET_PROPERTY.DATA[3 ]   = Pro2Cmd[3];
    //Si446xCmd.GET_PROPERTY.DATA[4 ]   = Pro2Cmd[4];
    //Si446xCmd.GET_PROPERTY.DATA[5 ]   = Pro2Cmd[5];
    //Si446xCmd.GET_PROPERTY.DATA[6 ]   = Pro2Cmd[6];
    //Si446xCmd.GET_PROPERTY.DATA[7 ]   = Pro2Cmd[7];
    //Si446xCmd.GET_PROPERTY.DATA[8 ]   = Pro2Cmd[8];
    //Si446xCmd.GET_PROPERTY.DATA[9 ]   = Pro2Cmd[9];
    //Si446xCmd.GET_PROPERTY.DATA[10]   = Pro2Cmd[10];
    //Si446xCmd.GET_PROPERTY.DATA[11]   = Pro2Cmd[11];
    //Si446xCmd.GET_PROPERTY.DATA[12]   = Pro2Cmd[12];
    //Si446xCmd.GET_PROPERTY.DATA[13]   = Pro2Cmd[13];
    //Si446xCmd.GET_PROPERTY.DATA[14]   = Pro2Cmd[14];
    //Si446xCmd.GET_PROPERTY.DATA[15]   = Pro2Cmd[15];

	for(int i=0; i<NUM_PROPS; i++) {
		out[i] = Pro2Cmd[i];
	}

	return Pro2Cmd[0];
}

/*!
 * Performs image rejection calibration. Completion can be monitored by polling CTS or waiting for CHIP_READY interrupt source.
 *
 * @param SEARCHING_STEP_SIZE
 * @param SEARCHING_RSSI_AVG
 * @param RX_CHAIN_SETTING1
 * @param RX_CHAIN_SETTING2
 */
void si446x_ircal(U8 SEARCHING_STEP_SIZE, U8 SEARCHING_RSSI_AVG, U8 RX_CHAIN_SETTING1, U8 RX_CHAIN_SETTING2)
{
    Pro2Cmd[0] = SI446X_CMD_ID_IRCAL;
    Pro2Cmd[1] = SEARCHING_STEP_SIZE;
    Pro2Cmd[2] = SEARCHING_RSSI_AVG;
    Pro2Cmd[3] = RX_CHAIN_SETTING1;
    Pro2Cmd[4] = RX_CHAIN_SETTING2;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_IRCAL, Pro2Cmd);
}

/*!
 * Receives information from the radio of the current packet. Optionally can be used to modify
 * the Packet Handler properties during packet reception.
 *
 * @param FIELD_NUMBER_MASK Packet Field number mask value.
 * @param LEN               Length value.
 * @param DIFF_LEN          Difference length.
 */
int si446x_get_packet_info(U8 FIELD_NUMBER_MASK, U16 LEN, int16_t DIFF_LEN )
{
    Pro2Cmd[0] = SI446X_CMD_ID_PACKET_INFO;
    Pro2Cmd[1] = FIELD_NUMBER_MASK;
    Pro2Cmd[2] = (U8)(LEN >> 8);
    Pro2Cmd[3] = (U8)(LEN);
    // the different of the byte, althrough it is signed, but to command hander
    // it can treat it as unsigned
    Pro2Cmd[4] = (U8)((U16)DIFF_LEN >> 8);
    Pro2Cmd[5] = (U8)(DIFF_LEN);

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_PACKET_INFO,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_PACKET_INFO,
                              Pro2Cmd );

    Si446xCmd.PACKET_INFO.LENGTH = ((U16)Pro2Cmd[0] << 8) & 0xFF00;
    Si446xCmd.PACKET_INFO.LENGTH |= (U16)Pro2Cmd[1] & 0x00FF;

	return Si446xCmd.PACKET_INFO.LENGTH;
}

// END SI446x
// --------------------------------------------------------
// RADIO COMMON

void radio_comm_SendCmd(unsigned byteCount, const uint8_t* pData) {
	radio_comm_PollCTS();
	radio_spi_sel_assert();
	spi_write_bytes(byteCount, pData);
	radio_spi_sel_no_assert();
	ctsWentHigh = 0;
}

/*!
 * Gets a command response from the radio chip
 *
 * @param cmd           Command ID
 * @param pollCts       Set to poll CTS
 * @param byteCount     Number of bytes to get from the radio chip
 * @param pData         Pointer to where to put the data
 */
void radio_comm_WriteData(uint8_t cmd, unsigned pollCts, uint8_t byteCount, uint8_t* pData)
{
    if(pollCts)
    {
        while(!ctsWentHigh)
        {
            radio_comm_PollCTS();
        }
    }
	radio_spi_sel_assert();
	radio_spi_go(cmd);
	spi_write_bytes(byteCount, pData);
	radio_spi_sel_no_assert();
	
    //radio_hal_ClearNsel();
    //radio_hal_SpiWriteByte(cmd);
    //radio_hal_SpiWriteData(byteCount, pData);
    //radio_hal_SetNsel();
	
    ctsWentHigh = 0;
}

void radio_comm_ReadData(uint8_t cmd, unsigned pollCts, uint8_t byteCount, uint8_t* pData) {
	if (pollCts) { // Not sure of case where you wouldn't want to do this
		radio_comm_PollCTS();
	}
	radio_spi_sel_assert();
	radio_spi_go(cmd);
	spi_read_bytes(byteCount, pData);
	radio_spi_sel_no_assert();
	ctsWentHigh = 0;
}

uint8_t radio_comm_GetResp(uint8_t byteCount, uint8_t* pData) {
	unsigned ctsVal;
	unsigned timeout=0;
	
	radio_spi_sel_assert();
	radio_spi_go(0x44); //read CMD buffer
	ctsVal = radio_spi_go(0);

	while(ctsVal != 0xFF && timeout++ <= CTS_TIMEOUT) {
		radio_spi_sel_no_assert();
		//for(unsigned i=0; i<CTS_WAIT; i++) ; // spin
		// Looking for at least 150ns, or likely even half that would be enough.
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		radio_spi_sel_assert();
		radio_spi_go(0x44); //read CMD buffer
		ctsVal = radio_spi_go(0);
	}
	
	if (ctsVal != 0xFF) {
		radio_spi_sel_no_assert();
		SI_ASSERT(0, "Fatal: CTS Timeout waiting for response\r\n");
		return 0;
	}
	else {
		ctsWentHigh = 1;
	}
	
	if (byteCount) {
		spi_read_bytes(byteCount, pData);
	}
	radio_spi_sel_no_assert();
	
	return ctsVal;
}

unsigned int radio_comm_PollCTS() {
	unsigned timeout=0;

#ifdef SI446X_AGGRESSIVE_CTS
	if (ctsWentHigh) return 1;
#else
	ctsWentHigh = 0;
#endif

	while(ctsWentHigh == 0 && timeout < CTS_TIMEOUT) {
		radio_comm_GetResp(0, NULL);
		timeout++;
	}
	if (timeout == CTS_TIMEOUT) {
		ctsWentHigh = 0;
		SI_ASSERT(0, "Fatal: CTS Timeout waiting for response\r\n");
		return 0;
	}

	return 1;
}

uint8_t radio_comm_SendCmdGetResp(uint8_t cmdByteCount, uint8_t* pCmdData, uint8_t respByteCount, uint8_t* pRespData) {
    radio_comm_SendCmd(cmdByteCount, pCmdData);
    return radio_comm_GetResp(respByteCount, pRespData);
}

// END RADIO COMMON
// --------------------------------------------------------
// RADIO HAL


// END RADIO HAL
