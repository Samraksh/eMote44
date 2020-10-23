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
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"
#include "STM32H7_lptim_interface.h"


#if STM32H7_32B_TIMER == 2
    #define TIM_32 TIM2
    #define RCC_APB1ENR_TIM_32_EN RCC_APB1LENR_TIM2EN
    #define DBGMCU_APB1_FZ_DBG_TIM_32_STOP DBGMCU_APB1LFZ1_DBG_TIM2
    #define TIM_32_IRQn TIM2_IRQn
    #if STM32H7_16B_TIMER == 3
        #define TIM_16 TIM5
        #define RCC_APB1ENR_TIM_16_EN RCC_APB1LENR_TIM5EN
        #define TIM_SMCR_TS_BITS (TIM_SMCR_TS_0 * 1) // TS = 001
    #elif STM32H7_16B_TIMER == 4
        #define TIM_16 TIM4
        #define RCC_APB1ENR_TIM_16_EN RCC_APB1LENR_TIM4EN
        #define TIM_SMCR_TS_BITS (TIM_SMCR_TS_0 * 1) // TS = 001
    #elif STM32H7_16B_TIMER == 8
        #define TIM_16 TIM8
        #define RCC_APB2ENR_TIM_16_EN RCC_APB2ENR_TIM8EN
        #define TIM_SMCR_TS_BITS (TIM_SMCR_TS_0 * 1) // TS = 001
    #elif STM32H7_16B_TIMER == 9
        #define TIM_16 TIM9
        #define RCC_APB2ENR_TIM_16_EN RCC_APB2ENR_TIM9EN
        #define TIM_SMCR_TS_BITS (TIM_SMCR_TS_0 * 0) // TS = 000
    #else
        #error wrong 16 bit timer (3, 4, 8 or 9)
    #endif
#elif STM32H7_32B_TIMER == 5
    #define TIM_32 TIM5
    #define RCC_APB1ENR_TIM_32_EN RCC_APB1LENR_TIM5EN
    #define DBGMCU_APB1_FZ_DBG_TIM_32_STOP DBGMCU_APB1LFZ1_DBG_TIM5
    #define TIM_32_IRQn TIM5_IRQn
    #if STM32H7_16B_TIMER == 1
        #define TIM_16 TIM1
        #define RCC_APB2ENR_TIM_16_EN RCC_APB2ENR_TIM1EN
        #define TIM_SMCR_TS_BITS (TIM_SMCR_TS_0 * 0) // TS = 000
    #elif STM32H7_16B_TIMER == 3
        #define TIM_16 TIM5
        #define RCC_APB1ENR_TIM_16_EN RCC_APB1LENR_TIM5EN
        #define TIM_SMCR_TS_BITS (TIM_SMCR_TS_0 * 2) // TS = 010
    #elif STM32H7_16B_TIMER == 8
        #define TIM_16 TIM8
        #define RCC_APB2ENR_TIM_16_EN RCC_APB2ENR_TIM8EN
        #define TIM_SMCR_TS_BITS (TIM_SMCR_TS_0 * 3) // TS = 011
    #elif STM32H7_16B_TIMER == 12
        #define TIM_16 TIM12
        #define RCC_APB1ENR_TIM_16_EN RCC_APB1LENR_TIM12EN
        #define TIM_SMCR_TS_BITS (TIM_SMCR_TS_0 * 1) // TS = 001
    #else
        #error wrong 16 bit timer (1, 3, 8 or 12)
    #endif
#else
    #error wrong 32 bit timer (2 or 5)
#endif 

#define TIM_CLK_HZ (SYSTEM_APB1_CLOCK_HZ * 2)

#define GPIO_0 _P(B,12)
#define GPIO_1 _P(B,13)

TIM_HandleTypeDef    TimHandle2_SystemTime;
TIM_HandleTypeDef    TimHandle5;

HAL_CALLBACK_FPN lptimCallBackISR;
UINT32 lptimCallBackISR_Param;

static HAL_CONTINUATION LPTIM_interrupt_continuation;

HAL_CALLBACK_FPN tim5CallBackISR;
UINT32 callBackISR_Param;

volatile UINT64 m_systemTime = 0;

const UINT64 TIMER_5_TIME_CUSHION = 500;  // 5 us
const UINT64 TIMER_5_MAX_ALLOWABLE_WAIT = 0xFFFEFFFF;

HAL_CALLBACK_FPN earlyRtcCallBackISR;
UINT32 earlyRtcCallBackISR_Param;
//static HAL_CONTINUATION RTC_interrupt_continuation;

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
	__asm__("BKPT");
}

extern "C" {

// calls Virtual Timer callback
void TIM5_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle5);
}
  
// timer 2 keeps track of system time. Interrupt happens every 0xFFFF FFFF clocks (48MHz)
void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle2_SystemTime);	
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TimHandle2_SystemTime.Instance) {
		m_systemTime += (0x1ull <<32);

	} else if (htim->Instance == TimHandle5.Instance) {
		callBackISR_Param = ADVTIMER_32BIT;
		HAL_TIM_Base_Stop_IT(&TimHandle5);
		INTERRUPT_START
		tim5CallBackISR(&callBackISR_Param);
		INTERRUPT_END
	}
}

} // extern "C"

void queueLptimCallback(void){
	//LPTIM_interrupt_continuation.Enqueue();
	lptimCallBackISR(&lptimCallBackISR_Param);
}

void lptimSetCompareTriggered(void *arg){
	lptimCallBackISR(&lptimCallBackISR_Param);
}

UINT32 CPU_SystemClock()
{
    return SYSTEM_CLOCK_HZ;
}

UINT32 CPU_TicksPerSecond()
{
    return SLOW_CLOCKS_PER_SECOND;
}

UINT32 CPU_Timer_GetMaxTicks(UINT8 Timer)
{
	UINT32 maxTicks = 0;
	
	if(Timer == ADVTIMER_32BIT)
	{
		maxTicks = 0xFFFFFFFF;
	}
	/*else if(Timer == RTC_32BIT)
	{
		maxTicks = 0xFFFFFFFF;
	}*/
	else if(Timer == LPTIM)
	{
		maxTicks = 0x1FFFF;
	}
	return maxTicks;
}

//--//

UINT64 CPU_TicksToTime( UINT64 Ticks, UINT16 Timer )
{
	// A unit of Time is 100ns
	return (UINT64)(CPU_TicksToMicroseconds(Ticks, Timer) * 10);
}

UINT64 CPU_TicksToTime( UINT32 Ticks32, UINT16 Timer )
{
	// A unit of Time is 100ns
	return (UINT64)(CPU_TicksToMicroseconds(Ticks32, Timer) * 10);
}

//--//

UINT64 CPU_MillisecondsToTicks( UINT64 mSec, UINT16 Timer )
{
    UINT32 timerFrequency = TIM_CLK_HZ;
	UINT8 i;

	for (i=0; i<g_CountOfHardwareTimers; i++){
		if (Timer == g_HardwareTimerIDs[i]){
			timerFrequency = g_HardwareTimerFrequency[i];
		}
	}


	return ( mSec * (timerFrequency / 1000) );
}

UINT64 CPU_MillisecondsToTicks( UINT32 mSec, UINT16 Timer )
{
    UINT32 timerFrequency = TIM_CLK_HZ;
	UINT8 i;

	for (i=0; i<g_CountOfHardwareTimers; i++){
		if (Timer == g_HardwareTimerIDs[i]){
			timerFrequency = g_HardwareTimerFrequency[i];
		}
	}


	return ( mSec * (timerFrequency / 1000) );
}

//--//

UINT64 CPU_MicrosecondsToTicks( UINT64 uSec, UINT16 Timer )
{
	UINT32 timerFrequency = TIM_CLK_HZ;
	UINT8 i;

	for (i=0; i<g_CountOfHardwareTimers; i++){
		if (Timer == g_HardwareTimerIDs[i]){
			timerFrequency = g_HardwareTimerFrequency[i];
		}
	}

		return ( uSec * (timerFrequency / CLOCK_COMMON_FACTOR) );
}

UINT32 CPU_MicrosecondsToTicks( UINT32 uSec, UINT16 Timer )
{
	UINT32 timerFrequency = TIM_CLK_HZ;
	UINT8 i;

	for (i=0; i<g_CountOfHardwareTimers; i++){
		if (Timer == g_HardwareTimerIDs[i]){
			timerFrequency = g_HardwareTimerFrequency[i];
		}
	}

		return ( uSec * (timerFrequency / CLOCK_COMMON_FACTOR) );
}

//--//

UINT32 CPU_MicrosecondsToSystemClocks( UINT32 uSec )
{
    uSec *= (TIM_CLK_HZ/CLOCK_COMMON_FACTOR);
    uSec /= (ONE_MHZ        /CLOCK_COMMON_FACTOR);

    return uSec;
}

int CPU_MicrosecondsToSystemClocks( int uSec )
{
    uSec *= (TIM_CLK_HZ/CLOCK_COMMON_FACTOR);
    uSec /= (ONE_MHZ        /CLOCK_COMMON_FACTOR);

    return uSec;
}

//--//

/*int CPU_SystemClocksToMicroseconds( int Ticks )
{
    Ticks *= (ONE_MHZ        /CLOCK_COMMON_FACTOR);
    Ticks /= (TIM_CLK_HZ/CLOCK_COMMON_FACTOR);

    return Ticks;
}*/

// This isn't cleanest but whole time system needs help
UINT64 CPU_TicksToMicroseconds( UINT64 ticks, UINT16 Timer )
{
	UINT64 timerFrequency = TIM_CLK_HZ; // DEFAULT_TIMER value
	UINT8 i;

	// Preferred if timerFrequency is multiple of CLOCK_COMMON_FACTOR
	if (Timer == DEFAULT_TIMER) {
		UINT64 ret;
		ret = ticks/(timerFrequency/CLOCK_COMMON_FACTOR); // e.g. ticks/30
		return ret;
	}

	for (i=0; i<g_CountOfHardwareTimers; i++){
		if (Timer == g_HardwareTimerIDs[i]){
			timerFrequency = g_HardwareTimerFrequency[i];
			break;
		}
	}

#ifdef _DEBUG
	// To avoid overflow we need to assume that CLOCK_COMMON_FACTOR divides timerFrequency
	if (timerFrequency % CLOCK_COMMON_FACTOR != 0)
		__BKPT();
#endif

	// Lets assume that all timers are reported in 1 MHz or at least fake it
	return ticks/(timerFrequency/CLOCK_COMMON_FACTOR);
	//return (UINT64)((ticks * CLOCK_COMMON_FACTOR) / timerFrequency);
}

UINT32 CPU_TicksToMicroseconds( UINT32 ticks, UINT16 Timer )
{
	UINT32 ret;
	UINT64 calc;
	UINT64 timerFrequency = TIM_CLK_HZ; // SYSTEM_TIME defaults to this
	UINT8 i;

	for (i=0; i<g_CountOfHardwareTimers; i++){
		if (Timer == g_HardwareTimerIDs[i]){
			timerFrequency = g_HardwareTimerFrequency[i];
			break;
		}
	}

	//ret = ((ticks * CLOCK_COMMON_FACTOR) / timerFrequency); // was overflowing trivally
	calc = ((UINT64)ticks * CLOCK_COMMON_FACTOR) / timerFrequency;
	if (calc > 0xFFFFFFFF) { ret = 0xFFFFFFFF; }
	else ret = calc;

	return ret;
}

UINT64 CPU_Timer_CurrentTicks(UINT16 Timer)
{
	static UINT64 prevRead = 0;
	UINT64 retVal = 0;
	if (Timer == LPTIM){
		// LPTIM time is in microseconds (although it is based off a 32kH physical clock)
		return requestLptimCounter();
	}
	else {
		GLOBAL_LOCK(irq);
		// ADVTIMER_32BIT
		// SYSTEM_TIME
		UINT64 current_systemTime =  m_systemTime;
		UINT64 currentValue = (UINT64)(TimHandle2_SystemTime.Instance->CNT);
		
		current_systemTime &= (0xFFFFFFFF00000000ull);
		current_systemTime |= currentValue;
		if (current_systemTime < prevRead){
			// lower 32-bit timer wrapped, so we'll bump our return value's bit 32nd bit (the real system time will do this in the interrupt routine after we exit)
  			current_systemTime += (0x1ull <<32);
		}
		retVal = current_systemTime;
		prevRead = retVal;
	}
	
	return retVal;
}

// This function is tuned for 8MHz of the emote in Release mode
void CPU_Timer_Sleep_MicroSeconds( UINT32 uSec, UINT16 Timer)
{
	if(Timer == ADVTIMER_32BIT)
	{
		if(uSec <= 1)
		{
			return;
		}

		GLOBAL_LOCK(irq);

		if(uSec <= 5)
		{
			UINT32 limit = (uSec << 1);
			for(volatile UINT32 i = 0; i < limit; i++);
			return;
		}

		// Adjustment for 5us of processing overhead
		uSec = uSec - 5;

		UINT64 currentCounterVal = CPU_Timer_CurrentTicks(SYSTEM_TIME);
		UINT64 ticks = CPU_MicrosecondsToTicks(uSec, Timer);
		while(CPU_Timer_CurrentTicks(SYSTEM_TIME) - currentCounterVal <= ticks);
	}
}


// CompareValue is based on the *System time* regardless of what clock is used.
// The difference between the compareValue and the current time must then be translated into the appropriate compare value for the clock that will be used to generate the interrupt
BOOL CPU_Timer_SetCompare(UINT16 Timer, UINT64 compareValue)
{
#ifdef _DEBUG
	static volatile unsigned sets_lptim1 = 0;
	static volatile unsigned sets_tim5 = 0;
	if (Timer == ADVTIMER_32BIT) sets_tim5++;
	else if (Timer == LPTIM) sets_lptim1++;
#endif
	UINT64 now = CPU_Timer_CurrentTicks(SYSTEM_TIME);

	if (Timer == ADVTIMER_32BIT){
		// making sure we have enough time before the timer fires to exit SetCompare, the VT callback and the timer interrupt
		if (compareValue < (now + TIMER_5_TIME_CUSHION)){
			compareValue = (now + TIMER_5_TIME_CUSHION);
		}  
		
		UINT64 totalCompareTime = compareValue - now;
		if ( totalCompareTime > TIMER_5_MAX_ALLOWABLE_WAIT ){ 
			compareValue = TIMER_5_MAX_ALLOWABLE_WAIT; 
		} 

		//UINT64 timer5ticks = CPU_MicrosecondsToTicks(CPU_TicksToMicroseconds(totalCompareTime, SYSTEM_TIME), ADVTIMER_32BIT);
		//TimHandle5.Init.Period = timer5ticks;

		// The above looks dumb and causes translation errors
		// I get the point, SYSTEM_TIME and ADVTIMER_32BIT might be different... but they aren't

		__HAL_TIM_DISABLE(&TimHandle5);
		TimHandle5.Instance->ARR = totalCompareTime;
		TimHandle5.Instance->EGR = TIM_EGR_UG;

		if (HAL_TIM_Base_Start_IT(&TimHandle5) != HAL_OK){
			hal_printf("error start\r\n");
		}

	} 
	else if (Timer == LPTIM)
	{
		// LPTIM time is in microseconds already (although it is based off a 32kH physical clock)
		volatile UINT64 nowLPTIM = CPU_Timer_CurrentTicks(LPTIM);

		UINT64 minLptimTimeout = MIN_LPTIM_COUNT;
		UINT64 totalCompareTime;
		if (compareValue < (nowLPTIM + minLptimTimeout)) {
			totalCompareTime = MIN_LPTIM_COUNT;
		} else {
			totalCompareTime = compareValue - nowLPTIM;								
		}
		callLptimSetCompareMicroseconds(totalCompareTime);
	}
}

//--//

UINT64 HAL_Time_CurrentTicks()
{
	return CPU_Timer_CurrentTicks(SYSTEM_TIME);    
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  /*##-1- Enable peripheral clock #################################*/
  if (htim->Instance == TIM2) {
	  /* TIMx Peripheral clock enable */
	  __HAL_RCC_TIM2_CLK_ENABLE();
	  
	  /*##-2- Configure the NVIC for TIMx ########################################*/
	  /* Set the TIMx priority */
	  
	 // __NVIC_SetVector(TIM2_IRQn, (uint32_t)TIM2_IRQHandler);
	  
	  HAL_NVIC_SetPriority(TIM2_IRQn, 3, 0);

	  /* Enable the TIMx global Interrupt */
	  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  }
  else if (htim->Instance == TIM5) {
	  /* TIMx Peripheral clock enable */
	  __HAL_RCC_TIM5_CLK_ENABLE();
	  
	  /*##-2- Configure the NVIC for TIMx ########################################*/
	  /* Set the TIMx priority */
	  
	  //__NVIC_SetVector(TIM5_IRQn, (uint32_t)TIM5_IRQHandler);
	  
	  HAL_NVIC_SetPriority(TIM5_IRQn, 3, 0);

	  /* Enable the TIMx global Interrupt */
	  HAL_NVIC_EnableIRQ(TIM5_IRQn);
  }
  
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2) {
		HAL_NVIC_DisableIRQ(TIM2_IRQn);
	}
	else if (htim->Instance == TIM5) {
		HAL_NVIC_DisableIRQ(TIM5_IRQn);
	}
}


// Set TIM2 instance keeps track of system time
// TIM2 should constantly run in the background
// TODO: update timer based on how much time we have slept when RTC timer wakes us up
// TIM2_IRQHandler increments the variable that tracks the upper 32-bits of the 64-bit system time
BOOL CPU_Timer_Initialize_System_time(){
	TimHandle2_SystemTime.Instance = TIM2;

	TimHandle2_SystemTime.Init.Period            = 0xFFFFFFFF;
	TimHandle2_SystemTime.Init.Prescaler         = 0; // This has a built in +1
	TimHandle2_SystemTime.Init.ClockDivision     = 1;
	TimHandle2_SystemTime.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle2_SystemTime.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&TimHandle2_SystemTime) != HAL_OK)
	{
		// Initialization Error 
		Error_Handler();
	}

	//##-2- Start the TIM Base generation in interrupt mode ####################
	__HAL_TIM_CLEAR_FLAG(&TimHandle2_SystemTime, TIM_FLAG_UPDATE); // Fires immediately otherwise
	if (HAL_TIM_Base_Start_IT(&TimHandle2_SystemTime) != HAL_OK)
	{
		// Starting Error 
		Error_Handler();
	}

	return TRUE;
}

BOOL CPU_Timer_Initialize(UINT16 Timer, BOOL IsOneShot, UINT32 Prescaler, HAL_CALLBACK_FPN ISR)
{
	// Dont allow initializing of timers with NULL as the callback function
	if(ISR == NULL)
		return FALSE;

	if(Timer == ADVTIMER_32BIT )
	{
		tim5CallBackISR = ISR;
		//hal_printf("init adv 32bit\r\n"); // serial console is not up yet
		// Set TIM2 instance keeps track of system time
		TimHandle5.Instance = TIM5;

		TimHandle5.Init.Period            = 10000000;
		TimHandle5.Init.Prescaler         = 0;	// This has a built in +1
		TimHandle5.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
		TimHandle5.Init.CounterMode       = TIM_COUNTERMODE_UP;
		TimHandle5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

		if (HAL_TIM_Base_Init(&TimHandle5) != HAL_OK)
		{
			// Starting Error 
			Error_Handler();
		}
		__HAL_TIM_URS_ENABLE(&TimHandle5);
		__HAL_TIM_CLEAR_FLAG(&TimHandle5, TIM_FLAG_UPDATE);
	}
	else if (Timer == LPTIM)
	{
		lptimCallBackISR = ISR;
		lptimCallBackISR_Param = LPTIM;

		// this will handle firing the LPTIM callback if timeout is too short
		LPTIM_interrupt_continuation.InitializeCallback(lptimSetCompareTriggered, NULL);

		LptimInit();
	}
	return TRUE;
}

BOOL CPU_Timer_UnInitialize(UINT16 Timer)
{
	if(Timer == ADVTIMER_32BIT )
	{
		if (HAL_TIM_Base_Stop_IT(&TimHandle5) != HAL_OK)
		{
			/* Starting Error */
			Error_Handler();
		}
	
		if (HAL_TIM_Base_DeInit(&TimHandle5) != HAL_OK)
		{
			/* Initialization Error */
			Error_Handler();
		}  
		if (HAL_TIM_Base_Stop_IT(&TimHandle2_SystemTime) != HAL_OK)
		{
			/* Starting Error */
			Error_Handler();
		}
	
		if (HAL_TIM_Base_DeInit(&TimHandle2_SystemTime) != HAL_OK)
		{
			/* Initialization Error */
			Error_Handler();
		} 
	} /*else if(Timer == RTC_32BIT )
	{
	
	}*/
    //CPU_INTC_DeactivateInterrupt(TIM_32_IRQn);
    
    //TIM_16->CR1 &= ~TIM_CR1_CEN; // disable timers
    //TIM_32->CR1 &= ~TIM_CR1_CEN;
    
    // disable timer clocks
//#ifdef RCC_APB1ENR_TIM_16_EN
//    RCC->APB1LENR &= ~(RCC_APB1ENR_TIM_32_EN | RCC_APB1ENR_TIM_16_EN);
//#else
//    RCC->APB1LENR &= ~RCC_APB1ENR_TIM_32_EN;
//    RCC->APB2ENR &= ~RCC_APB2ENR_TIM_16_EN;
//#endif
    
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

    uSec *= (TIM_CLK_HZ / CLOCK_COMMON_FACTOR);
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

void CPU_GetDriftParameters  ( INT32* a, INT32* b, INT64* c )
{
    *a = 0;
    *b = 1;
    *c = 0;
}

// timeToAdd is in 100-nanosecond (ns) increments. This is a Microsoft thing.
void CPU_AddClockTime(UINT16 Timer, UINT64 timeToAdd)
{
	UINT32 timerFrequency = TIM_CLK_HZ;
	UINT64 ticksToAdd = 0;
	UINT8 i;

	for (i=0; i<g_CountOfHardwareTimers; i++){
		if (Timer == g_HardwareTimerIDs[i]){
			timerFrequency = g_HardwareTimerFrequency[i];
		}
	}

	if(Timer == ADVTIMER_32BIT)
	{
		if(timeToAdd == 0)
		{
			return;
		}

		// ticksToAdd = timeToAdd * (0.0000001 s) * 8000000 ticks/s (@ 8MHz)
		ticksToAdd = timeToAdd * 0.0000001 * timerFrequency;
		//g_STM32F10x_AdvancedTimer.AddTicks(ticksToAdd);
	}
	else
	{
		ASSERT(0);
	}
}
