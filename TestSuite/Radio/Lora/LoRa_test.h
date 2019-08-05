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
	unsigned count;
	char name[23];
} my_pkt_t;
