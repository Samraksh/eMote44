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

void CPU_ChangePowerLevel(POWER_LEVEL level)
{
    switch(level)
    {
        case POWER_LEVEL__MID_POWER:
            break;

        case POWER_LEVEL__LOW_POWER:
            break;

        case POWER_LEVEL__HIGH_POWER:
        default:
            break;
    }
}

void HAL_CPU_Sleep( SLEEP_LEVEL level, UINT64 wakeEvents )
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();

    switch(level)
    {
    case SLEEP_LEVEL__DEEP_SLEEP: // stop
        // stop peripherals if needed
        if (g_STM32H7_stopHandler != NULL)
            g_STM32H7_stopHandler();

        // TODO: Explicitly clear PWR_CR1_PDDS ?
//        PWR->CR1 |= PWR_CR1_FLPS | PWR_CR1_LPDS; // low power deepsleep
//        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
//        __WFI(); // stop clocks and wait for external interrupt

//#if SYSTEM_CRYSTAL_CLOCK_HZ != 0
//        RCC->CR |= RCC_CR_HSEON;             // HSE on
//#endif
//        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;  // reset deepsleep

//        while(!(RCC->CR & RCC_CR_HSERDY));
//        RCC->CR |= RCC_CR_PLLON;             // pll on
//        while(!(RCC->CR & RCC_CR_PLLRDY));
//        RCC->CFGR |= RCC_CFGR_SW_PLL1;        // sysclk = pll out
//#if SYSTEM_CRYSTAL_CLOCK_HZ != 0
//        RCC->CR &= ~RCC_CR_HSION;            // HSI off
//#endif
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

        // restart peripherals if needed
        if (g_STM32H7_restartHandler != NULL)
            g_STM32H7_restartHandler();
        return;
		
    case SLEEP_LEVEL__OFF: // standby
        // stop peripherals if needed
       // if (g_STM32H7_stopHandler != NULL)
       //     g_STM32H7_stopHandler();

       // PWR->CPUCR |= PWR_CPUCR_PDDS_D1; //PWR_CR1_PDDS; // power down deepsleep
       // SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
      //  __WFI(); // soft power off, never returns
        HAL_PWR_EnterSTANDBYMode();
		return;

    default: // sleep
        // TODO: Clear SLEEPDEEP bit in SCB->SCR ?
        //PWR->CR |= PWR_CR_CWUF;
        //__WFI(); // sleep and wait for interrupt
		HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
        return;
    }
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