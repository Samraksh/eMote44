#include <tinyhal.h>
#include "usb_serial_ext.h"

extern "C" {
#include "serial_frame.h"
}

#ifdef _DEBUG
static unsigned dropped_strings;
#endif

#ifndef JUMBO_FRAME_MAX
#define JUMBO_FRAME_MAX (128*1024) // 128 kiB
#endif

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
#ifdef _DEBUG
	if (buf == NULL) { __BKPT(); dropped_strings++; return sz; }
#else
	if (buf == NULL) { __BKPT(); return sz; }
#endif
	memset(buf, 0, buf_max);
	ret = serial_frame_encode(data, sz, buf_max, buf, DEST_BASE, frame_type);

	if (ret > 0) {
		usb_serial_ext_free(ret); // Free, add to TX buffer, and queues TX all in one. What a deal.
	}
	else {
		__BKPT();
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
	if (buf == NULL) { __BKPT(); return 0; }
	memset(buf, 0, buf_max);
	ret = serial_frame_encode(data, sz, buf_max, buf, DEST_BASE, frame_type);

	if (ret > 0) {
		usb_serial_ext_free(ret); // Free, add to TX buffer, and queues TX all in one. What a deal.
	}
	else {
		__BKPT();
		usb_serial_ext_free(0);
	}
	return sz;
}

void * serial_frame_malloc(size_t size) { return usb_serial_ext_malloc(size); }
