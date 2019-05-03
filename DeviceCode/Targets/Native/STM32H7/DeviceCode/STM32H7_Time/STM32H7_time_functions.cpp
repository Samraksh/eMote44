////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//  Implementation for STM32H7: Copyright (c) Oberon microsystems, Inc.
//
//  *** System Timer Driver ***
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include "..\stm32h7xx.h"

static UINT64 g_nextEvent;   // tick time of next event to be scheduled
static TIM_HandleTypeDef    TimHandle2;

static void Error_Handler(void)
{
	__asm__("BKPT");
}

extern "C" {
void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle2);
}
} // extern "C"

UINT32 CPU_SystemClock()
{
    return SYSTEM_CLOCK_HZ;
}

UINT32 CPU_TicksPerSecond()
{
    return SLOW_CLOCKS_PER_SECOND;
}

UINT64 CPU_TicksToTime( UINT64 Ticks )
{
    Ticks *= (TEN_MHZ               /SLOW_CLOCKS_TEN_MHZ_GCD);
    Ticks /= (SLOW_CLOCKS_PER_SECOND/SLOW_CLOCKS_TEN_MHZ_GCD);

    return Ticks;
}

UINT64 CPU_TicksToTime( UINT32 Ticks32 )
{
    UINT64 Ticks;

    Ticks  = (UINT64)Ticks32 * (TEN_MHZ               /SLOW_CLOCKS_TEN_MHZ_GCD);
    Ticks /=                   (SLOW_CLOCKS_PER_SECOND/SLOW_CLOCKS_TEN_MHZ_GCD);

    return Ticks;
}

UINT64 CPU_MillisecondsToTicks( UINT64 Ticks )
{
    Ticks *= (SLOW_CLOCKS_PER_SECOND/SLOW_CLOCKS_MILLISECOND_GCD);
    Ticks /= (1000                  /SLOW_CLOCKS_MILLISECOND_GCD);

    return Ticks;
}

UINT64 CPU_MillisecondsToTicks( UINT32 Ticks32 )
{
    UINT64 Ticks;

    Ticks  = (UINT64)Ticks32 * (SLOW_CLOCKS_PER_SECOND/SLOW_CLOCKS_MILLISECOND_GCD);
    Ticks /=                   (1000                  /SLOW_CLOCKS_MILLISECOND_GCD);

    return Ticks;
}


UINT64 CPU_MicrosecondsToTicks( UINT64 uSec )
{
#if ONE_MHZ <= SLOW_CLOCKS_PER_SECOND
    return uSec * (SLOW_CLOCKS_PER_SECOND / ONE_MHZ);
#else
    return uSec / (ONE_MHZ / SLOW_CLOCKS_PER_SECOND);
#endif
}

UINT32 CPU_MicrosecondsToTicks( UINT32 uSec )
{
#if ONE_MHZ <= SLOW_CLOCKS_PER_SECOND
    return uSec * (SLOW_CLOCKS_PER_SECOND / ONE_MHZ);
#else
    return uSec / (ONE_MHZ / SLOW_CLOCKS_PER_SECOND);
#endif
}

UINT32 CPU_MicrosecondsToSystemClocks( UINT32 uSec )
{
    uSec *= (SYSTEM_CLOCK_HZ/CLOCK_COMMON_FACTOR);
    uSec /= (ONE_MHZ        /CLOCK_COMMON_FACTOR);

    return uSec;
}

int CPU_MicrosecondsToSystemClocks( int uSec )
{
    uSec *= (SYSTEM_CLOCK_HZ/CLOCK_COMMON_FACTOR);
    uSec /= (ONE_MHZ        /CLOCK_COMMON_FACTOR);

    return uSec;
}

int CPU_SystemClocksToMicroseconds( int Ticks )
{
    Ticks *= (ONE_MHZ        /CLOCK_COMMON_FACTOR);
    Ticks /= (SYSTEM_CLOCK_HZ/CLOCK_COMMON_FACTOR);

    return Ticks;
}

static void Timer_Interrupt (void* param) // 32 bit timer compare event
{
    if (HAL_Time_CurrentTicks() >= g_nextEvent) { // handle event
       HAL_COMPLETION::DequeueAndExec(); // this also schedules the next one, if there is one
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		Timer_Interrupt(NULL);
	}
}

UINT64 HAL_Time_CurrentTicks()
{
	return TimHandle2.Instance->CNT;
    // UINT32 t2, t5; // cascaded timers
    // do {
        // t5 = TimHandle5.Instance->CNT;
        // t2 = TimHandle2.Instance->CNT;
    // } while (t5 != TimHandle5.Instance->CNT); // asure consistent values
    // return t2 | (UINT64)t5 << 32;
}

void HAL_Time_SetCompare( UINT64 CompareValue )
{
    GLOBAL_LOCK(irq);
    g_nextEvent = CompareValue;
    TimHandle2.Instance->CCR1 = (UINT32)CompareValue; // compare to low bits

    if (HAL_Time_CurrentTicks() >= CompareValue) { // missed event
        // trigger immediate interrupt
        TimHandle2.Instance->EGR = TIM_EGR_CC1G; // trigger compare1 event
    }

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2) {
	  __HAL_RCC_TIM2_CLK_ENABLE();
	  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  }
  // else if (htim->Instance == TIM5) {
	  // __HAL_RCC_TIM5_CLK_ENABLE();
	  // HAL_NVIC_EnableIRQ(TIM5_IRQn);
  // }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2) {
		HAL_NVIC_DisableIRQ(TIM2_IRQn);
	}
	// else if (htim->Instance == TIM5) {
		// HAL_NVIC_DisableIRQ(TIM5_IRQn);
	// }
}

BOOL HAL_Time_Initialize()
{
	uint32_t uwPrescalerValue = (SystemCoreClock / 1000000) -1;

	// TimHandle5.Instance = TIM5;
	// TimHandle5.Init.Period            = 0xFFFFFFFF;
	// TimHandle5.Init.Prescaler         = uwPrescalerValue;
	// TimHandle5.Init.ClockDivision     = 0;
	// TimHandle5.Init.CounterMode       = TIM_COUNTERMODE_UP;
	// TimHandle5.Init.RepetitionCounter = 0;

	// if (HAL_TIM_Base_Init(&TimHandle5) != HAL_OK)
	// {
		// Error_Handler();
	// }

	TimHandle2.Instance = TIM2;
	TimHandle2.Init.Period            = 0xFFFFFFFF;
	TimHandle2.Init.Prescaler         = uwPrescalerValue;
	TimHandle2.Init.ClockDivision     = 0;
	TimHandle2.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle2.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&TimHandle2) != HAL_OK)
	{
		Error_Handler();
	}

	//if (HAL_TIM_Base_Start_IT(&TimHandle5) != HAL_OK) // We don't want TIM_IT_UPDATE I think?
	// if (HAL_TIM_Base_Start(&TimHandle5) != HAL_OK)
	// {
		// Error_Handler();
	// }

	// FIXME: Needs TIM_IT_UPDATE and CC or whatever
	//if (HAL_TIM_Base_Start_IT(&TimHandle2) != HAL_OK)
	if (HAL_TIM_Base_Start(&TimHandle2) != HAL_OK)
	{
		Error_Handler();
	}
	return TRUE;
}

BOOL HAL_Time_Uninitialize()
{

	// if (HAL_TIM_Base_Stop_IT(&TimHandle5) != HAL_OK)
	// {
		// /* Starting Error */
		// Error_Handler();
	// }

	// if (HAL_TIM_Base_DeInit(&TimHandle5) != HAL_OK)
	// {
		// /* Initialization Error */
		// Error_Handler();
	// }
	if (HAL_TIM_Base_Stop_IT(&TimHandle2) != HAL_OK)
	{
		/* Starting Error */
		Error_Handler();
	}

	if (HAL_TIM_Base_DeInit(&TimHandle2) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
    return TRUE;
}

//
// To calibrate this constant, uncomment #define CALIBRATE_SLEEP_USEC in TinyHAL.c
//
#define STM32H7_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS 3

void HAL_Time_Sleep_MicroSeconds( UINT32 uSec )
{
    GLOBAL_LOCK(irq);

    UINT32 current   = HAL_Time_CurrentTicks();
    UINT32 maxDiff = CPU_MicrosecondsToTicks( uSec );

    if(maxDiff <= STM32H7_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS) maxDiff  = 0;
    else maxDiff -= STM32H7_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS;

    while(((INT32)(HAL_Time_CurrentTicks() - current)) <= maxDiff);
}

void HAL_Time_Sleep_MicroSeconds_InterruptEnabled( UINT32 uSec )
{
    // iterations must be signed so that negative iterations will result in the minimum delay

    uSec *= (SYSTEM_CYCLE_CLOCK_HZ / CLOCK_COMMON_FACTOR);
    uSec /= (ONE_MHZ               / CLOCK_COMMON_FACTOR);

    // iterations is equal to the number of CPU instruction cycles in the required time minus
    // overhead cycles required to call this subroutine.
    int iterations = (int)uSec - 5;      // Subtract off call & calculation overhead

    CYCLE_DELAY_LOOP(iterations);
}

INT64 HAL_Time_TicksToTime( UINT64 Ticks )
{
    return CPU_TicksToTime( Ticks );
}

INT64 HAL_Time_CurrentTime()
{
    return CPU_TicksToTime( HAL_Time_CurrentTicks() );
}

void HAL_Time_GetDriftParameters  ( INT32* a, INT32* b, INT64* c )
{
    *a = 1;
    *b = 1;
    *c = 0;
}



//******************** Profiler ********************

UINT64 Time_CurrentTicks()
{
    return HAL_Time_CurrentTicks();
}
