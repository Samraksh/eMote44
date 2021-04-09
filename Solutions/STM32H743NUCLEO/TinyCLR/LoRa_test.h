#pragma once

#include <tinyhal.h>


#include <SX1276/SX1276.h>
#include <SX1276/parameters.h>
//#include <pwr/netmf_pwr_wakelock.h>

// MF normally doesn't allow us to use these functions
// I don't know why and I assume for an obsolete or not good reason.
#undef strcpy
#undef strncpy
#undef strlen
#undef strncmp

typedef struct my_pkt_struct {
	UINT8 count;
	char name[23];
} my_pkt_t;

typedef struct channel_info_pkt_struct {
	UINT8 packet_id;
	UINT8 next_channel;
	UINT16 node_id;
} channel_info_pkt_t;

typedef struct data_pkt_struct {
	UINT8 packet_id;
	UINT8 count;
	UINT16 node_id;
	char name[23];
} data_pkt_t;

typedef struct ack_pkt_struct {
	UINT8 packet_id;
	UINT16 node_id;
} ack_pkt_t;

typedef struct info_pkt_struct {
	UINT8 packet_id;
	UINT16 node_id;
	UINT8 info[25];
} info_pkt_t;

void ChangeState();
void ApplicationEntryPoint();