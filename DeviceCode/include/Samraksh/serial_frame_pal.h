#pragma once

// extern "C" {
// #include "serial_frame.h"
// }

// FIX ME: bleah, this is also in serial_frame.h
// enum {
	// FRAME_TYPE_DEBUG_STRING		=0,
	// FRAME_TYPE_DATA_STRING		=1,
	// FRAME_TYPE_BOOTLOADER_BIN	=2,
	// FRAME_TYPE_H7_PROTOBUF		=3,
	// FRAME_TYPE_BASE_PROTOBUF	=4,
	// FRAME_TYPE_F1_PROTOBUF		=5,
	// FRAME_TYPE_BIN_AUDIO		=6,
// };

// Shouldn't need to do __packed__ here
typedef struct {
	uint32_t version;
	uint32_t hour_idx;
	uint32_t cells[4];
	uint32_t tot;
	uint32_t power_in_24[24];
	uint32_t power_out_24[24];
	uint32_t solar_volt_24[24];
	float temperature_24[24];
} bms_rx_v6_t;

int send_framed_serial(const uint8_t *data, unsigned sz, BOOL isDebug);
int send_framed_serial_data(const uint8_t *data, unsigned sz, uint32_t frame_type);
void rx_framed_serial(uint8_t* buf, uint32_t len);
int read_serial_frame_buffer(uint8_t *buf, size_t size);
void framed_serial_init(void);
bms_rx_v6_t * get_bms_data_v6(void);
uint32_t get_bms_last_idx(void);