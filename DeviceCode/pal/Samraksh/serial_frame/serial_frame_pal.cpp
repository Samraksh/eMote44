#include <tinyhal.h>
#include "usb_serial_ext.h"

extern "C" {
#include "serial_frame.h"
}

#ifndef JUMBO_FRAME_MAX
#define JUMBO_FRAME_MAX (128*1024) // 128 kiB
#endif

// Mostly hold-over from host side
typedef struct {
	//FILE *audio_file;
	unsigned audio_bytes_written;
	unsigned debug_bytes_written;
	unsigned data_bytes_written;
	unsigned frame_count;
} mel_status_t;

#ifdef PLATFORM_ARM_STM32H743NUCLEO
static uint8_t rx_buf[FRAME_MAX_SIZE] __attribute__ (( section (".ram_d1"), aligned(32) ));
#else
static uint8_t rx_buf[FRAME_MAX_SIZE];
#endif

static HAL_CONTINUATION rx_buf_do;
static HAL_CONTINUATION bms_rx_do;
static volatile unsigned rx_cnt=0;
static mel_status_t serial_frame_status;

static void my_free(uint32_t x);

int send_framed_serial_data(const uint8_t *data, unsigned sz, uint32_t frame_type);

static void copy_buf_to_clr(uint8_t *buf, unsigned len) {
	// TODO
}

static void frame_error_handler(void) {
	//__BKPT();
}

static void debug_frame_handler(serial_frame_t *f, mel_status_t *status) {
	hal_printf("Got debug frame!\r\n");
	status->debug_bytes_written += f->sz;
}

static void bootloader_frame_handler(serial_frame_t *f, mel_status_t *status) {
#ifdef BOOTLOADER_MAGIC_ADDR
	__disable_irq();
	*((volatile unsigned *)BOOTLOADER_MAGIC_ADDR) = BOOTLOADER_MAGIC_WORD;
	SCB_CleanDCache();
	NVIC_SystemReset(); // Reboot to bootloader
#endif
}

static void data_string_frame_handler(serial_frame_t *f, mel_status_t *status) {
	copy_buf_to_clr(f->buf, f->sz);
	status->data_bytes_written += f->sz;
}

static void handle_frame(serial_frame_t *f, mel_status_t *status) {
	switch(f->type) {
		case FRAME_TYPE_DEBUG_STRING: debug_frame_handler(f, status); break;
		case FRAME_TYPE_BOOTLOADER_BIN: bootloader_frame_handler(f, status); break;
		case FRAME_TYPE_DATA_STRING: data_string_frame_handler(f, status); break;
		// case FRAME_TYPE_BIN_AUDIO: audio_frame_handler(f, status); break;
		default: break;
	}
	status->frame_count++;
}

#define CHECK_FLAG(x) {if (flag & x) hal_printf("Flag: " #x "\r\n");}
static bool parse_frame(serial_frame_t *f, mel_status_t *status) {
	if (f == NULL) { frame_error_handler(); return false; }
	frame_flag_t flag = f->flag;
	if (f->err == NO_FRAME) return false;
	if (f->err) hal_printf("Frame Error %d\r\n", f->err);

	CHECK_FLAG(NONE_FLAG);
	//CHECK_FLAG(FRAME_FOUND);
	//CHECK_FLAG(PARTIAL);
	CHECK_FLAG(NO_PAYLOAD);
	CHECK_FLAG(CRC_ERROR);

	if (flag & CRC_ERROR) {
		hal_printf("DEBUG: PAYLOAD SIZE: %u\r\n", f->sz);
	}

	if (flag & FRAME_FOUND) {
		handle_frame(f, status);
		my_free(f->sz);
		f->buf = NULL;
		f->sz = 0;
		return true;
	}
	return false;
}

static void handle_serial_rx(void *p) {
	serial_frame_t f = {0};
	int decode_ret;
	unsigned cnt = rx_cnt; // shadow this, interrupt may hit
	bool go = false;
	int i=0;

	do {
		decode_ret = serial_frame_decode(&rx_buf[i], cnt-i, &f);
		if (decode_ret < 0) { frame_error_handler(); break; }
		i += decode_ret;
		go = parse_frame(&f, &serial_frame_status);
	} while (go);

	GLOBAL_LOCK(irq);

	// Easy case, no bytes came in while we were parsing
	if (rx_cnt == cnt) {
		goto out;
	}

#ifdef _DEBUG
	if (rx_cnt < cnt) frame_error_handler(); // sanity check
#endif

	// Bytes came in, must shift the new data down to orgin 0
	memcpy(rx_buf, &rx_buf[cnt], rx_cnt-cnt);
	rx_buf_do.Enqueue(); // re-queue for the new data
out:
	rx_cnt = 0;
	// IRQ lock released
	return;
}

// Does a raw read and writes up to 'size' chars into the buf
// TODO THIS FUNCTION DOES NOT WORK IF BUFFER NOT COMPLETELY EMPTIED
int read_serial_frame_buffer(uint8_t *buf, size_t size) {
	int count;
	if (size == 0) return rx_cnt; // Returns available bytes
	GLOBAL_LOCK(irq);
	if (size > rx_cnt) count = rx_cnt;
	else count = size;
	memcpy(buf, rx_buf, count);
	rx_cnt -= count;
	#ifdef ALLOW_BKPT
	if (rx_cnt > 0) __BKPT();
	#endif
	return count;
}

// Expected to be called in ISR
// Hack it and set a continuation and fake a UART serial port
// Really this should integrate more nicely with the CLR but I don't have it in me right now.
void rx_framed_serial(uint8_t* buf, uint32_t len) {
	if (rx_cnt + len > FRAME_MAX_SIZE) {
		frame_error_handler();
		return; // no room, drop the data
	}
	memcpy(&rx_buf[rx_cnt], buf, len);
	rx_cnt += len;
	//rx_buf_do.Enqueue(); // TODO REENABLE ME
	//Events_Set( SYSTEM_EVENT_FLAG_COM_IN );
}

extern UART_HandleTypeDef huart2;
UART_HandleTypeDef *BMS_UART = &huart2;
static volatile uint8_t bms_rx_buf[512];
static volatile uint32_t bms_rx_bytes;
static const bool bms_active_transmit = false; // not used for now

static int get_cts(void) { return CPU_GPIO_GetPinState( _P(A,0) ); }
static void bms_clear_rts(void)  { CPU_GPIO_SetPinState( _P(A,1), FALSE); }
static void bms_assert_rts(void) { CPU_GPIO_SetPinState( _P(A,1), TRUE); }

static void set_uart_rx_it(bool set) {
	if (set) {
		SET_BIT  (BMS_UART->Instance->CR1, USART_CR1_RXNEIE_RXFNEIE);
	}
	else {
		CLEAR_BIT(BMS_UART->Instance->CR1, USART_CR1_RXNEIE_RXFNEIE);
	}
}

static void handle_bms_rx(void *p) {
	set_uart_rx_it(false);
	//hal_printf("Got %lu bytes BMS data\r\n", bms_rx_bytes);
}

static void got_bms_rts(GPIO_PIN Pin, BOOL PinState, void* context) {
	if (Pin != 0) return; // Sanity check the pin
	if (PinState == FALSE) { // Transmission complete
		bms_rx_do.Enqueue();
		return;
	}
	// Setup for incoming BMS transmission
	bms_rx_bytes = 0;
	set_uart_rx_it(true);
	bms_assert_rts();
}

static void bms_uart_irq_handler(void) {
	uint8_t data = (uint8_t)(BMS_UART->Instance->RDR & (uint8_t)0xFF);
	if (bms_rx_bytes == 0 && !bms_active_transmit) bms_clear_rts();
	if (bms_rx_bytes >= sizeof(bms_rx_buf)) return; // ignore out of bounds
	bms_rx_buf[bms_rx_bytes++] = data;
}

extern "C" {
void USART2_IRQHandler(void) {
	bms_uart_irq_handler();
}
}

void framed_serial_init(void) {
#ifdef BOOTLOADER_MAGIC_ADDR
	*((volatile uint32_t *)BOOTLOADER_MAGIC_ADDR) = 0;
#endif
	//send_framed_serial_data(NULL, 0, FRAME_TYPE_HELLO); // Kick out empty "hello" frame
	rx_buf_do.InitializeCallback(&handle_serial_rx, NULL);
	bms_rx_do.InitializeCallback(&handle_bms_rx, NULL);
	CPU_GPIO_EnableInputPin ( _P(A,0), FALSE, got_bms_rts, GPIO_INT_EDGE_BOTH, RESISTOR_DISABLED);
	CPU_GPIO_EnableOutputPin( _P(A,1), FALSE);
}

// Simple version for TinyCLR, DEBUG and STRING types
// Will drop data instead of failing. Probably need to re-think that.
int send_framed_serial(const uint8_t *data, unsigned sz, BOOL isDebug) {
	int ret, usb_ret;
	uint32_t frame_type;
	uint8_t *buf;
	unsigned buf_max = (FRAME_MIN_SIZE + sz)*2;

	if (isDebug)
		frame_type = FRAME_TYPE_DEBUG_STRING;
	else
		frame_type = FRAME_TYPE_DATA_STRING;

	buf = (uint8_t *)usb_serial_ext_malloc(buf_max);
	if (buf == NULL) { frame_error_handler(); return sz; }
	memset(buf, 0, buf_max);
	ret = serial_frame_encode(data, sz, buf_max, buf, DEST_BASE, frame_type);

	if (ret > 0) {
		usb_serial_ext_free(ret); // Free, add to TX buffer, and queues TX all in one. What a deal.
	}
	else {
		frame_error_handler();
		usb_serial_ext_free(0);   // Error, cleanup
	}
	return sz;
}

// More general version for PAL
// Returns number of bytes queued, '0' on error (buffer full, no memory, etc.)
int send_framed_serial_data(const uint8_t *data, unsigned sz, uint32_t frame_type) {
	int ret, usb_ret;
	uint8_t *buf;
	unsigned buf_max;

	// The x2 assumption is absurd for very large frames, so relax it
	// serial_frame_encode() will gracefully fail in the worst case
	// Make sure the buffer (e.g., USB tx buffer) is >= JUMBO_FRAME_THRESH
	if (sz >= JUMBO_FRAME_MAX/2)
		buf_max = JUMBO_FRAME_MAX;
	else
		buf_max = (FRAME_MIN_SIZE + sz)*2;

	buf = (uint8_t *)usb_serial_ext_malloc(buf_max);
	if (buf == NULL) { frame_error_handler(); return 0; }
	//memset(buf, 0, buf_max);
	ret = serial_frame_encode(data, sz, buf_max, buf, DEST_BASE, frame_type);

	if (ret > 0) {
		usb_serial_ext_free(ret); // Free, add to TX buffer, and queues TX all in one. What a deal.
	}
	else {
		frame_error_handler();
		usb_serial_ext_free(0);
	}
	return sz;
}

#ifdef PLATFORM_ARM_STM32H743NUCLEO
static uint8_t frame_scrach_space[FRAME_MAX_SIZE] __attribute__ (( section (".ram_d1"), aligned(32) ));
#else
static uint8_t frame_scrach_space[FRAME_MAX_SIZE];
#endif

static bool frame_scatch_alloc;

static void my_free(uint32_t x) {
	frame_scatch_alloc = false;
}

// ONLY ONE ALLOC AT A TIME
void * serial_frame_malloc(size_t size) {
	if (size > FRAME_MAX_SIZE) return NULL;
	if (frame_scatch_alloc) return NULL;

	frame_scatch_alloc = true;
	return frame_scrach_space;
}

// void * serial_frame_malloc(size_t size) { return usb_serial_ext_malloc(size); }
