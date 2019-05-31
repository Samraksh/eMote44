////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRIVERS_H_
#define _DRIVERS_H_ 1

/////////////////////////////////////////////////////////////////////
//
// Chipset
//

// boot
#include <CPU_BOOT_decl.h>

// Cache driver
#include <CPU_MMU_decl.h>

// Cache driver
#include <CPU_CACHE_decl.h>

// Gp I/O driver
#include <CPU_INTC_decl.h>

// Gp I/O driver
#include <CPU_GPIO_decl.h>

// Watchdog driver
#include <CPU_WATCHDOG_decl.h>

// SPI driver
#include <CPU_SPI_decl.h>

// External bus interface driver
#include <CPU_EBIU_decl.h>

// Power control unit
#include <CPU_PCU_decl.h>

// Clock management unit driver
#include <CPU_CMU_decl.h>

// DMA driver
#include <CPU_DMA_decl.h>

#include <PerformanceCounters_decl.h>

// Virtual Key
#include <VirtualKey_decl.h>

// Power API
#include <Power_decl.h>

#ifdef STM32H743xx
// RTC API
#include <CPU_RTC_decl.h>
#endif
//
// Chipset
//
/////////////////////////////////////////////////////////////////////

#endif // _DRIVERS_H_

