#pragma once

#include <tinyhal.h>


#include <radio/SX1276/SX1276wrapper.h>
#include <radio/SX1276/SamrakshSX1276Parameters.h>
//#include <pwr/netmf_pwr_wakelock.h>

// MF normally doesn't allow us to use these functions
// I don't know why and I assume for an obsolete or not good reason.
#undef strcpy
#undef strncpy
#undef strlen
#undef strncmp

typedef struct my_pkt_struct {
	unsigned count;
	char name[8];
} my_pkt_t;
