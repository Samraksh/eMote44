#include "RF231.h"
#include <tinyhal.h>
#include <stm32f10x.h> // TODO. FIX ME. Only needed for interrupt pin check and NOPs. Not platform independant.

#define TIME_OPTIMIZED_TRANSMIT
#ifndef TIME_OPTIMIZED_TRANSMIT
#define WRITE_THEN_TRANSMIT
#endif
//#define RF231_EXTENDED_MODE

RF231Radio grf231Radio;
RF231Radio grf231RadioLR;

BOOL GetCPUSerial(UINT8 * ptr, UINT16 num_of_bytes ){
	UINT32 Device_Serial0;UINT32 Device_Serial1; UINT32 Device_Serial2;
	Device_Serial0 = *(UINT32*)(0x1FFFF7E8);
	Device_Serial1 = *(UINT32*)(0x1FFFF7EC);
	Device_Serial2 = *(UINT32*)(0x1FFFF7F0);
	if(num_of_bytes==12){
	    ptr[0] = (UINT8)(Device_Serial0 & 0x000000FF);
	    ptr[1] = (UINT8)((Device_Serial0 & 0x0000FF00) >> 8);
	    ptr[2] = (UINT8)((Device_Serial0 & 0x00FF0000) >> 16);
	    ptr[3] = (UINT8)((Device_Serial0 & 0xFF000000) >> 24);

	    ptr[4] = (UINT8)(Device_Serial1 & 0x000000FF);
	    ptr[5] = (UINT8)((Device_Serial1 & 0x0000FF00) >> 8);
	    ptr[6] = (UINT8)((Device_Serial1 & 0x00FF0000) >> 16);
	    ptr[7] = (UINT8)((Device_Serial1 & 0xFF000000) >> 24);

	    ptr[8] = (UINT8)(Device_Serial2 & 0x000000FF);
	    ptr[9] = (UINT8)((Device_Serial2 & 0x0000FF00) >> 8);
	    ptr[10] = (UINT8)((Device_Serial2 & 0x00FF0000) >> 16);
	    ptr[11] = (UINT8)((Device_Serial2 & 0xFF000000) >> 24);
	    return TRUE;
	}
	else return FALSE;

}

//#define NATHAN_DEBUG_JUNK
#ifdef NATHAN_DEBUG_JUNK
static uint64_t send_ts_times[16];
static uint64_t send_times[16];
static volatile uint64_t rx_times[16];
static volatile uint64_t send_dones[16];
static volatile uint64_t rx_start_times[16];
static uint64_t sleep_times[16];
static volatile uint64_t irq_times[16];
static volatile uint64_t irq_causes[16];
static volatile uint32_t size_log[32];
static volatile uint64_t trx_ur_times[16];
static volatile uint64_t download_error_times[16];
static volatile uint64_t int_pend_times[16];

static void add_size(uint32_t size) {
	static int index=0;
	size_log[index] = size;
	index = (++index) % 32;
}

static void add_irq_time(uint32_t cause) {
	static int index=0;
	irq_times[index] = HAL_Time_CurrentTicks();
	irq_causes[index] = cause;
	index = (++index) % 16;
}

static void add_sleep_time() {
	static int index=0;
	sleep_times[index] = HAL_Time_CurrentTicks();
	index = (++index) % 16;
}

static void add_rx_start_time() {
	static int index=0;
	rx_start_times[index] = HAL_Time_CurrentTicks();
	index = (++index) % 16;
}

static void add_send_ts_time() {
	static int index=0;
	send_ts_times[index] = HAL_Time_CurrentTicks();
	index = (++index) % 16;
}

static void add_send_time() {
	static int index=0;
	send_times[index] = HAL_Time_CurrentTicks();
	index = (++index) % 16;
}

static void add_rx_time() {
	static int index=0;
	rx_times[index] = HAL_Time_CurrentTicks();
	index = (++index) % 16;
}

static void add_send_done() {
	static int index=0;
	send_dones[index] = HAL_Time_CurrentTicks();
	index = (++index) % 16;
}

static void add_trx_ur() {
	static int index=0;
	trx_ur_times[index] = HAL_Time_CurrentTicks();
	index = (++index) % 16;
}

static void add_download_error() {
	static int index=0;
	download_error_times[index] = HAL_Time_CurrentTicks();
	index = (++index) % 16;
}
static void add_int_pend() {
	static int index=0;
	int_pend_times[index] = HAL_Time_CurrentTicks();
	index = (++index) % 16;
}
#else
static void add_size(uint32_t size) { }
static void add_irq_time(uint32_t cause) { }
static void add_sleep_time() { }
static void add_rx_start_time() { }
static void add_send_ts_time() { }
static void add_send_time() { }
static void add_rx_time() { }
static void add_send_done() { }
static void add_trx_ur() { }
static void add_download_error() { }
static void add_int_pend() { }
#endif

static inline BOOL isInterrupt()
{
    return ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0);
}

static void interrupt_mode_check() {
#ifdef NDEBUG
	return;
#else
	if (isInterrupt()) {
		//hal_printf("Warning: RF231 driver called from interrupt context\r\n");
	}
	else return;
#endif
}

void* RF231Radio::Send_Ack(void *msg, UINT16 size, NetOpStatus status, UINT8 tracStatus) {
	////SendAckFuncPtrType AckHandler = Radio<Message_15_4_t>::GetMacHandler(active_mac_index)->GetSendAckHandler();
	////(*AckHandler)(msg, size, status);
	(Radio_event_handler.GetSendAckHandler())(msg, size, status, tracStatus);
	if (status != NetworkOperations_Success) return NULL;
	else return msg;
}


void RF231Radio::Wakeup() {
	INIT_STATE_CHECK();
	GLOBAL_LOCK(irq);
	if (state == STATE_SLEEP) {
		if(RF231_extended_mode){
			SlptrClear();
			HAL_Time_Sleep_MicroSeconds(380); // Wait for the radio to come out of sleep
			DID_STATE_CHANGE_ASSERT(RF230_TRX_OFF);
			ENABLE_LRR(TRUE);
			state = STATE_TRX_OFF;
		}
		else{
			SlptrClear();
			HAL_Time_Sleep_MicroSeconds(380); // Wait for the radio to come out of sleep
			DID_STATE_CHANGE_ASSERT(RF230_TRX_OFF);
			ENABLE_LRR(TRUE);
			state = STATE_TRX_OFF;
		}
	}
}

// For when interrupts are disabled. Checks radio IRQ pin.
BOOL RF231Radio::Interrupt_Pending() {
#ifndef NATHAN_DEBUG_JUNK
	return (EXTI_GetITStatus( 1<<(kinterrupt % 16) ) == SET ) ? TRUE : FALSE;
#else
	if (EXTI_GetITStatus(1<<(kinterrupt % 16)) == SET) {
		add_int_pend();
		return TRUE;
	}
	else {
		return FALSE;
	}
#endif
}

// Not for use with going to BUSY_TX, etc.
// On success, caller must change 'state'
BOOL RF231Radio::Careful_State_Change(uint32_t target) { return Careful_State_Change( (radio_hal_trx_status_t) target ); }
BOOL RF231Radio::Careful_State_Change(radio_hal_trx_status_t target) {

	uint32_t poll_counter=0;
	const uint32_t timeout = 0xFFFF;

	GLOBAL_LOCK(irq);

	Wakeup();

	// current status
	radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	radio_hal_trx_status_t orig_status = trx_status;

	ASSERT_RADIO(trx_status != P_ON); // P_ON is 0 and normally impossible to reach after init. So likely SPI died.

	if (target == trx_status) { return TRUE; } // already there!
	if (target == RX_ON && trx_status == BUSY_RX) { return TRUE; } // BUSY_RX and RX_ON are equiv.

	// Make sure we're not busy and can move.
	if ( trx_status == BUSY_RX || trx_status == BUSY_TX || Interrupt_Pending() )
	{
		return FALSE;
	}

	WriteRegister(RF230_TRX_STATE, target); // do the move

	do{
		trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
		if( poll_counter == timeout || (trx_status != orig_status \
				&& trx_status != target \
				&& trx_status != STATE_TRANSITION_IN_PROGRESS \
				&& !(trx_status == PLL_ON && target == RX_ON)) ) // Don't ask me why... but this seems to be a thing. Revisit. --NPS
			{
				switch(trx_status) {
					case BUSY_RX: state = STATE_BUSY_RX; break;
					case BUSY_TX: state = STATE_BUSY_TX; break;
					default: state = STATE_PLL_ON; ASSERT_RADIO(0); // Unknown. Put here just because.
				}
				return FALSE;
			}
		poll_counter++;
	} while(trx_status != target);

	// Check one last time for interrupt.
	// Not clear how this could happen, but assume it would be an RX. --NPS
	if ( Interrupt_Pending() ) { state = STATE_BUSY_RX; return FALSE; } // Not an error, just odd
	
	// Reset cmd here just to be clean.
	if ( trx_status == PLL_ON || trx_status == TRX_OFF )
	{
		cmd = CMD_NONE;
	}

	// We made it!
	return TRUE;
}


BOOL RF231Radio::Careful_State_Change_Extended(uint32_t target) { return Careful_State_Change_Extended( (radio_hal_trx_status_t) target ); }
BOOL RF231Radio::Careful_State_Change_Extended(radio_hal_trx_status_t target) {

	volatile uint32_t poll_counter=0;
	const uint32_t timeout = 0xFFFF;

	GLOBAL_LOCK(irq);

	Wakeup();

	// current status
	radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	radio_hal_trx_status_t orig_status = trx_status;

	ASSERT_RADIO(trx_status != P_ON); // P_ON is 0 and normally impossible to reach after init. So likely SPI died.

	if (target == trx_status) { return TRUE; } // already there!
	if (target == RX_AACK_ON && trx_status == BUSY_RX_AACK) { return TRUE; } // BUSY_RX_AACK and RX_AACK_ON are equiv.

	// Make sure we're not busy and can move.
	if ( trx_status == BUSY_RX_AACK || trx_status == BUSY_TX_ARET || Interrupt_Pending() )
	{
		return FALSE;
	}

	//if(target == RX_AACK_ON){
		WriteRegister(RF230_TRX_STATE, PLL_ON); // do the move

		do{
			trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			if( poll_counter == timeout || (trx_status != orig_status \
					&& trx_status != PLL_ON \
					&& trx_status != STATE_TRANSITION_IN_PROGRESS \
					&& !(trx_status == PLL_ON && target == RX_AACK_ON)) ) // Don't ask me why... but this seems to be a thing. Revisit. --NPS
				{
					switch(trx_status) {
						case BUSY_RX_AACK:
							state = STATE_BUSY_RX_AACK;
							break;
						case BUSY_TX_ARET:
							state = STATE_BUSY_TX_ARET;
							break;
						default:
							state = STATE_PLL_ON;
							ASSERT_RADIO(0); // Unknown. Put here just because.
					}
					//SOFT_BREAKPOINT();
					return FALSE;
				}
			poll_counter++;
		} while(trx_status != PLL_ON);
	//}

	//if(target == TX_ARET_ON){
		WriteRegister(RF230_TRX_STATE, target); // do the move

		do{
			trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			if( poll_counter == timeout || (trx_status != orig_status \
					&& trx_status != target \
					&& trx_status != STATE_TRANSITION_IN_PROGRESS \
					&& !(trx_status == TX_ARET_ON && target == RX_AACK_ON)) ) // Don't ask me why... but this seems to be a thing. Revisit. --NPS
				{
					switch(trx_status) {
						case BUSY_RX_AACK:
							state = STATE_BUSY_RX_AACK;
							break;
						case BUSY_TX_ARET:
							state = STATE_BUSY_TX_ARET;
							break;
						default:
							state = STATE_PLL_ON;
							ASSERT_RADIO(0); // Unknown. Put here just because.
					}
					SOFT_BREAKPOINT();
					return FALSE;
				}
			poll_counter++;
		} while(trx_status != target);
	//}

	// Check one last time for interrupt.
	// Not clear how this could happen, but assume it would be an RX. --NPS
	if ( Interrupt_Pending() ) {
		state = STATE_BUSY_RX_AACK;
		SOFT_BREAKPOINT();
		return FALSE;
	} // Not an error, just odd

	// Reset cmd here just to be clean.
	if ( trx_status == PLL_ON || trx_status == TRX_OFF )
	{
		cmd = CMD_NONE;
	}

	// We made it!
	return TRUE;
}

Message_15_4_t* RF231Radio::Preload(Message_15_4_t* msg, UINT16 size){
	const int crc_size = 2;
	// Adding 2 for crc
	if(size + crc_size > IEEE802_15_4_FRAME_LENGTH){
		return NULL;
	}

	radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	if(trx_status == BUSY_TX || trx_status == BUSY_TX_ARET || trx_status == TX_ARET_ON){
		return NULL;
	}
	memcpy(grf231Radio.tx_msg_ptr, msg, size);
	return msg;
}

void* RF231Radio::SendStrobe(UINT16 size)
{
	UINT32 eventOffset;
	UINT32 timestamp;

	UINT8 lLength;
	UINT8 *timeStampPtr;
	UINT8 *ldata;

	Message_15_4_t* temp;

	const int crc_size = 2;

	IEEE802_15_4_Header_t *header = (IEEE802_15_4_Header_t*)tx_msg_ptr->GetHeader();
	sequenceNumberSender = header->dsn;

	// Adding 2 for crc
	if(size + crc_size > IEEE802_15_4_FRAME_LENGTH){
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_BadPacket, TRAC_STATUS_FAIL_TO_SEND);
	}

	if ( !IsInitialized() ) {
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Fail, TRAC_STATUS_FAIL_TO_SEND);
	}

	interrupt_mode_check();

	GLOBAL_LOCK(irq);

	add_send_time(); // Debugging. Will remove.

#ifdef DEBUG_RF231
	radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	hal_printf("(Send)trx_status is %d; state is %d\n", trx_status, state);
#endif
	if(RF231_extended_mode){
		// Go to PLL_ON
		if ( Careful_State_Change_Extended(RF230_TX_ARET_ON) ) {
			state = STATE_TX_ARET_ON;
		}
		else {
			radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			hal_printf("(Send)trx_status is %d; state is %d\n", trx_status, state);
			return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
		}
	}
	else{
		// Go to PLL_ON
		if ( Careful_State_Change(RF230_PLL_ON) ) {
			state = STATE_PLL_ON;
		}
		else {
			return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
		}
	}

	tx_length = size;
	ldata = (UINT8*) tx_msg_ptr;

	/***********Start of "Write to SPI and then transmit"*********/
#if defined(WRITE_THEN_TRANSMIT)

	//Start writing data to the SPI bus
	SelnClear();

	CPU_SPI_WriteByte(config, RF230_CMD_FRAME_WRITE);

	// Write the size of packet that is sent out
	// Including FCS which is automatically generated and is two bytes
	//Plus 4 bytes for timestamp
	CPU_SPI_ReadWriteByte(config, size + crc_size);

	lLength = size;

	add_size(size);

	for(int ii=0; ii<lLength; ii++) {
		CPU_SPI_ReadWriteByte(config, *(ldata++));
	}

	CPU_SPI_ReadByte(config);

	//Indicate end of writing to SPI bus
	SelnSet();

	//Check CCA before transmission
	// Go to RX_ON
	if ( Careful_State_Change(RF230_RX_ON) ) {
		state = STATE_RX_ON;
	}
	else {
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
	}
	DeviceStatus ds = ClearChannelAssesment();
	if(ds == DS_Fail){
		hal_printf("send CCA failed\n");
		return NULL;
	}

	// Go to PLL_ON
	if ( Careful_State_Change(RF230_PLL_ON) ) {
		state = STATE_PLL_ON;
	}
	else {
		hal_printf("send Radio state change failed; state is %d\n", state);
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
	}

	// Initiate frame transmission by asserting SLP_TR pin
	SlptrSet();
	HAL_Time_Sleep_MicroSeconds(16);
	SlptrClear();

	/***********End of "Write to SPI and then transmit"*********/


	/***********Start of "Time optimized frame transmit procedure"*********/
#elif defined(TIME_OPTIMIZED_TRANSMIT)
	// Make sure SLP_TR is low before we start.
	SlptrSet();
	HAL_Time_Sleep_MicroSeconds(16);
	SlptrClear();

	//Start writing data to the SPI bus
	SelnClear();

	CPU_SPI_WriteByte(config, RF230_CMD_FRAME_WRITE);

	// Write the size of packet that is sent out
	// Including FCS which is automatically generated and is two bytes
	//Plus 4 bytes for timestamp
	CPU_SPI_ReadWriteByte(config, size + crc_size);

	lLength = size;

	add_size(size);

	for(int ii=0; ii<lLength; ii++) {
		CPU_SPI_ReadWriteByte(config, *(ldata++));
	}

	CPU_SPI_ReadByte(config);

	//Indicate end of writing to SPI bus
	SelnSet();
#endif
	/***********End of "Time optimized frame transmit procedure"*********/

	if(RF231_extended_mode){
		state = STATE_BUSY_TX_ARET;
		cmd = CMD_TX_ARET;
	}
	else{
		state = STATE_BUSY_TX;
		cmd = CMD_TRANSMIT;
	}

#ifdef DEBUG_RF231
	hal_printf("(Send)Finished send\n");
#endif
	return tx_msg_ptr;
}

void* RF231Radio::Send_TimeStamped(void* msg, UINT16 size, UINT32 eventTime)
{
	UINT32 eventOffset;
	UINT32 timestamp;

	UINT8 lLength;
	UINT8 *timeStampPtr;
	UINT8 *ldata;

	Message_15_4_t* temp = NULL;

	//static int busyRxCounter = 0;

	const int timestamp_size = 4; // we decrement in a loop later.
	const int crc_size = 2;

	IEEE802_15_4_Header_t *header = (IEEE802_15_4_Header_t*)tx_msg_ptr->GetHeader();
	sequenceNumberSender = header->dsn;

	// Adding 2 for crc and 4 bytes for timestamp
	if(size + crc_size + timestamp_size > IEEE802_15_4_FRAME_LENGTH){
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_BadPacket, TRAC_STATUS_FAIL_TO_SEND);
	}

	if ( !IsInitialized() ) {
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Fail, TRAC_STATUS_FAIL_TO_SEND);
	}

	interrupt_mode_check();

	GLOBAL_LOCK(irq);

	add_send_ts_time(); // Debugging. Will remove.

#ifdef DEBUG_RF231
	radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	hal_printf("(Send_TimeStamped)trx_status is %d; state is %d\n", trx_status, state);
#endif
	if(RF231_extended_mode){
		// Go to PLL_ON
		if ( Careful_State_Change_Extended(RF230_TX_ARET_ON) ) {
			state = STATE_TX_ARET_ON;
		}
		else {
			//CPU_GPIO_SetPinState( FAST_RECOVERY_SEND, TRUE );
			//CPU_GPIO_SetPinState( FAST_RECOVERY_SEND, FALSE );
			/*radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
#ifdef DEBUG_RF231
			hal_printf("(Send_TimeStamped)trx_status is %d; state is %d\n", trx_status, state);
#endif
			if(trx_status == BUSY_RX_AACK){
				busyRxCounter++;
				//ASSERT_RADIO(busyRxCounter < 500);
			}
			else{
				busyRxCounter = 0;
			}*/

			//CPU_GPIO_SetPinState( FAST_RECOVERY_SEND, TRUE );
			//CPU_GPIO_SetPinState( FAST_RECOVERY_SEND, FALSE );
			return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
		}
	}
	else{
		// Go to PLL_ON
		if ( Careful_State_Change(RF230_PLL_ON) ) {
			state = STATE_PLL_ON;
		}
		else {
			return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
		}
	}

	tx_length = size;
	ldata = (UINT8*) msg;


	/***********Start of "Write to SPI and then transmit"*********/
#if defined(WRITE_THEN_TRANSMIT)

	//Start writing data to the SPI bus
	SelnClear();

	CPU_SPI_WriteByte(config, RF230_CMD_FRAME_WRITE);

	// Write the size of packet that is sent out
	// Including FCS which is automatically generated and is two bytes
	//Plus 4 bytes for timestamp
	CPU_SPI_ReadWriteByte(config, size + crc_size + timestamp_size);

	lLength = size;
	timeStampPtr = (UINT8 *) &eventOffset;

	add_size(size);

	for(int ii=0; ii<lLength; ii++) {
		CPU_SPI_ReadWriteByte(config, *(ldata++));
	}

	//Transmit the event timestamp
	timestamp = HAL_Time_CurrentTicks() & 0xFFFFFFFF; // Lower bits only
	eventOffset = timestamp - eventTime;

	for(int ii=0; ii<timestamp_size; ii++) {
		CPU_SPI_ReadWriteByte(config, *(timeStampPtr++));
	}

	CPU_SPI_ReadByte(config);

	//Indicate end of writing to SPI bus
	SelnSet();

	//Check CCA before transmission
	// Go to RX_ON
	if ( Careful_State_Change(RF230_RX_ON) ) {
		state = STATE_RX_ON;
	}
	else {
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
	}
	DeviceStatus ds = ClearChannelAssesment();
	if(ds == DS_Fail){
		hal_printf("sendTS CCA failed\n");
		return NULL;
	}

	// Go to PLL_ON
	if ( Careful_State_Change(RF230_PLL_ON) ) {
		state = STATE_PLL_ON;
	}
	else {
		hal_printf("SendTS Radio state change failed; state is %d\n", state);
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
	}

	// Initiate frame transmission by asserting SLP_TR pin
	SlptrSet();
	HAL_Time_Sleep_MicroSeconds(16);
	SlptrClear();

	/***********End of "Write to SPI and then transmit"*********/


	/***********Start of "Time optimized frame transmit procedure"*********/
#elif defined(TIME_OPTIMIZED_TRANSMIT)
	// Make sure SLP_TR is low before we start.
	SlptrSet();
	HAL_Time_Sleep_MicroSeconds(16);
	SlptrClear();

	//Start writing data to the SPI bus
	SelnClear();

	//CPU_GPIO_SetPinState( RF231_TX_TIMESTAMP, TRUE );
	timestamp = HAL_Time_CurrentTicks() & 0xFFFFFFFF; // Lower bits only
	//CPU_GPIO_SetPinState( RF231_TX_TIMESTAMP, FALSE );
	CPU_SPI_WriteByte(config, RF230_CMD_FRAME_WRITE);

	// Write the size of packet that is sent out
	// Including FCS which is automatically generated and is two bytes
	//Plus 4 bytes for timestamp
	CPU_SPI_ReadWriteByte(config, size + crc_size + timestamp_size);

	lLength = size;
	timeStampPtr = (UINT8 *) &eventOffset;

	add_size(size);

	for(int ii=0; ii<lLength; ii++) {
		CPU_SPI_ReadWriteByte(config, *(ldata++));
	}

	//Transmit the event timestamp
	//timestamp = HAL_Time_CurrentTicks() & 0xFFFFFFFF; // Lower bits only
	eventOffset = timestamp - eventTime;

	for(int ii=0; ii<timestamp_size; ii++) {
		CPU_SPI_ReadWriteByte(config, *(timeStampPtr++));
	}

	CPU_SPI_ReadByte(config);

	//CPU_GPIO_SetPinState( RF231_TX_TIMESTAMP, TRUE );
	//CPU_GPIO_SetPinState( RF231_TX_TIMESTAMP, FALSE );
	//Indicate end of writing to SPI bus
	SelnSet();
#endif
	/***********End of "Time optimized frame transmit procedure"*********/

	if(RF231_extended_mode){
		state = STATE_BUSY_TX_ARET;
		cmd = CMD_TX_ARET;
	}
	else{
		state = STATE_BUSY_TX;
		cmd = CMD_TRANSMIT;
	}

	// exchange bags
	tx_msg_ptr = (Message_15_4_t*) msg;

#ifdef DEBUG_RF231
	hal_printf("RF231Radio::Send_TimeStamped - sent successfully\n");
#endif

	return tx_msg_ptr;
}

void* RF231Radio::SendRetry(){
	if ( !IsInitialized() ) {
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Fail, TRAC_STATUS_FAIL_TO_SEND);
	}

	interrupt_mode_check();

	GLOBAL_LOCK(irq);

	if(RF231_extended_mode){
		// Go to TX_ARET_ON
		if ( Careful_State_Change_Extended(RF230_TX_ARET_ON) ) {
			state = STATE_TX_ARET_ON;
		}
		else {
			radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
#ifdef DEBUG_RF231
			hal_printf("(Send_TimeStamped)trx_status is %d; state is %d\n", trx_status, state);
#endif
			return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
		}
	}
	else{
		// Go to PLL_ON
		if ( Careful_State_Change(RF230_PLL_ON) ) {
			state = STATE_PLL_ON;
		}
		else {
			return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
		}
	}

	// Toggle SLP_TR pin to indicate radio to start TX
	SlptrSet();
	//HAL_Time_Sleep_MicroSeconds(16);
	SlptrClear();
}

DeviceStatus RF231Radio::EnableCSMA(){
	//Page 73 in RF231 datasheet
	//Bits 7-4 - MAX_FRAME_RETRIES - being set to 0 (0000)
	//Bits 3-1 - MAX_CSMA_RETRIES - being set to 7 (000)
	//Bit 0 - SLOTTED_OPERATION - set to 0
	//Register XAH_CTRL_1 is 0x2C
	//0000 0000
	WriteRegister(RF230_XAH_CTRL_0, 0x00);
}

DeviceStatus RF231Radio::DisableCSMA(){
	//Page 73 in RF231 datasheet
	//Bits 7-4 - MAX_FRAME_RETRIES - being set to 0 (0000)
	//Bits 3-1 - MAX_CSMA_RETRIES - being set to 7 (111)
	//Bit 0 - SLOTTED_OPERATION - set to 0
	//Register XAH_CTRL_1 is 0x2C
	//0000 1110
	WriteRegister(RF230_XAH_CTRL_0, 0x0E);
}

DeviceStatus RF231Radio::Reset()
{
	INIT_STATE_CHECK()

	interrupt_mode_check();

	GLOBAL_LOCK(irq);

	// The radio intialization steps in the following lines are semantically copied from the corresponding tinyos implementation
	// Specified in the datasheet a sleep of 510us
	// The performance of this function is good but at times its found to generate different times. Its possible that there were other
	// events happening on the pin that was used to measure this or there is a possible bug !!!
	HAL_Time_Sleep_MicroSeconds(510);

	// Clear rstn pin
	RstnClear();

	// Clear the slptr pin
	SlptrClear();

	// Sleep for 6us
	HAL_Time_Sleep_MicroSeconds(6);

	RstnSet();

	// The RF230_TRX_CTRL_0 register controls the drive current of the digital output pads and the CLKM clock rate
	// Setting value to 0
	WriteRegister(RF230_TRX_CTRL_0, RF231_TRX_CTRL_0_VALUE);

	UINT32 reg = 0;
	reg = ReadRegister(RF230_TRX_CTRL_0);

	// The RF230_TRX_STATE register controls the state transition
	WriteRegister(RF230_TRX_STATE, RF230_TRX_OFF);

	// Nived.Sivadas - Hanging in the initialize function caused by the radio being in an unstable state
	// This fix will return false from initialize and enable the user of the function to exit gracefully
	// Fix for the hanging in the initialize function
	DID_STATE_CHANGE_ASSERT(RF230_TRX_OFF);

	HAL_Time_Sleep_MicroSeconds(510);

	if(RF231_extended_mode){
		// Register controls the interrupts that are currently enabled
		WriteRegister(RF230_IRQ_MASK, RF230_IRQ_TRX_UR | RF230_IRQ_AMI | RF230_IRQ_TRX_END | RF230_IRQ_RX_START);
	}
	else{
		WriteRegister(RF230_IRQ_MASK, RF230_IRQ_TRX_UR | RF230_IRQ_TRX_END | RF230_IRQ_RX_START);
	}

	// The RF230_CCA_THRES sets the ed level for CCA, currently setting threshold to 0xc7
	WriteRegister(RF230_CCA_THRES, RF230_CCA_THRES_VALUE);

	// Controls output power and ramping of the transistor
	WriteRegister(RF230_PHY_TX_PWR, RF230_TX_AUTO_CRC_ON | (0 & RF230_TX_PWR_MASK));
	// Nived.Sivadas - turning off auto crc check
	//WriteRegister(RF230_PHY_TX_PWR, 0 | (0 & RF230_TX_PWR_MASK));
	tx_power = 0 & RF230_TX_PWR_MASK;
	channel = RF230_DEF_CHANNEL & RF230_CHANNEL_MASK;

	// Sets the channel number
	WriteRegister(RF230_PHY_CC_CCA, RF230_CCA_MODE_VALUE | channel);

	// Enable the gpio pin as the interrupt point
	if(this->GetRadioName() == RF231RADIO)
		CPU_GPIO_EnableInputPin(INTERRUPT_PIN, FALSE, Radio_Handler, GPIO_INT_EDGE_HIGH, RESISTOR_DISABLED);
	else if(this->GetRadioName() == RF231RADIOLR)
		CPU_GPIO_EnableInputPin(INTERRUPT_PIN_LR, FALSE, Radio_Handler_LR, GPIO_INT_EDGE_HIGH, RESISTOR_DISABLED);


	SlptrSet();

	// set software state machine state to sleep
	state = STATE_SLEEP;
#	ifdef DEBUG_RF231
	hal_printf("RF231: RESET\r\n");
#	endif

	cmd = CMD_NONE;
	return DS_Success;
}


UINT32 RF231Radio::GetChannel()
{
	return this->channel;
}


UINT32 RF231Radio::GetTxPower()
{
	return this->tx_power;
}

// Change the power level of the radio
// Always ends in TRX_OFF or SLEEP states.
DeviceStatus RF231Radio::ChangeTxPower(int power) {
	interrupt_mode_check();
	GLOBAL_LOCK(irq);

	//For some reason, radio loses state between init and setting radio state (TxPower for instance).
	//This is a hack until the reason is found out.
	radio_hal_trx_status_t trx_status_tmp = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	if(trx_status_tmp == P_ON){
		if(TRUE != SpiInitialize())
		{
			ASSERT_RADIO(0);
		}
	}

	if(RF231_extended_mode){
		if ( !Careful_State_Change_Extended(RF230_TRX_OFF) || !IsInitialized() ) {
			return DS_Fail; // We were busy.
		}
	}
	else{
		if ( !Careful_State_Change(RF230_TRX_OFF) || !IsInitialized() ) {
			return DS_Fail; // We were busy.
		}
	}
	state = STATE_TRX_OFF;
	this->tx_power = power  & RF230_TX_PWR_MASK;
	WriteRegister(RF230_PHY_TX_PWR, RF230_TX_AUTO_CRC_ON | (power & RF230_TX_PWR_MASK));

	if (sleep_pending) { Sleep(0); } // I disagree. --NPS

	return DS_Success;
}

// Change the power level of the radio
// Always ends in TRX_OFF or SLEEP states.
DeviceStatus RF231Radio::ChangeChannel(int channel) {
	interrupt_mode_check();
	GLOBAL_LOCK(irq);

	//For some reason, radio loses state between init and setting radio state (TxPower for instance).
	//This is a hack until the reason is found out.
	radio_hal_trx_status_t trx_status_tmp = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	if(trx_status_tmp == P_ON){
		if(TRUE != SpiInitialize())
		{
			ASSERT_RADIO(0);
		}
	}

	if(RF231_extended_mode){
		if ( !Careful_State_Change_Extended(RF230_TRX_OFF) || !IsInitialized() ) {
			return DS_Fail; // We were busy.
		}
	}
	else{
		if ( !Careful_State_Change(RF230_TRX_OFF) || !IsInitialized() ) {
			return DS_Fail; // We were busy.
		}
	}
	state = STATE_TRX_OFF;
	// The value passed as channel until this point is an enum and needs to be offset by 11 to set the
	// actual radio channel value
	this->channel = (channel + RF230_CHANNEL_OFFSET) & RF230_CHANNEL_MASK;
	WriteRegister(RF230_PHY_CC_CCA, RF230_CCA_MODE_VALUE | this->channel);

	if (sleep_pending) { Sleep(0); } // I disagree. --NPS

	return DS_Success;
}

// Nathan's re-write of sleep function
DeviceStatus RF231Radio::Sleep(int level)
{
	// Initialize state change check variables
	// Primarily used if DID_STATE_CHANGE_ASSERT is used
	INIT_STATE_CHECK();
	UINT32 regState;

	GLOBAL_LOCK(irq);
	sleep_pending = TRUE;
	//irq.Probe();

#ifdef DEBUG_RF231
	hal_printf("RF231Radio::Sleep: before state:%d\n", state);
#endif
	// If we are already in sleep state do nothing
	// Unsure if during sleep we can read registers (No, you can't --NPS).
	if(state == STATE_SLEEP) {
		return DS_Success;
	}

	// Queue the sleep request if we think we are busy.
	//if(state == STATE_BUSY_TX || state == STATE_BUSY_RX) { return DS_Success; } // This is now done in Careful_State_Change()

	//For some reason, radio loses state between init and setting radio state (TxPower for instance).
	//This is a hack until the reason is found out.
	radio_hal_trx_status_t trx_status_tmp = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	if(trx_status_tmp == P_ON){
		if(TRUE != SpiInitialize())
		{
			ASSERT_RADIO(0);
		}
		WriteRegister(RF230_TRX_STATE, RF230_TRX_OFF);
		return DS_Success; // state is P_ON, might need to wait before it changes from P_ON.
	}

	if(RF231_extended_mode){
		if ( Careful_State_Change_Extended(RF230_TRX_OFF) ) {
			state = STATE_TRX_OFF;
		}
		else { // If we are busy that's OK, the sleep request is still queued.
			return DS_Success;
		}
	}
	else{
		// Go to TRX_OFF
		if ( Careful_State_Change(RF230_TRX_OFF) ) {
			state = STATE_TRX_OFF;
		}
		else { // If we are busy that's OK, the sleep request is still queued.
			return DS_Success;
		}
	}

	// Read current state of radio to be sure. --Update, I'm actually sure now, but we'll leave it. --NPS
	regState = (VERIFY_STATE_CHANGE);

	// Observe that I am trying, perhaps stupidly, to be clever with fall-through here... --NPS
	switch(regState) {
		case RF230_RX_ON:
		case RF230_PLL_ON:
		case RF230_P_ON:
			WriteRegister(RF230_TRX_STATE, RF230_TRX_OFF);
			DID_STATE_CHANGE_ASSERT(RF230_TRX_OFF);
		case RF230_TRX_OFF:
			ENABLE_LRR(FALSE);
			SlptrSet();
			state = STATE_SLEEP;
			sleep_pending = TRUE;
#ifdef DEBUG_RF231
			hal_printf("RF231: SLEEP.\r\n");
#endif
			break;
		default:
			sleep_pending = TRUE; // Only sleep from known states, otherwise just try later. Redundant.
	}

	//NATHAN_SET_DEBUG_GPIO(0);

	//CPU_GPIO_SetPinState( RF231_RADIO_STATEPIN2, FALSE );
	add_sleep_time();

#ifdef DEBUG_RF231
	hal_printf("RF231Radio::Sleep: after state:%d\n", state);
#endif
	return DS_Success;
}


void* RF231Radio::Send(void* msg, UINT16 size)
{
	//CPU_GPIO_SetPinState( RF231_TX, TRUE );
	UINT32 eventOffset;
	UINT32 timestamp;

	UINT8 lLength;
	UINT8 *timeStampPtr;
	UINT8 *ldata;

	Message_15_4_t* temp;

	const int crc_size = 2;

	IEEE802_15_4_Header_t *header = (IEEE802_15_4_Header_t*)tx_msg_ptr->GetHeader();
	sequenceNumberSender = header->dsn;

	// Adding 2 for crc
	if(size + crc_size> IEEE802_15_4_FRAME_LENGTH){
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_BadPacket, TRAC_STATUS_FAIL_TO_SEND);
	}

	if ( !IsInitialized() ) {
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Fail, TRAC_STATUS_FAIL_TO_SEND);
	}

	interrupt_mode_check();

	GLOBAL_LOCK(irq);

	add_send_time(); // Debugging. Will remove.

#ifdef DEBUG_RF231
	radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	hal_printf("(Send)trx_status is %d; state is %d\n", trx_status, state);
#endif
	if(RF231_extended_mode){
		// Go to PLL_ON
		if ( Careful_State_Change_Extended(RF230_TX_ARET_ON) ) {
			state = STATE_TX_ARET_ON;
		}
		else {
			radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			hal_printf("(Send)trx_status is %d; state is %d\n", trx_status, state);
			return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
		}
	}
	else{
		// Go to PLL_ON
		if ( Careful_State_Change(RF230_PLL_ON) ) {
			state = STATE_PLL_ON;
		}
		else {
			return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
		}
	}

	tx_length = size;
	ldata =(UINT8*) msg;

	/***********Start of "Write to SPI and then transmit"*********/
#if defined(WRITE_THEN_TRANSMIT)

	//Start writing data to the SPI bus
	SelnClear();

	CPU_SPI_WriteByte(config, RF230_CMD_FRAME_WRITE);

	// Write the size of packet that is sent out
	// Including FCS which is automatically generated and is two bytes
	//Plus 4 bytes for timestamp
	CPU_SPI_ReadWriteByte(config, size + crc_size);

	lLength = size;

	add_size(size);

	for(int ii=0; ii<lLength; ii++) {
		CPU_SPI_ReadWriteByte(config, *(ldata++));
	}

	CPU_SPI_ReadByte(config);

	//Indicate end of writing to SPI bus
	SelnSet();

	//Check CCA before transmission
	// Go to RX_ON
	if ( Careful_State_Change(RF230_RX_ON) ) {
		state = STATE_RX_ON;
	}
	else {
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
	}
	DeviceStatus ds = ClearChannelAssesment();
	if(ds == DS_Fail){
		hal_printf("send CCA failed\n");
		return NULL;
	}

	// Go to PLL_ON
	if ( Careful_State_Change(RF230_PLL_ON) ) {
		state = STATE_PLL_ON;
	}
	else {
		hal_printf("send Radio state change failed; state is %d\n", state);
		return Send_Ack(tx_msg_ptr, tx_length, NetworkOperations_Busy, TRAC_STATUS_FAIL_TO_SEND);
	}

	// Initiate frame transmission by asserting SLP_TR pin
	SlptrSet();
	HAL_Time_Sleep_MicroSeconds(16);
	SlptrClear();

	/***********End of "Write to SPI and then transmit"*********/


	/***********Start of "Time optimized frame transmit procedure"*********/
#elif defined(TIME_OPTIMIZED_TRANSMIT)
	// Make sure SLP_TR is low before we start.
	SlptrSet();
	HAL_Time_Sleep_MicroSeconds(16);
	SlptrClear();

	//Start writing data to the SPI bus
	SelnClear();

	CPU_SPI_WriteByte(config, RF230_CMD_FRAME_WRITE);

	// Write the size of packet that is sent out
	// Including FCS which is automatically generated and is two bytes
	//Plus 4 bytes for timestamp
	CPU_SPI_ReadWriteByte(config, size + crc_size);

	lLength = size;

	add_size(size);

	for(int ii=0; ii<lLength; ii++) {
		CPU_SPI_ReadWriteByte(config, *(ldata++));
	}

	CPU_SPI_ReadByte(config);

	//Indicate end of writing to SPI bus
	SelnSet();
#endif
	/***********End of "Time optimized frame transmit procedure"*********/


	if(RF231_extended_mode){
		state = STATE_BUSY_TX_ARET;
		cmd = CMD_TX_ARET;
	}
	else{
		state = STATE_BUSY_TX;
		cmd = CMD_TRANSMIT;
	}

	// exchange bags
	//temp = tx_msg_ptr;
	tx_msg_ptr = (Message_15_4_t*) msg;

	//CPU_GPIO_SetPinState( RF231_TX, FALSE );

#ifdef DEBUG_RF231
	hal_printf("(Send)Finished send\n");
#endif
	return tx_msg_ptr;
}


DeviceStatus RF231Radio::AntDiversity(BOOL enable)
{
	// only works on the long range radio board
	if(this->GetRadioName() != RF231RADIOLR)
	{
		return DS_Fail;
	}

	UINT8 data = ReadRegister(RF231_REG_ANT_DIV);

	if(enable)
	{
		data |= 0x0C;
	}
	else
	{
		data &= ~(3 << 2);
	}


	WriteRegister(RF231_REG_ANT_DIV, data);

	return DS_Success;
}


DeviceStatus RF231Radio::PARXTX(BOOL enable)
{
	// only works on the long range radio board
	if(this->GetRadioName() != RF231RADIOLR)
	{
		return DS_Fail;
	}

	UINT8 data = ReadRegister(RF231_REG_TX_CTRL_1);

	if(enable)
	{
		data |= 0x80;
	}
	else
	{
		data &= ~(1 << 7);
	}

	WriteRegister(RF231_REG_TX_CTRL_1, data);

	return DS_Success;
}


void RF231Radio::Amp(BOOL TurnOn)
{
	if(this->GetRadioName() != RF231RADIOLR)
	{
		return;
	}

	CPU_GPIO_SetPinState(AMP_PIN_LR, TurnOn);
}


/* AnanthAtSamraksh - adding below change to fix receive on LR radio extension board - 2/11/2014 */

uint8_t  rf231_read_reg(uint8_t reg) {
	uint8_t ret;
	RF231_SEL(0);
	while (SPI_I2S_GetFlagStatus(RF231_SPI, SPI_I2S_FLAG_TXE) == RESET) { ; }
	SPI_I2S_SendData(RF231_SPI, 0x80 | reg);
	while ( SPI_I2S_GetFlagStatus(RF231_SPI, SPI_I2S_FLAG_RXNE) == RESET) {;}
	ret = SPI_I2S_ReceiveData(RF231_SPI);
	while (SPI_I2S_GetFlagStatus(RF231_SPI, SPI_I2S_FLAG_TXE) == RESET) { ; }
	SPI_I2S_SendData(RF231_SPI, 0x00);
	while ( SPI_I2S_GetFlagStatus(RF231_SPI, SPI_I2S_FLAG_RXNE) == RESET) {;}
	ret = SPI_I2S_ReceiveData(RF231_SPI);
	RF231_SEL(1);
	return ret;
}

uint8_t rf231_write_reg(uint8_t reg, uint8_t data) {
	uint8_t ret;
	volatile int i;
	RF231_SEL(0);
	while (SPI_I2S_GetFlagStatus(RF231_SPI, SPI_I2S_FLAG_TXE) == RESET) { ; }
	SPI_I2S_SendData(RF231_SPI, 0xC0 | reg);
	while ( SPI_I2S_GetFlagStatus(RF231_SPI, SPI_I2S_FLAG_RXNE) == RESET) {;}
	ret = SPI_I2S_ReceiveData(RF231_SPI);
	while (SPI_I2S_GetFlagStatus(RF231_SPI, SPI_I2S_FLAG_TXE) == RESET) { ; }
	SPI_I2S_SendData(RF231_SPI, data);
	while ( SPI_I2S_GetFlagStatus(RF231_SPI, SPI_I2S_FLAG_RXNE) == RESET) {;}
	ret = SPI_I2S_ReceiveData(RF231_SPI);
	RF231_SEL(1);
	HAL_Time_Sleep_MicroSeconds(3500);
	return ret;
}

void rf231_enable_pa_rxtx(void) {
	volatile uint8_t data = rf231_read_reg(RF231_REG_TX_CTRL_1);
	data |= 0x80;
	rf231_write_reg(RF231_REG_TX_CTRL_1, data);
}

/* AnanthAtSamraksh */


DeviceStatus RF231Radio::Initialize(RadioEventHandler *event_handler, UINT8 radio, UINT8 mac_id)
{
	INIT_STATE_CHECK();

	interrupt_mode_check();

	/*CPU_GPIO_EnableOutputPin(RF231_HW_ACK_RESP_TIME, TRUE);
	CPU_GPIO_SetPinState(RF231_HW_ACK_RESP_TIME, FALSE);
	CPU_GPIO_EnableOutputPin(RF231_START_OF_RX_MODE, TRUE);
	CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, FALSE);
	CPU_GPIO_EnableOutputPin(CCA_PIN, TRUE);
	CPU_GPIO_SetPinState(CCA_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(RF231_GENERATE_HW_ACK, TRUE);
	CPU_GPIO_SetPinState(RF231_GENERATE_HW_ACK, FALSE);
	CPU_GPIO_EnableOutputPin(RF231_RADIO_STATEPIN2, TRUE);
	CPU_GPIO_SetPinState( RF231_RADIO_STATEPIN2, TRUE );
	CPU_GPIO_SetPinState( RF231_RADIO_STATEPIN2, FALSE );
	CPU_GPIO_EnableOutputPin(FAST_RECOVERY_SEND, TRUE);
	CPU_GPIO_SetPinState( FAST_RECOVERY_SEND, FALSE );

	CPU_GPIO_EnableOutputPin(RF231_RX_START, TRUE);
	CPU_GPIO_SetPinState( RF231_RX_START, FALSE );
	CPU_GPIO_EnableOutputPin(RF231_AMI, TRUE);
	CPU_GPIO_SetPinState( RF231_AMI, FALSE );
	CPU_GPIO_EnableOutputPin(RF231_TRX_RX_END, TRUE);
	CPU_GPIO_SetPinState( RF231_TRX_RX_END, FALSE );
	CPU_GPIO_EnableOutputPin(RF231_TRX_TX_END, TRUE);
	CPU_GPIO_SetPinState( RF231_TRX_TX_END, FALSE );

	//CPU_GPIO_EnableOutputPin(RF231_TURN_ON_RX, FALSE);
	CPU_GPIO_EnableOutputPin(RF231_RX, FALSE);
	CPU_GPIO_EnableOutputPin(RF231_TX_TIMESTAMP, FALSE);
	CPU_GPIO_SetPinState( RF231_TX_TIMESTAMP, FALSE );

	CPU_GPIO_EnableOutputPin(RF231_TX, FALSE);
	CPU_GPIO_EnableOutputPin(RF231_FRAME_BUFF_ACTIVE, FALSE);*/

	/*CPU_GPIO_EnableOutputPin((GPIO_PIN)30, TRUE);
	CPU_GPIO_EnableOutputPin((GPIO_PIN)31, TRUE);
	CPU_GPIO_SetPinState( (GPIO_PIN)30, FALSE );
	CPU_GPIO_SetPinState( (GPIO_PIN)31, FALSE );*/

	//active_mac_index = Radio<Message_15_4_t>::GetMacIdIndex();
	// Set MAC datastructures
	Radio_event_handler.SetReceiveHandler(event_handler->GetReceiveHandler());
	Radio_event_handler.SetSendAckHandler(event_handler->GetSendAckHandler());
	Radio_event_handler.SetRadioInterruptHandler(event_handler->GetRadioInterruptHandler());
	/*if(Radio<Message_15_4_t>::Initialize(event_handler, mac_id) != DS_Success){
		return DS_Fail;
	}*/

	//If the radio hardware is not already initialized, initialize it
	if(!IsInitialized())
	{
		sequenceNumberSender = 0; sequenceNumberReceiver = 0;
		if(CPU_Radio_GetRadioAckType() == HARDWARE_ACK){
			RF231_extended_mode = 1;
		}
		else{
			RF231_extended_mode = 0;
		}

		// Give the radio its name , rf231 or rf231 long range
		this->SetRadioName(radio);

		// Set the corresponding gpio pins
		if(this->GetRadioName() == RF231RADIO)
		{
			kslpTr 		= 	 SLP_TR_PIN;
			krstn 		= 	 RSTN_PIN;
			kseln		= 	 SELN_PIN;
			kinterrupt	= 	 INTERRUPT_PIN;
		}
		else if(this->GetRadioName() == RF231RADIOLR)
		{
			kslpTr		=    SLP_TR_PIN_LR;
			krstn 		= 	 RSTN_PIN_LR;
			kseln		= 	 SELN_PIN_LR;
			kinterrupt	= 	 INTERRUPT_PIN_LR;

			// Enable the amp pin
			//CPU_GPIO_EnableOutputPin(AMP_PIN_LR, FALSE);
		}

		//Get cpu serial and hash it to use as node id
		UINT8 cpuserial[12];
		GetCPUSerial(cpuserial,12);
		UINT16 tempNum=0;
		UINT16 * temp = (UINT16 *) cpuserial;
		for (int i=0; i< 6; i++){
			tempNum=tempNum ^ temp[i]; //XOR 72-bit number to generate 16-bit hash
		}
#ifdef DEBUG_RF231
		hal_printf("RF231: Radio ID: %d\r\n", tempNum);
#endif
		SetAddress(tempNum);
		SetInitialized(TRUE);

		if(rx_msg_ptr != NULL)
		{
			private_free(rx_msg_ptr);
		}
		rx_msg_ptr = (Message_15_4_t *) private_malloc(sizeof(Message_15_4_t));
		if(rx_msg_ptr == NULL)
		{
			return DS_Fail;
		}

		if(tx_msg_ptr != NULL)
		{
			private_free(tx_msg_ptr);
		}
		tx_msg_ptr = (Message_15_4_t *) private_malloc(sizeof(Message_15_4_t));
		if(tx_msg_ptr == NULL)
		{
			return DS_Fail;
		}

		GLOBAL_LOCK(irq);
		//for(UINT8 i = 0; i < 30; i++)
			//data[i] = 0;
		if(TRUE != GpioPinInitialize())
		{
			return DS_Fail;
		}
		//configure_exti();

		if(TRUE != SpiInitialize())
		{
			ASSERT_RADIO(0);
		}

		// The radio initialization steps in the following lines are semantically copied from the corresponding tinyos implementation
		// Specified in the datasheet a sleep of 510us
		// The performance of this function is good but at times its found to generate different times. Its possible that there were other
		// events happening on the pin that was used to measure this or there is a possible bug !!!
		HAL_Time_Sleep_MicroSeconds(510);
		// Clear rstn pin
		RstnClear();
		// Clear the slptr pin
		SlptrClear();
		// Sleep for 6us
		HAL_Time_Sleep_MicroSeconds(6);

		RstnSet();

		// The RF230_TRX_CTRL_0 register controls the drive current of the digital output pads and the CLKM clock rate
		// Setting value to 0
		WriteRegister(RF230_TRX_CTRL_0, RF231_TRX_CTRL_0_VALUE);

		UINT32 reg = 0;
		reg = ReadRegister(RF230_TRX_CTRL_0);

		// The RF230_TRX_STATE register controls the state transition
		WriteRegister(RF230_TRX_STATE, RF230_TRX_OFF);

		// Nived.Sivadas - Hanging in the initialize function caused by the radio being in an unstable state
		// This fix will return false from initialize and enable the user of the function to exit gracefully
		// Fix for the hanging in the initialize function
		DID_STATE_CHANGE_ASSERT(RF230_TRX_OFF);

		HAL_Time_Sleep_MicroSeconds(510);

		if(this->GetRadioName() == RF231RADIOLR)
		{
			/* AnanthAtSamraksh - adding below line to fix receive on LR radio extension board - 2/11/2014 */
			// Enable external PA and control from RF231
			//rf231_enable_pa_rxtx();
			ENABLE_LRR(TRUE);
		}

		if(RF231_extended_mode){
		// Register controls the interrupts that are currently enabled
			WriteRegister(RF230_IRQ_MASK, RF230_IRQ_TRX_UR | RF230_IRQ_AMI | RF230_IRQ_TRX_END | RF230_IRQ_RX_START);
		}
		else{
			WriteRegister(RF230_IRQ_MASK, RF230_IRQ_TRX_UR | RF230_IRQ_TRX_END | RF230_IRQ_RX_START);
		}

		// The RF230_CCA_THRES sets the ed level for CCA, currently setting threshold to 0xc7
		WriteRegister(RF230_CCA_THRES, RF230_CCA_THRES_VALUE);

		// Controls output power and ramping of the transistor
		WriteRegister(RF230_PHY_TX_PWR, RF230_TX_AUTO_CRC_ON | (0 & RF230_TX_PWR_MASK));
		// Nived.Sivadas - turning off auto crc check
		//WriteRegister(RF230_PHY_TX_PWR, 0 | (0 & RF230_TX_PWR_MASK));
		tx_power = 0 & RF230_TX_PWR_MASK;
		channel = RF230_DEF_CHANNEL & RF230_CHANNEL_MASK;

		// Sets the channel number
		WriteRegister(RF230_PHY_CC_CCA, RF230_CCA_MODE_VALUE | channel);
		// Enable the gpio pin as the interrupt point
		if(this->GetRadioName() == RF231RADIO){
			CPU_GPIO_EnableInputPin(INTERRUPT_PIN, FALSE, Radio_Handler, GPIO_INT_EDGE_HIGH, RESISTOR_DISABLED);
			EXTI_ClearITPendingBit(EXTI_Line1);
		}
		else if(this->GetRadioName() == RF231RADIOLR){
			CPU_GPIO_EnableInputPin(INTERRUPT_PIN_LR, FALSE, Radio_Handler_LR, GPIO_INT_EDGE_HIGH, RESISTOR_DISABLED);
			EXTI_ClearITPendingBit(EXTI_Line1);
			//CPU_GPIO_EnableOutputPin(AMP_PIN_LR, FALSE);
			GPIO_ConfigurePin(GPIOB, GPIO_Pin_12, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
		}

		/***********Extended mode configuration***********/
		if(RF231_extended_mode){
			//Configure MAC short address, PAN-ID and IEEE address
			//Page 76-78 in RF231 datasheet
			//Page 54 - Configuration and address bits are to be set in TRX_OFF or PLL_ON state prior
			//			to switching to RX_AACK mode
			UINT16 addressHigh = GetAddress() >> 8;
			UINT16 addressLow = GetAddress() & 0xFF;
			WriteRegister(RF230_SHORT_ADDR_0, addressLow);
			WriteRegister(RF230_SHORT_ADDR_1, addressHigh);
			WriteRegister(RF230_PAN_ID_0, 0x55);
			WriteRegister(RF230_PAN_ID_1, 0x55);
			WriteRegister(RF230_IEEE_ADDR_0, 0x00);
			WriteRegister(RF230_IEEE_ADDR_1, 0x00);
			WriteRegister(RF230_IEEE_ADDR_2, 0x00);
			WriteRegister(RF230_IEEE_ADDR_3, 0x00);
			WriteRegister(RF230_IEEE_ADDR_4, 0x00);
			WriteRegister(RF230_IEEE_ADDR_5, 0x00);
			WriteRegister(RF230_IEEE_ADDR_6, 0x00);
			WriteRegister(RF230_IEEE_ADDR_7, 0x00);

			//RX_AACK configuration
			WriteRegister(RF230_TRX_CTRL_2, RF231_TRX_CTRL_2_VALUE);
			WriteRegister(RF230_XAH_CTRL_1, RF231_XAH_CTRL_1_VALUE);
			WriteRegister(RF230_XAH_CTRL_0, RF231_XAH_CTRL_0_VALUE);
			WriteRegister(RF230_CSMA_SEED_1, RF231_CSMA_SEED_1_VALUE);
			WriteRegister(RF230_CSMA_SEED_0, RF231_CSMA_SEED_0_VALUE);

			//Put the radio into extended mode (RX_AACK)
			/*WriteRegister(RF230_TRX_STATE, RF230_RX_AACK_ON);
			DID_STATE_CHANGE_ASSERT(RF230_RX_AACK_ON);*/

			//TX_ARET configuration
			WriteRegister(RF230_TRX_CTRL_1, RF231_TRX_CTRL_1_VALUE);
			WriteRegister(RF230_CSMA_BE, RF231_CSMA_BE_VALUE);
			/*WriteRegister(RF230_TRX_STATE, RF230_TX_ARET_ON);
			DID_STATE_CHANGE_ASSERT(RF230_TX_ARET_ON);*/
		}
		/*************************************************/

		SlptrSet();
		
		// TODO: with CSMA do we ever want to sleep?
		sleep_pending = TRUE;
		// set software state machine state to sleep
		state = STATE_SLEEP;
		//NATHAN_SET_DEBUG_GPIO(0);
#		ifdef DEBUG_RF231
		hal_printf("RF231: INIT. Default sleep\r\n");
#		endif
		//CPU_GPIO_SetPinState(   RF231_RADIO_STATEPIN2, FALSE );
		// Initialize default radio handlers

		// Added here until the state issues are resolved
		//TurnOn();

		cmd = CMD_NONE;

	}

	return DS_Success;
}


DeviceStatus RF231Radio::UnInitialize()
{
    DeviceStatus ret = DS_Success;

	interrupt_mode_check();

    if(IsInitialized())
    {
		if(rx_msg_ptr != NULL )
		{
			private_free(rx_msg_ptr);
			rx_msg_ptr = NULL;
		}

		if(tx_msg_ptr != NULL )
		{
			private_free(tx_msg_ptr);
			tx_msg_ptr = NULL;
		}
        RstnClear();
        SetInitialized(FALSE);
        /*ASSERT_RADIO((active_mac_index & 0xFF00) == 0);*/
        if(Radio<Message_15_4_t>::UnInitialize((UINT8)active_mac_index) != DS_Success) {
                ret = DS_Fail;
                SOFT_BREAKPOINT();
        }
        if(SpiUnInitialize() != TRUE) {
            ret = DS_Fail;
            SOFT_BREAKPOINT();
        }
        if(GpioPinUnInitialize() != TRUE) {
            ret = DS_Fail;
            SOFT_BREAKPOINT();
        }
        if(this->GetRadioName() == RF231RADIO){
            CPU_GPIO_DisablePin(INTERRUPT_PIN, RESISTOR_DISABLED,  GPIO_Mode_IN_FLOATING, GPIO_ALT_PRIMARY);
        }
        else if(this->GetRadioName() == RF231RADIOLR){
            CPU_GPIO_DisablePin(INTERRUPT_PIN_LR, RESISTOR_DISABLED, GPIO_Mode_IN_FLOATING, GPIO_ALT_PRIMARY);
            CPU_GPIO_DisablePin(AMP_PIN_LR, RESISTOR_DISABLED, GPIO_Mode_IN_FLOATING, GPIO_ALT_PRIMARY);
        }
    }
    return ret;
}


//template<class T>
void RF231Radio::WriteRegister(UINT8 reg, UINT8 value)
{
	GLOBAL_LOCK(irq);

	// FORCE_PLL_ON should never be used.
	ASSERT_RADIO( reg != RF230_TRX_STATE || (value&0x1F) != FORCE_PLL_ON);

	SelnClear();

	CPU_SPI_WriteByte(config, RF230_CMD_REGISTER_WRITE | reg);
	CPU_SPI_ReadWriteByte(config, value);
	CPU_SPI_ReadByte(config);

	SelnSet();

}


// Initializes the three pins, SELN, SLPTR and RSTN to the states specified
// Assumes that the these pins are not used by other modules. This should generally be handled by the gpio module
// Returns a void data type
//template<class T>
BOOL RF231Radio::GpioPinInitialize()
{

	if(CPU_GPIO_PinIsBusy(kseln))
		return FALSE;

	if(CPU_GPIO_PinIsBusy(kslpTr))
		return FALSE;

	if(CPU_GPIO_PinIsBusy(krstn))
		return FALSE;


	if(!CPU_GPIO_ReservePin(kseln, TRUE))
		return FALSE;

	if(!CPU_GPIO_ReservePin(kslpTr, TRUE))
		return FALSE;

	if(!CPU_GPIO_ReservePin(krstn, TRUE))
		return FALSE;


	CPU_GPIO_EnableOutputPin(kseln,TRUE);
	CPU_GPIO_EnableOutputPin(kslpTr,FALSE);
	CPU_GPIO_EnableOutputPin(krstn,TRUE);

	return TRUE;
}


//TODO: combine GpioPinUnInitialize and GpioPinInitialize
BOOL RF231Radio::GpioPinUnInitialize()
{

    if(!CPU_GPIO_ReservePin(kseln, FALSE))
        return FALSE;

    if(!CPU_GPIO_ReservePin(kslpTr, FALSE))
        return FALSE;

    if(!CPU_GPIO_ReservePin(krstn, FALSE))
        return FALSE;

    CPU_GPIO_DisablePin(kseln, RESISTOR_DISABLED, GPIO_Mode_IN_FLOATING, GPIO_ALT_PRIMARY);
    CPU_GPIO_DisablePin(kslpTr, RESISTOR_DISABLED, GPIO_Mode_IN_FLOATING, GPIO_ALT_PRIMARY);
    CPU_GPIO_DisablePin(krstn, RESISTOR_DISABLED, GPIO_Mode_IN_FLOATING, GPIO_ALT_PRIMARY);

    return TRUE;
}


// Calls the mf spi initialize function and returns true if the intialization was successful
//template<class T>
BOOL RF231Radio::SpiInitialize()
{
	// Calling mf spi initialize function
	//if(TRUE != CPU_SPI_Initialize())
	//	return FALSE;

	config.DeviceCS               = 10; //TODO - fix me..
	config.CS_Active              = false;
	config.CS_Setup_uSecs         = 0;
	config.CS_Hold_uSecs          = 0;
	config.MSK_IDLE               = false;
	config.MSK_SampleEdge         = false;
	config.Clock_RateKHz          = 16; // THIS IS IGNORED.
	if(this->GetRadioName() == RF231RADIO)
	{
		config.SPI_mod                = RF231_SPI_BUS;
	}
	else if(this->GetRadioName() == RF231RADIOLR)
	{
		config.SPI_mod 				  = RF231_LR_SPI_BUS;
	}

	config.MD_16bits = FALSE;


	// Enable the SPI depending on the radio who is the user
	CPU_SPI_Enable(config);

	return TRUE;
}


BOOL RF231Radio::SpiUnInitialize()
{
    CPU_SPI_Uninitialize(config);
    return TRUE;
}


// This function moves the radio from sleep to RX_ON
//template<class T>
DeviceStatus RF231Radio::TurnOnRx()
{
	INIT_STATE_CHECK();
	interrupt_mode_check();
	GLOBAL_LOCK(irq);

	if (!IsInitialized()) {
		ASSERT_RADIO(0);
		return DS_Fail;
	}

	sleep_pending = FALSE;

	add_rx_start_time();

	//For some reason, radio loses state between init and setting radio state (TxPower for instance).
	//This is a hack until the reason is found out.
	radio_hal_trx_status_t trx_status_tmp = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	if(trx_status_tmp == P_ON){
		if(TRUE != SpiInitialize())
		{
			ASSERT_RADIO(0);
		}
	}

	if(RF231_extended_mode){
		if ( !Careful_State_Change_Extended(RX_AACK_ON) ) {
#ifdef DEBUG_RF231
			radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			hal_printf("RF231Radio::TurnOnRx - returning failure; status is %d\n", trx_status);
#endif
			//ASSERT_RADIO(0);
			return DS_Fail;
		}
	}
	else{
		if ( !Careful_State_Change(RX_ON) ) {
			//ASSERT_RADIO(0);
			return DS_Fail;
		}
	}

	if(RF231_extended_mode){
		state = STATE_RX_AACK_ON;
	}
	else{
		state = STATE_RX_ON;
	}
	return DS_Success;
}	//RF231Radio::TurnOnRx()


DeviceStatus RF231Radio::TurnOffRx()
{
	INIT_STATE_CHECK();
	interrupt_mode_check();
	GLOBAL_LOCK(irq);

	if (!IsInitialized()) {
		//ASSERT_SP(0);
		return DS_Success;
	}

	sleep_pending = FALSE;

	add_rx_start_time();

	//For some reason, radio loses state between init and setting radio state (TxPower for instance).
	//This is a hack until the reason is found out.
	radio_hal_trx_status_t trx_status_tmp = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	if(trx_status_tmp == P_ON){
		if(TRUE != SpiInitialize())
		{
			ASSERT_RADIO(0);
		}
	}

	if(RF231_extended_mode){
		if ( !Careful_State_Change_Extended(TRX_OFF) ) {
#ifdef DEBUG_RF231
			radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			hal_printf("RF231Radio::TurnOffRx - returning failure; status is %d\n", trx_status);
#endif
			ASSERT_SP(0);
			return DS_Fail;
		}
	}
	else{
		if ( !Careful_State_Change(TRX_OFF) ) {
			ASSERT_SP(0);
			return DS_Fail;
		}
	}

	//TODO:? handle external interrupt firing to indicate RF231 IRQ_4 AWAKE_END?

	state = STATE_TRX_OFF;

	return DS_Success;
}	//RF231Radio::TurnOffRx()


//template<class T>
__IO UINT8 RF231Radio::ReadRegister(UINT8 reg)
{
	GLOBAL_LOCK(irq);
	UINT8 read_reg;

	SelnClear();

	CPU_SPI_WriteByte(config, RF230_CMD_REGISTER_READ | reg);
	CPU_SPI_ReadWriteByte(config, 0);
	read_reg = CPU_SPI_ReadByte(config);


	SelnSet();

	return read_reg;

}


//	Responsible for clear channel assessment
//  Takes numberMicroSecond as parameter allowing the user to specify the time to watch the channels
//	Returns DS_Success if the channel is free, DS_Busy is not and DS_Fail is the assessment failed
//  While this function takes an argument for time, it should be noted that the rf231 radio does not
//  have the ability to increase its measurement time to more than 8 symbols in the RX_ON state. Hence in the
//  RX_ON state the result is available at the end of 140 us. However, it may happen that the radio is in BUSY_RX state
//  in which case we have to wait for a while before the cca can be done, this function can be used in those cases.

//template<class T>
DeviceStatus RF231Radio::ClearChannelAssesment(UINT32 numberMicroSecond)
{
	UINT8 trx_status;

	//return DS_Fail; // Not yet supported --NPS

	GLOBAL_LOCK(irq);

	// The radio takes a minimum of 140 us to do cca, any numbers less than this are not acceptable
	if(numberMicroSecond < 140)
		return DS_Fail;

	// If cca is initiated during sleep, come out of sleep do cca and go back to sleep
	if(state == STATE_SLEEP)
	{
		if(TurnOnRx() != DS_Success)
			return DS_Fail;

		sleep_pending = TRUE;
		if(RF231_extended_mode){
			state = STATE_RX_AACK_ON;
		}
		else{
			state = STATE_RX_ON;
		}
	}

	// Must be in RX mode to do measurment.
	radio_hal_trx_status_t trx_status_tmp = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	if(RF231_extended_mode){
		if (trx_status_tmp != RX_AACK_ON && trx_status_tmp != BUSY_RX_AACK){
			return DS_Fail;
		}
	}
	else{
		if (trx_status_tmp != RX_ON && trx_status_tmp != BUSY_RX){
			return DS_Fail;
		}
	}

	if(RF231_extended_mode){
		if(state != STATE_RX_AACK_ON)
		{
			UINT8 reg = VERIFY_STATE_CHANGE;
			return DS_Fail;
		}
	}
	else{
		if(state != STATE_RX_ON)
		{
			UINT8 reg = VERIFY_STATE_CHANGE;
			return DS_Fail;
		}
	}

	//UINT8 reg = VERIFY_STATE_CHANGE;

	// Make a cca request
	WriteRegister(RF230_PHY_CC_CCA, RF230_CCA_REQUEST | RF230_CCA_MODE_VALUE | channel);

	// Busy wait for the duration of numberMicrosecond
	HAL_Time_Sleep_MicroSeconds(numberMicroSecond);

	// Read the register to check the result of the assessment
	trx_status = ReadRegister(RF230_TRX_STATUS);


	// If the CCA was initiated during sleep, go back to sleep once the work is done
	if(sleep_pending)
	{
		// If sleep is success then go back to sleep and turn sleep_pending to FALSE
		if(Sleep(0) == DS_Success)
		{
			state = STATE_SLEEP;
			sleep_pending = FALSE;
		}
		else
		{
			return DS_Fail;
		}
	}

	// return the result of the assessment
	return ((trx_status & RF230_CCA_DONE) ? ((trx_status & RF230_CCA_STATUS) ? DS_Success : DS_Busy) : DS_Fail );
}	//RF231Radio::ClearChannelAssesment

// See RF231 datasheet section 8.3.4
// Return 0 to 28, 3dB steps, from -91 dBm to 10 dBm
INT32 RF231Radio::GetRSSI() {
	if(RF231_extended_mode){
		if (state != STATE_RX_AACK_ON && state != STATE_BUSY_RX_AACK) {
			return 0x7FFFFFFF;
		}
	}
	else{
		if (state != STATE_RX_ON && state != STATE_BUSY_RX) {
			return 0x7FFFFFFF;
		}
	}

	return ReadRegister(RF230_PHY_RSSI) & 0x1F;
}

// See RF231 datasheet section 8.3.4
// Return 0 to 28, 3dB steps, from -91 dBm to 10 dBm
UINT16 RF231Radio::GetAverageOrMaxRSSI_countN(const UINT8 rssiCount){
#ifdef RSSI_CHECK_ALL_VALUES
	int measureAgainCounter = 0;
measureAgain:
	static int startIndexForGoodRssi = 0;
#endif
	if(RF231_extended_mode){
		if (state != STATE_RX_AACK_ON && state != STATE_BUSY_RX_AACK) {
			hal_printf("GetAverageOrMaxRSSI_countN; state is: %d\n", state);
			return 0xFFFF;
		}
	}
	else{
		if (state != STATE_RX_ON && state != STATE_BUSY_RX) {
			hal_printf("GetAverageOrMaxRSSI_countN; state is: %d\n", state);
			return 0xFFFF;
		}
	}
	UINT16 rssi = 0;
	UINT8 rssiBuffer[rssiCount] = {0};
	//Takes 109 usec to read 8 RSSI values
	for(int i = 0; i < rssiCount; i++){
		rssiBuffer[i] = ReadRegister(RF230_PHY_RSSI) & RF230_RSSI_MASK;
	}
#ifndef RSSI_CHECK_ALL_VALUES
	//Find average RSSI
	/*for(int i = 0; i < rssiCount; i++){
		averageRssi += rssiBuffer[i];
	}
	averageRssi = averageRssi/rssiCount;*/

	//Find max RSSI
	for(int i = 0; i < rssiCount; i++){
		if(rssiBuffer[i] > rssi){
			rssi = rssiBuffer[i];
		}
	}
#endif

#ifdef RSSI_CHECK_ALL_VALUES
	//Below method is aggressive in terms of finding if there is some transmission activity going on
	for(int i = 0; i < rssiCount; i++){
		if(rssiBuffer[i] >= rssiThresholdForTransmission){
			//If index is g.t 3 and either current value is g.t prev value or current is g.t threshold, result is true
			if( i >= 3){
				result = true;
				startIndexForGoodRssi = i;
			}
		}
		else{
			result = false;
		}
	}

	//If high rssi values are towards the end, then measure again.
	//Maybe there is a transmission that just started
	if(result && startIndexForGoodRssi >= 7){
		measureAgainCounter++;
		startIndexForGoodRssi = 0;
		if(measureAgainCounter >= 2){
			return false;
		}
		goto measureAgain;
	}
#endif

	return rssi;
}

//From the RF231 datasheet (pg 89, 90)
//Min RSSI (absolute value) is 0 and max is 28 in steps of 3dB.
//P_RF = RSSI_BASE_VAL + 3*(RSSI -1) [dBm] (RSSI_BASE_VAL is -91 dbm)
//0 db is -91 dbm and 28 is -10 dbm
//Choosing a value of 4 db as the threshold for detection of transmission, which
//	corresponds to -82 dbm.
//RF230_CCA_THRES_VALUE is set to the default value of C7 which corresponds to -77dbm (-91 + 2*7).
//	7 is the lower 4 bits of the register
//NOTE: this is in the basic operating mode of the radio
//Check RSSI over a period of 16 usec (2 usec per measurement) and if RSSI value of 10 and above
//	is more than 4 towards the end, then return success, else fail
//#define RSSI_CHECK_ALL_VALUES
BOOL RF231Radio::CheckForRSSI()
{
	const UINT8 rssiThresholdForTransmission = 4;
	bool result = false;
	UINT16 rssi = GetAverageOrMaxRSSI_countN(8);

	if(rssi >= rssiThresholdForTransmission){
		result = true;
	}
	else{
		result = false;
	}

	return result;
}

//	Responsible for clear channel assessment
//  Default version waits for 140 us
//	Returns DS_Success if the channel is free, DS_Busy is not and DS_Fail is the assessment failed
//

//template<class T>
DeviceStatus RF231Radio::ClearChannelAssesment()
{
	//return DS_Fail;  // Not yet supported --NPS

	GLOBAL_LOCK(irq);

	// Must be in RX mode to do measurment.
	radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
	if(RF231_extended_mode){
		if (trx_status != RX_AACK_ON && trx_status != BUSY_RX_AACK){
			//hal_printf("CCA failed; state is %d\n", state);
			return DS_Fail;
		}
	}
	else{
		if (trx_status != RX_ON && trx_status != BUSY_RX){
			//hal_printf("CCA failed; state is %d\n", state);
			return DS_Fail;
		}
	}

	WriteRegister(RF230_PHY_CC_CCA, RF230_CCA_REQUEST | RF230_CCA_MODE_VALUE | channel);

	//CheckForRSSI takes 116 usec (109 usec + 7 usec for processing)
	BOOL rssiStatus = CheckForRSSI();

	//If there is energy in the channel, return failure
	if(rssiStatus){
		return DS_Fail;
	}
	else{
		return DS_Success;
	}

	//irq.Release();

	// Busy wait for the minimum duration of 140 us
	//HAL_Time_Sleep_MicroSeconds(150);

	//UINT8 stat;
	//stat = ReadRegister(RF230_TRX_STATE);
	//stat = VERIFY_STATE_CHANGE;

	//return ((stat & RF230_CCA_DONE) ? ((stat & RF230_CCA_STATUS) ? DS_Success : DS_Busy) : DS_Fail );
}


//template<class T>
void RF231Radio::HandleInterrupt()
{
	static volatile UINT32 irq_cause = 0;
	INT16 temp = 0;
	const UINT8 UNSUPPORTED_INTERRUPTS = TRX_IRQ_BAT_LOW | TRX_IRQ_TRX_UR;
	INIT_STATE_CHECK();

	// I don't want to do a big lock here but the rest of the driver is so ugly... --NPS
	GLOBAL_LOCK(irq);

	irq_cause = ReadRegister(RF230_IRQ_STATUS); // This clears the IRQ as well
	//hal_printf("irq_cause is: %u\n", irq_cause);
#ifdef DEBUG_RF231
	hal_printf("irq_cause is: %u\n", irq_cause);
#endif

	// DEBUG NATHAN
	add_irq_time(irq_cause);
	// DEBUG NATHAN

	// Only consider CMD_TRANSMIT.
	// If this came in on CMD_RECEIVE, we had overlapping RX events
	if( irq_cause & UNSUPPORTED_INTERRUPTS ) {
		add_trx_ur();
		if (cmd == CMD_TRANSMIT) { ASSERT_RADIO(0); }
		//cmd = CMD_NONE; // Thinking about this... --NPS
		// else it was an RX overrun and we live with it.
	}

	if( (irq_cause & 0xFF) == (TRX_NO_IRQ & 0xFF) ) {
#ifdef DEBUG_RF231
		ASSERT_SP(0);
#endif
		return;
	}

	// See datasheet section 9.7.5. We handle both of these manually.
	if(irq_cause & TRX_IRQ_PLL_LOCK) {
		//(Radio<Message_15_4_t>::GetMacHandler(active_mac_index)->GetRadioInterruptHandler())(PreambleDetect);
	}
	if(irq_cause & TRX_IRQ_PLL_UNLOCK) {  }

	//if( (irq_cause & TRX_IRQ_RX_START) && (irq_cause & TRX_IRQ_AMI) ){
	/*if( irq_cause == (TRX_IRQ_RX_START | TRX_IRQ_AMI) ){
		CPU_GPIO_SetPinState( RF231_AMI, TRUE );
		CPU_GPIO_SetPinState( RF231_AMI, FALSE );
		CPU_GPIO_SetPinState( RF231_AMI, TRUE );
		CPU_GPIO_SetPinState( RF231_AMI, FALSE );
		//This is an invalid case
		//irq_cause = TRX_NO_IRQ;
		//return;
	}*/

	if(irq_cause & TRX_IRQ_RX_START)
	{
		//CPU_GPIO_SetPinState( RF231_RX_START, TRUE );
		add_rx_start_time();
		if(RF231_extended_mode){
			state = STATE_BUSY_RX_AACK; // Seems like we should change state, so I made one up...
		}
		else{
			state = STATE_BUSY_RX; // Seems like we should change state, so I made one up...
		}
		//NATHAN_SET_DEBUG_GPIO(0);
#		ifdef DEBUG_RF231
		hal_printf("RF231: TRX_IRQ_RX_START\r\n");
#		endif
		// We got an Recieve frame start when the driver is not doing any thing particular
		// let us process this interrupt then

		temp = ReadRegister(RF230_PHY_RSSI) & RF230_RSSI_MASK;

		// Keeps track of average rssi, why ?? may be useful someday :)
		// How is this an average??? --NPS
		rssi_busy += temp - (rssi_busy >> 2);

		// Add the rssi to the message
		IEEE802_15_4_Metadata_t* metadata = rx_msg_ptr->GetMetaData();
		metadata->SetRssi(temp);

		// Do the time stamp here instead of after done, I think.
		// Note there is potential to use a capture time here, for better accuracy.
		// Currently, this will depend on interrupt latency.

		receive_timestamp = HAL_Time_CurrentTicks();

		// We have a 64 bit local clock, do we need 64 bit timestamping, perhaps not
		// Lets stick to 32, the iris implementation uses the timer to measure when the input was
		// captured giving more accurate timestamping, we are going to rely on this less reliable technique
		//AnanthAtSamraksh: defaulting to the AdvancedTimer

		// This is not being used anywhere right now, so commenting out.
		//time = (HAL_Time_CurrentTicks() >> 32); // Throwing away the bottom 32-bits? Doesn't that really hurt timing??? --NPS

		// Initiate cmd receive // This is tied to
//		if(RF231_extended_mode){
//			cmd = CMD_RX_AACK;
//		}
//		else{
//			cmd = CMD_RECEIVE;
//		}

		//HAL_Time_Sleep_MicroSeconds(64); // wait 64us to prevent spurious TRX_UR interrupts. // TODO... HELP --NPS




		 ////(Radio<Message_15_4_t>::GetMacHandler(active_mac_index)->GetRadioInterruptHandler())(StartOfReception,(void*)rx_msg_ptr);
		//(Radio_event_handler.GetRadioInterruptHandler())(StartOfReception,(void*)rx_msg_ptr);
		//CPU_GPIO_SetPinState( RF231_RX_START, FALSE );
	}


	if(irq_cause & TRX_IRQ_AMI)
	{
		//CPU_GPIO_SetPinState( RF231_AMI, TRUE );
		//hal_printf("Inside TRX_IRQ_AMI\n");
#ifdef DEBUG_RF231
		hal_printf("Inside TRX_IRQ_AMI\n");
#endif
		//After an address match, next step is FCS which generates TRX_END interrupt
		////cmd = CMD_RX_AACK;

		add_rx_start_time();
		if(RF231_extended_mode){
			state = STATE_BUSY_RX_AACK; // Seems like we should change state, so I made one up...
		}
		else{
			state = STATE_BUSY_RX; // Seems like we should change state, so I made one up...
		}
		//NATHAN_SET_DEBUG_GPIO(0);
#		ifdef DEBUG_RF231
		hal_printf("RF231: TRX_IRQ_RX_START\r\n");
#		endif
		// We got an Recieve frame start when the driver is not doing any thing particular
		// let us process this interrupt then

		temp = ReadRegister(RF230_PHY_RSSI) & RF230_RSSI_MASK;

		// Keeps track of average rssi, why ?? may be useful someday :)
		// How is this an average??? --NPS
		rssi_busy += temp - (rssi_busy >> 2);

		// Add the rssi to the message
		IEEE802_15_4_Metadata_t* metadata = rx_msg_ptr->GetMetaData();
		metadata->SetRssi(temp);

		// Do the time stamp here instead of after done, I think.
		// Note there is potential to use a capture time here, for better accuracy.
		// Currently, this will depend on interrupt latency.
		//receive_timestamp = HAL_Time_CurrentTicks();

		// We have a 64 bit local clock, do we need 64 bit timestamping, perhaps not
		// Lets stick to 32, the iris implementation uses the timer to measure when the input was
		// captured giving more accurate timestamping, we are going to rely on this less reliable technique
		//AnanthAtSamraksh: defaulting to the AdvancedTimer

		// This is not being used anywhere right now, so commenting out.
		//time = (HAL_Time_CurrentTicks() >> 32); // Throwing away the bottom 32-bits? Doesn't that really hurt timing??? --NPS

		// Initiate cmd receive
		if(RF231_extended_mode){
			cmd = CMD_RX_AACK;
		}
		else{
			cmd = CMD_RECEIVE;
		}





		/*//Enable ACKs
		//1100 0010
		WriteRegister(RF230_CSMA_SEED_1, RF231_CSMA_SEED_1_VALUE);
		if(DS_Success == DownloadMessage(sizeof(IEEE802_15_4_Header_t))){
			if(rx_length > IEEE802_15_4_FRAME_LENGTH){
#ifdef DEBUG_RF231
			hal_printf("Radio Receive Error: Packet too big: %d\r\n",rx_length);
#endif
				return;
			}
			IEEE802_15_4_Header_t* header = (IEEE802_15_4_Header_t*)rx_msg_ptr->GetHeader();
			//UINT8* payloadMSG = rx_msg_ptr->GetPayload();
			if(header->dest != GetAddress() && header->dest != 65535){
			//if(header->dest == GetAddress()){
				//Disable ACKs
				//Bit 4 	- AACK_DIS_ACK 		- If this bit is set no ack frames are transmitted (0)
				//1101 0010
				WriteRegister(RF230_CSMA_SEED_1, 0xD2);
				hal_printf("Incorrect dest address: %d\n", header->dest);
				hal_printf("src address: %d\n", header->src);
				//hal_printf("payload[8]: %d\n", payloadMSG[8]);
				//ASSERT_RADIO(0);
				//return;
			}
		}
		*/

		//HAL_Time_Sleep_MicroSeconds(64); // wait 64us to prevent spurious TRX_UR interrupts. // TODO... HELP --NPS

		//CPU_GPIO_SetPinState( RF231_AMI, FALSE );
		(Radio_event_handler.GetRadioInterruptHandler())(StartOfReception,(void*)rx_msg_ptr);
	}

	// The contents of the frame buffer went out (OR we finished a RX --NPS)
	if(irq_cause & TRX_IRQ_TRX_END)
	{
		if(cmd == CMD_TRANSMIT)
		{
#			ifdef DEBUG_RF231
			hal_printf("RF231: TRX_IRQ_TRX_END : Transmit Done\r\n");
#			endif

			add_send_done();

			//CPU_GPIO_SetPinState( RF231_TRX_TX_END, TRUE );
			//CPU_GPIO_SetPinState( RF231_TRX_TX_END, FALSE );

			state = STATE_PLL_ON;

			UINT8 trx_state = ReadRegister(RF230_TRX_STATE) & TRAC_STATUS_MASK;
			// Call radio send done event handler when the send is complete
			//SendAckFuncPtrType AckHandler = Radio<Message_15_4_t>::GetMacHandler(active_mac_index)->GetSendAckHandler();
			//(*AckHandler)(tx_msg_ptr, tx_length,NetworkOperations_Success);
			(Radio_event_handler.GetSendAckHandler())(tx_msg_ptr, tx_length, NetworkOperations_Success, trx_state);

			cmd = CMD_NONE;

			if(sleep_pending)
			{
				Sleep(0);
				return;
			}
			else //
			{
				Careful_State_Change(RX_ON);
				state = STATE_RX_ON;
			}
		}
		else if(cmd == CMD_RECEIVE)
		{
			//CPU_GPIO_SetPinState( RF231_RX, TRUE );
#			ifdef DEBUG_RF231
			hal_printf("RF231: TRX_IRQ_TRX_END : Receive Done\n");
#			endif

			//CPU_GPIO_SetPinState( RF231_TRX_RX_END, TRUE );
			//CPU_GPIO_SetPinState( RF231_TRX_RX_END, FALSE );

			// Go to PLL_ON at least until the frame buffer is empty

			if(DS_Success==DownloadMessage()){
				//rx_msg_ptr->SetActiveMessageSize(rx_length);
				if(rx_length>  IEEE802_15_4_FRAME_LENGTH){
#					ifdef DEBUG_RF231
					hal_printf("Radio Receive Error: Packet too big: %d\r\n",rx_length);
#					endif
					return;
				}

				// Please note this is kind of a hack.
				// Manually check our interrupts here (since we are locked).
				// If we see a new interrupt, just assume it means we had an RX overrun.
				// In which case we just drop the packet (or packets?) --NPS

				// Un-sure if this is how to drop a packet. --NPS
				if ( !Interrupt_Pending() ) {
					//int type = rx_msg_ptr->GetHeader()->type;
					//(rx_msg_ptr->GetHeader())->SetLength(rx_length);
					//rx_msg_ptr = (Message_15_4_t *) (Radio<Message_15_4_t>::GetMacHandler(active_mac_index)->GetReceiveHandler())(rx_msg_ptr, rx_length);
					(Radio_event_handler.GetReceiveHandler())(rx_msg_ptr, rx_length);
				}
			}
			else {
				// download error
				add_download_error();
			}

			cmd = CMD_NONE;

			// Check if mac issued a sleep while i was receiving something
			if(sleep_pending)
			{
				Sleep(0);
				return;
			}
			else { // Now safe to go back to RX_ON
				Careful_State_Change(RX_ON);
				state = STATE_RX_ON;
			}
			//CPU_GPIO_SetPinState( RF231_RX, FALSE );
		}
		else if(cmd == CMD_TX_ARET)
		{
			/*radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			if(trx_status == RX_AACK_ON){
				CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, TRUE);
				CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, FALSE);
				CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, TRUE);
				CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, FALSE);
			}*/
			add_send_done();

			state = STATE_PLL_ON;
#ifdef DEBUG_RF231
			hal_printf("Inside TRX_IRQ_TRX_END(CMD_TX_ARET)\n");
			hal_printf("HandleInterrupt::TRX_IRQ_TRX_END(CMD_TX_ARET) sequenceNumberSender: %d\n", sequenceNumberSender);
#endif
			// Call radio send done event handler when the send is complete
			//SendAckFuncPtrType AckHandler = Radio<Message_15_4_t>::GetMacHandler(active_mac_index)->GetSendAckHandler();
			//(*AckHandler)(tx_msg_ptr, tx_length,NetworkOperations_Success);
#ifdef DEBUG_RF231
			/*IEEE802_15_4_Header_t *header = (IEEE802_15_4_Header_t*)tx_msg_ptr->GetHeader();
			UINT8* payloadMSG = tx_msg_ptr->GetPayload();
			if(header->dsn == 97){
				hal_printf("(CMD_TX_ARET)Sending DATA; payload[0]: %d; payload[1]: %d; payload[2]: %d; payload[3]: %d; payload[8]: %d\n\n", payloadMSG[0], payloadMSG[1], payloadMSG[2], payloadMSG[3], payloadMSG[8]);
				hal_printf("(CMD_TX_ARET)header->fcf: %d;header->dsn: %d;header->dest: %d;header->destpan: %d;header->src: %d;header->srcpan: %d;header->length: %d;header->mac_id: %d;header->type: %d;header->flags: %d\n\n", header->fcf,header->dsn,header->dest,header->destpan,header->src,header->srcpan,header->length,header->mac_id,header->type,header->flags);
			}*/
#endif
			//CPU_GPIO_SetPinState( RF231_TRX_TX_END, TRUE );
			//CPU_GPIO_SetPinState( RF231_TRX_TX_END, FALSE );
			UINT8 trx_state = ReadRegister(RF230_TRX_STATE) & TRAC_STATUS_MASK;
			//if(trx_state == 0x00){	//Success
				//hal_printf("(CMD_TX_ARET)trx_state: %d\n", trx_state);
				//CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, TRUE);
				//CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, FALSE);
				//CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, TRUE);
				//CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, FALSE);
				/*if(DS_Success == DownloadMessage()){
					(rx_msg_ptr->GetHeader())->length = rx_length;
					IEEE802_15_4_Header_t* header = (IEEE802_15_4_Header_t*)rx_msg_ptr->GetHeader();
					UINT8* payloadMSG = rx_msg_ptr->GetPayload();
					if(header->src == 0 && header->dest == 0){
						hal_printf("ACK dest address: %d\n", header->dest);
						hal_printf("ACK src address: %d\n", header->src);
						hal_printf("ACK seq number: %d\n", header->dsn);
					}
				}*/
				(Radio_event_handler.GetSendAckHandler())(tx_msg_ptr, tx_length, NetworkOperations_Success, trx_state);
			//}
			//else if(trx_state == 0x00){	//Success)

			//}

			cmd = CMD_NONE;

			if(sleep_pending)
			{
				Sleep(0);
				/*CPU_GPIO_SetPinState(RF231_HW_ACK_RESP_TIME, TRUE);
				CPU_GPIO_SetPinState(RF231_HW_ACK_RESP_TIME, FALSE);
				CPU_GPIO_SetPinState(RF231_HW_ACK_RESP_TIME, TRUE);
				CPU_GPIO_SetPinState(RF231_HW_ACK_RESP_TIME, FALSE);*/
				return;
			}
			else //
			{
				Careful_State_Change_Extended(RX_AACK_ON);
				state = STATE_RX_AACK_ON;
			}
			//CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, TRUE);
			//CPU_GPIO_SetPinState(RF231_START_OF_RX_MODE, FALSE);
#ifdef DEBUG_RF231
			radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			hal_printf("RF231Radio::HandleInterrupt(TX_ARET)-status is %d, state is %d\n", trx_status, state);
#endif
		}
		else if(cmd == CMD_RX_AACK)
		{
			//CPU_GPIO_SetPinState( RF231_TRX_RX_END, TRUE );
			//CPU_GPIO_SetPinState( RF231_TRX_RX_END, FALSE );
#ifdef DEBUG_RF231
			hal_printf("Inside TRX_IRQ_TRX_END(CMD_RX_AACK)\n");
#endif
			//state = STATE_RX_AACK_ON; // Right out of BUSY_RX

			//(Radio_event_handler.GetReceiveHandler())(rx_msg_ptr, rx_length);

			//Refer page 62 and 69, 70
			//Getting bits 5,6,7 of TRX_STATE to check if status is SUCCESS_WAIT_FOR_ACK
			//	Indicates that an ACK frame is about to be sent
			/*UINT8 trx_state = ReadRegister(RF230_TRX_STATE) & TRAC_STATUS_MASK;
			hal_printf("(CMD_RX_AACK)trx_state: %d\n", trx_state);*/

			//if(trx_state == 0x40 && state == STATE_BUSY_RX_AACK)
			//{
				if(DS_Success == DownloadMessage()){
					//CPU_GPIO_SetPinState( (GPIO_PIN)CCA_PIN, TRUE );
					//rx_msg_ptr->SetActiveMessageSize(rx_length);
					if(rx_length > IEEE802_15_4_FRAME_LENGTH){
#ifdef DEBUG_RF231
						hal_printf("Radio Receive Error: Packet too big: %d\r\n",rx_length);
#endif
						return;
					}

					// Please note this is kind of a hack.
					// Manually check our interrupts here (since we are locked).
					// If we see a new interrupt, just assume it means we had an RX overrun.
					// In which case we just drop the packet (or packets?) --NPS

					// Un-sure if this is how to drop a packet. --NPS
					if ( !Interrupt_Pending() ) {
						// Initiate frame transmission by asserting SLP_TR pin
						//Send ACK only if msg has been successfully downloaded and receiveHandler will be called.

						/*SlptrSet();
						SlptrClear();
						CPU_GPIO_SetPinState( (GPIO_PIN)CCA_PIN, TRUE );
						CPU_GPIO_SetPinState( (GPIO_PIN)CCA_PIN, FALSE );*/

#ifdef DEBUG_RF231
						//CPU_GPIO_SetPinState(RF231_HW_ACK_RESP_TIME, TRUE);
						//CPU_GPIO_SetPinState(RF231_HW_ACK_RESP_TIME, FALSE);
						IEEE802_15_4_Header_t* header = (IEEE802_15_4_Header_t*)rx_msg_ptr->GetHeader();
						sequenceNumberReceiver = header->dsn;
						hal_printf("HandleInterrupt::TRX_IRQ_TRX_END(CMD_RX_AACK) header->dsn: %d; sequenceNumberReceiver: %d\n", header->dsn, sequenceNumberReceiver);

						UINT8* payloadMSG = rx_msg_ptr->GetPayload();
						/*if(header->dsn == 27){
							hal_printf("(CMD_RX_AACK)Received DISCO; payload[1]: %d; payload[2]: %d; payload[3]: %d; payload[4]: %d; payload[8]: %d\n\n", payloadMSG[1], payloadMSG[2], payloadMSG[3], payloadMSG[4], payloadMSG[8]);
							hal_printf("(CMD_RX_AACK)header->fcf: %d;header->dsn: %d;header->dest: %d;header->destpan: %d;header->src: %d;header->srcpan: %d;header->length: %d;header->mac_id: %d;header->type: %d;header->flags: %d\n\n", header->fcf,header->dsn,header->dest,header->destpan,header->src,header->srcpan,header->length,header->mac_id,header->type,header->flags);
						}*/
						if(header->dsn == 97){
							//hal_printf("(CMD_RX_AACK)Received DATA; payload[1]: %d; payload[2]: %d; payload[3]: %d; payload[4]: %d; payload[8]: %d\n\n", payloadMSG[1], payloadMSG[2], payloadMSG[3], payloadMSG[4], payloadMSG[8]);
							hal_printf("(CMD_RX_AACK)Received DATA; payload[0]: %d; payload[1]: %d; payload[2]: %d; payload[3]: %d; payload[8]: %d\n\n", payloadMSG[0], payloadMSG[1], payloadMSG[2], payloadMSG[3], payloadMSG[8]);
							//hal_printf("(CMD_RX_AACK)header->fcf: %d;header->dsn: %d;header->dest: %d;header->destpan: %d;header->src: %d;header->srcpan: %d;header->length: %d;header->mac_id: %d;header->type: %d;header->flags: %d\n\n", header->fcf,header->dsn,header->dest,header->destpan,header->src,header->srcpan,header->length,header->mac_id,header->type,header->flags);
							//hal_printf("(CMD_RX_AACK)header->fcf: %d;header->dsn: %d;header->dest: %d;header->destpan: %d;header->src: %d;header->srcpan: %d\n\n", header->fcf,header->dsn,header->dest,header->destpan,header->src,header->srcpan);
							hal_printf("(CMD_RX_AACK)header->dsn: %d;header->dest: %d;header->destpan: %d;header->src: %d;header->srcpan: %d\n\n", header->dsn,header->dest,header->destpan,header->src,header->srcpan);

						}
#endif

						/*
						IEEE802_15_4_Header_t* header = (IEEE802_15_4_Header_t*)rx_msg_ptr->GetHeader();
						//UINT8* payloadMSG = rx_msg_ptr->GetPayload();
						if(header->dest != GetAddress() && header->dest != 65535){
							UINT16 myAddr = GetAddress();
							hal_printf("Incorrect dest address: %d\n", header->dest);
							hal_printf("src address: %d\n", header->src);
							//ASSERT_RADIO(0);
							//hal_printf("payload[8]: %d\n", payloadMSG[8]);
							//return;
						}
						*/

						(rx_msg_ptr->GetHeader())->length = rx_length;
						//rx_msg_ptr = (Message_15_4_t *) (Radio<Message_15_4_t>::GetMacHandler(active_mac_index)->GetReceiveHandler())(rx_msg_ptr, rx_length);
						(Radio_event_handler.GetReceiveHandler())(rx_msg_ptr, rx_length);

						//CPU_GPIO_SetPinState( (GPIO_PIN)CCA_PIN, FALSE );

						//if(rx_msg_ptr->GetHeader()->dsn != OMAC_DISCO_SEQ_NUMBER){
							//As per page 62, transmission is signaled using SLP_TR after 6 symbols (96 usec).
							//But that translates to 130 usec for eMote debug version.
							//This should be changed for release version of eMote.
							//TODO:Modify for release
							/*HAL_Time_Sleep_MicroSeconds(OMAC_HW_ACK_DELAY_MICRO);
							SlptrSet();
							SlptrClear();*/
							//CPU_GPIO_SetPinState( (GPIO_PIN)RF231_GENERATE_HW_ACK, TRUE );
							//CPU_GPIO_SetPinState( (GPIO_PIN)RF231_GENERATE_HW_ACK, FALSE );
						/*}
						else{
							HAL_Time_Sleep_MicroSeconds(125);
							SlptrSet();
							SlptrClear();
						}*/
					}
				}
				else {
					// download error
					add_download_error();
				}
			/*}
			else{
				ASSERT_RADIO(0);
			}*/

			cmd = CMD_NONE;

			// Check if mac issued a sleep while i was receiving something
			if(sleep_pending)
			{
				Sleep(0);
				return;
			}
			else { // Now safe to go back to RX_ON
				Careful_State_Change_Extended(RX_AACK_ON);
				state = STATE_RX_AACK_ON;
			}
		}
		else
		{ // Something happened. unclear what. Try to go to a safe state.
#ifndef NDEBUG
			//hal_printf("Warning: RF231: Strangeness at line %d\r\n", __LINE__);
			hal_printf("irq_cause is: %u; cmd is: %d; line: %d\n", irq_cause, cmd, __LINE__);
#endif

#ifdef DEBUG_RF231
			radio_hal_trx_status_t trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			hal_printf("1. current status inside handle interrupt is: %d; state is: %d\n", trx_status, state);
			Careful_State_Change(PLL_ON);
			state = STATE_PLL_ON;
			trx_status = (radio_hal_trx_status_t) (VERIFY_STATE_CHANGE);
			hal_printf("2. current state inside handle interrupt is: %d\n", trx_status);
#endif

			if(RF231_extended_mode){
				Careful_State_Change(TRX_OFF);
				state = STATE_TRX_OFF;
			}
			else{
				Careful_State_Change(TRX_OFF);
				state = STATE_TRX_OFF;
			}
		}
	}

	if(sleep_pending)
	{
		Sleep(0);
	}
}	//RF231Radio::HandleInterrupt()


// Re-writing this crap
DeviceStatus RF231Radio::DownloadMessage()
{
	DeviceStatus retStatus = DS_Success;
	UINT8 phy_rssi_reg_value = ReadRegister(RF230_PHY_RSSI);
	UINT8 rx_crc_valid = phy_rssi_reg_value & 0x80;
	//UINT8 rssi = phy_rssi_reg_value & RF230_RSSI_MASK;

//	UINT16 rssi = GetAverageOrMaxRSSI_countN(8);

	UINT8 phy_status;
	UINT8 len;
	UINT8 lqi;
	UINT32 cnt = 0;
	UINT8* temp_rx_msg_ptr;

	// Auto-CRC is enabled. This checks the status bit.
	if ( !rx_crc_valid ) {
		retStatus = DS_Fail;
	}

	GLOBAL_LOCK(irq);

	// DEBUG NATHAN
	add_rx_time();
	// DEBUG NATHAN

	temp_rx_msg_ptr = (UINT8 *) rx_msg_ptr;
	memset(temp_rx_msg_ptr, 0,  IEEE802_15_4_FRAME_LENGTH-sizeof(IEEE802_15_4_Metadata_t));

	//RF231_240NS_DELAY();
	SelnClear();
	//RF231_240NS_DELAY();

	// phy_status could contain meta data depending on settings.
	// At the moment it does not.
	phy_status = CPU_SPI_WriteReadByte(config, RF230_CMD_FRAME_READ);
	//RF231_240NS_DELAY();

	// next byte is legnth to read including CRC
	len = length = CPU_SPI_WriteReadByte(config, 0);
	//RF231_240NS_DELAY();

	// We don't want to read the two CRC bytes into the packet
	rx_length = len - 2;

	while ( ((len--) -2) > 0) {
		temp_rx_msg_ptr[cnt++] = CPU_SPI_WriteReadByte(config, 0);
		//RF231_240NS_DELAY();
	}

	// Two dummy reads for the CRC bytes
	CPU_SPI_WriteReadByte(config, 0); //RF231_240NS_DELAY();
	CPU_SPI_WriteReadByte(config, 0); //RF231_240NS_DELAY();

	// last, the LQI
	lqi = CPU_SPI_WriteReadByte(config, 0); //RF231_240NS_DELAY();

	SelnSet();

	IEEE802_15_4_Metadata_t* metadata = rx_msg_ptr->GetMetaData();
	metadata->SetRssi(ReadRegister(RF230_PHY_ED_LEVEL));  //BK: In extended mode ED detection is recommended over manual RSSI measurements. It is also recommended to use automatic ED detection rather than manual measurements.
	metadata->SetLqi(lqi);
	metadata->SetReceiveTimeStamp(receive_timestamp);

	return retStatus;
}

DeviceStatus RF231Radio::DownloadMessage(UINT16 tmp_length)
{
	DeviceStatus retStatus = DS_Success;
	UINT8 phy_rssi_reg_value = ReadRegister(RF230_PHY_RSSI);
	UINT8 rx_crc_valid = phy_rssi_reg_value & 0x80;
	//UINT8 rssi = phy_rssi_reg_value & RF230_RSSI_MASK;

//	UINT16 rssi = GetAverageOrMaxRSSI_countN(8);

	UINT8 phy_status;
	UINT8 len;
	UINT8 lqi;
	UINT32 cnt = 0;
	UINT8* temp_rx_msg_ptr;

	// Auto-CRC is enabled. This checks the status bit.
	if ( !rx_crc_valid ) {
		retStatus = DS_Fail;
	}

	GLOBAL_LOCK(irq);

	// DEBUG NATHAN
	//add_rx_time();
	// DEBUG NATHAN

	temp_rx_msg_ptr = (UINT8 *) rx_msg_ptr;
	memset(temp_rx_msg_ptr, 0,  IEEE802_15_4_FRAME_LENGTH);

	//RF231_240NS_DELAY();
	SelnClear();
	//RF231_240NS_DELAY();

	// phy_status could contain meta data depending on settings.
	// At the moment it does not.

	phy_status = CPU_SPI_WriteReadByte(config, RF230_CMD_FRAME_READ);
	//RF231_240NS_DELAY();

	// next byte is legnth to read including CRC
	len = length = CPU_SPI_WriteReadByte(config, 0);
	//RF231_240NS_DELAY();

	// We don't want to read the two CRC bytes into the packet
	rx_length = len - 2;

	len = tmp_length+2;

	while ( ((len--) -2) > 0) {
		temp_rx_msg_ptr[cnt++] = CPU_SPI_WriteReadByte(config, 0);
		/*if(cnt > tmp_length){
			break;
		}*/
		//RF231_240NS_DELAY();
	}

	// Two dummy reads for the CRC bytes
	/*
	CPU_SPI_WriteReadByte(config, 0); //RF231_240NS_DELAY();
	CPU_SPI_WriteReadByte(config, 0); //RF231_240NS_DELAY();
	// last, the LQI
	lqi = CPU_SPI_WriteReadByte(config, 0); //RF231_240NS_DELAY();
	*/

	SelnSet();

	/*IEEE802_15_4_Metadata_t* metadata = rx_msg_ptr->GetMetaData();
	metadata->SetLqi(lqi);
	metadata->SetReceiveTimeStamp(receive_timestamp);*/

	return retStatus;
}

void Radio_Handler_LR(GPIO_PIN Pin,BOOL PinState, void* Param)
{
	grf231RadioLR.HandleInterrupt();
}

void Radio_Handler(GPIO_PIN Pin, BOOL PinState, void* Param)
{
	grf231Radio.HandleInterrupt();
}

