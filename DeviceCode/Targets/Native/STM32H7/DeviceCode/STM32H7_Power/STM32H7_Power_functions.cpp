//////////////////////////////////////////////////////////////////////////////////////////
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
//  Copyright (c) Microsoft Open Technologies. All rights reserved.
//  Implementation for STM32H7: Copyright (c) Oberon microsystems, Inc.
//
//  *** CPU Power States ***
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include <stm32h7xx_hal.h>
#include "STM32H7_Power_functions.h"
#include "..\stm32h7xx.h"


static void (*g_STM32H7_stopHandler)();
static void (*g_STM32H7_restartHandler)();


void STM32H7_SetPowerHandlers(void (*stop)(), void (*restart)())
{
    g_STM32H7_stopHandler = stop;
    g_STM32H7_restartHandler = restart;
}


BOOL CPU_Initialize()
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();
    //CPU_INTC_Initialize();
    return TRUE;
}

extern void MaxSystemClock_Config(void);
extern void MinSystemClock_Config(void);

static void set_cpu_480(void) {
	GLOBAL_LOCK(irq);
	MaxSystemClock_Config();
}

static void set_cpu_60(void) {
	GLOBAL_LOCK(irq);
	MinSystemClock_Config();
}

void CPU_ChangePowerLevel(POWER_LEVEL level)
{
    switch(level)
    {
        case POWER_LEVEL__MID_POWER: set_cpu_480();
            break;

        case POWER_LEVEL__LOW_POWER: set_cpu_60();
            break;

        case POWER_LEVEL__HIGH_POWER:
        default: set_cpu_480();
            break;
    }
}

#ifdef _DEBUG
static int check_pending_isr(void) {
	return SCB->ICSR & SCB_ICSR_ISRPENDING_Msk;
}

static int get_count_isr(void) {
	int ret=0;
	for(int i=0; i<150; i++) {
		if (NVIC_GetPendingIRQ((IRQn_Type)i)) {
			ret++;
		}
	}
	return ret;
}

static int get_first_isr_after(int after) {
	for(int i=after; i<150; i++) {
		if (NVIC_GetPendingIRQ((IRQn_Type)i)) {
			return i;
		}
	}
	return 0; // we don't use watchdog
}
#endif

void HAL_CPU_Sleep( SLEEP_LEVEL level, UINT64 wakeEvents )
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();

    switch(level) {
		case SLEEP_LEVEL__SLEEP: __WFI(); break;
		case SLEEP_LEVEL__DEEP_SLEEP:
		case SLEEP_LEVEL__OFF:
		default: __BKPT(); // No other sleep levels are supported
    }

// #ifdef _DEBUG
	// if (!check_pending_isr()) __BKPT(); // lets try to figure out who woke us...
	// volatile int count = get_count_isr();
	// volatile int first = get_first_isr_after(0);
// #endif
}

void CPU_Halt()  // unrecoverable error
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();
    while(1);
}

void CPU_Reset()
{
	HAL_NVIC_SystemReset();
    /*NATIVE_PROFILE_HAL_PROCESSOR_POWER();
    SCB->AIRCR = (0x5FA << SCB_AIRCR_VECTKEY_Pos)  // unlock key
               | (1 << SCB_AIRCR_SYSRESETREQ_Pos); // reset request
    while(1); // wait for reset*/
}

BOOL CPU_IsSoftRebootSupported ()
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();
    return TRUE;
}

void HAL_AssertEx()
{
    __BKPT(0);
    while(true) { /*nop*/ }
}


#ifdef EMOTE_WAKELOCKS
static uint32_t wakelock;
static UINT64 waketime;

void WakeLockInit(void) {
	wakelock = 0;
#if defined (EMOTE_WAKELOCK_STARTUP) && (EMOTE_WAKELOCK_STARTUP > 0)
	waketime = HAL_Time_CurrentTicks() + CPU_MicrosecondsToTicks((UINT32)1000000 * EMOTE_WAKELOCK_STARTUP);
#else
	waketime = 0;
#endif
}

void WakeLock(uint32_t lock) {
#ifdef POWER_PROFILE_HACK
	power_event_add(RTC_GetCounter(), WAKELOCK_ON, lock, -1);
#endif
	GLOBAL_LOCK(irq);
	wakelock |= lock;
}

void WakeUnlock(uint32_t lock) {
#ifdef POWER_PROFILE_HACK
	power_event_add(RTC_GetCounter(), WAKELOCK_OFF, lock, -1);
#endif
	GLOBAL_LOCK(irq);
	wakelock &= ~lock;
}

void WakeUntil(UINT64 until) {
	UINT64 now;

	GLOBAL_LOCK(irq);

	// Value of 0 will force kill the wakelock
	if (until == 0) {
		waketime = 0;
		return;
	}

	now = HAL_Time_CurrentTicks();

	// Make sure time is valid (in the future)
	// Make sure time is not before previous time.
	if (until < now || until < waketime) {
		return;
	}

	waketime = until;
}

bool GetWakeLocked(void) {
	bool doWFI;
	ASSERT_IRQ_MUST_BE_OFF();
	if (wakelock) { doWFI = TRUE; }
	else if (waketime > 0) {
		UINT64 now = HAL_Time_CurrentTicks();
		if (waketime >= now) {
			doWFI = TRUE; // Wakelocked
		} else {
			waketime = 0;
			doWFI = FALSE;
		}
	} else { doWFI = FALSE; }
	return doWFI;
}

#else // EMOTE_WAKELOCKS
void WakeLock(uint32_t lock) {}
void WakeUnlock(uint32_t lock) {}
void WakeUntil(UINT64 until) {}
bool GetWakeLocked(void) {return false;}
void WakeLockInit(void) {}
#endif // EMOTE_WAKELOCKS