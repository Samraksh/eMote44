#include <stdbool.h>
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"
#include "lptim.h"

//#define LPTIM_DEBUG_VERBOSE
#define DELTA_TICKS_GUARD 2 // <= this amt is "now"
#define DELTA_TICKS_EXTRA 0  // Manually compensate this many ticks on set compare

#define LSE_HZ 32768

#ifndef ALLOW_BKPT
#undef __BKPT
#define __BKPT() ((void)0)
#endif

static LPTIM_HandleTypeDef hlptim1;
static LPTIM_HandleTypeDef hlptim2;

// Set the LPTIM used for the clocking in this file.
// Anticipate giving LPTIM1 to VT and its code doing it own thing other than init for now.
static LPTIM_HandleTypeDef *my_lptim; // Will init to LPTIM2
static LPTIM_HandleTypeDef *vt_lptim; // Will init to LPTIM1

static volatile bool cmp_set;
static volatile bool vt_fire_on_roll; // for longer timers, fire every roll until correct epoch

// Upper 32-bits of an effective 48-bit counter (when added with LPTIM)
static volatile uint32_t lse_counter32;
static volatile uint32_t lse_counter32_vt;

// Used to check timer monotonicity and return in last resort case where we cannot determine time
// Hopefully never happens
static volatile uint64_t lse_counter64_prev;
static volatile uint64_t lse_counter64_vt_prev;

// Locks
static uint32_t lptim_lock;
static uint32_t lptim_lock_vt;

typedef enum {
	lptim_lock_none	=0,
	lptim_lock_cmp	=1,
} lptim_lock_id_t;

static void LPTIM_Error_Handler(void) {

}

static inline void wait_64_nop(void) {
	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();

	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();

	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();

	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP();
}

// Test if in interrupt context
static inline bool isInterrupt() {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

// Returns true if lock aquired
static bool get_lock_inner(volatile uint32_t *Lock_Variable, lptim_lock_id_t id) {
	uint32_t status;
	if (__LDREXW(Lock_Variable) != lptim_lock_none) {
		__CLREX(); // AM I NEEDED?
		return false;
	}
	status = __STREXW(id, Lock_Variable);
	return (status == 0);
}

// Should try more than once, can legit fail even if lock is free.
// Example, will never succeed if any interrupt hits in between.
// ldrex-strex only guarantees that lock is free when it says so, but NOT the inverse.
static uint32_t get_lock(volatile uint32_t *Lock_Variable, lptim_lock_id_t id) {
	int attempts=3;
	do {
		if ( get_lock_inner(Lock_Variable, id) )
		{ __DMB(); return 0; }
	} while (--attempts);
	return *Lock_Variable; // return who we think the blocking owner is. NOT GUARANTEED TO BE RIGHT.
}

static void free_lock(volatile uint32_t *Lock_Variable) {
	__DMB();
	*Lock_Variable = lptim_lock_none;
	return;
}

uint64_t lptim_get_counter_us_fast(void) {
	// uint64_t read = HAL_LPTIM_ReadCounter(my_lptim);
	// return (lse_counter32 << 16);
	uint64_t read = lse_counter32;
	read = (read << 16) + HAL_LPTIM_ReadCounter(my_lptim);
	return read * 1000000 / LSE_HZ;
}

// Returns LSE native tick count (32-bit)
static uint64_t my_get_counter_lptim(volatile uint32_t *counter32, LPTIM_HandleTypeDef *lptim_ptr) {
	uint64_t ret = 0;
	uint32_t read, prim, counter_temp;
	unsigned timeout = 3;

	while(timeout) {
		prim = __get_PRIMASK(); __disable_irq();

		// If in ISR or interrupts were disabled in caller, use heroic effort
		// Check manually if ARRM interrupt is pending, and service it if necessary
		// Best to avoid this case if at all possible
		if ( isInterrupt() || prim ) {
			if (__HAL_LPTIM_GET_FLAG(lptim_ptr, LPTIM_FLAG_ARRM) != RESET && __HAL_LPTIM_GET_IT_SOURCE(lptim_ptr, LPTIM_IT_ARRM) != RESET) {
				__HAL_LPTIM_CLEAR_FLAG(lptim_ptr, LPTIM_FLAG_ARRM);
				HAL_LPTIM_AutoReloadMatchCallback(lptim_ptr);
			}
		}

		read = HAL_LPTIM_ReadCounter(lptim_ptr);
		counter_temp = *counter32; // Save it to check again later

		if (!prim) __enable_irq();

		__NOP(); // Not itself important, but explictly allow for interrupt to fire, if it can

		// Check ARRM again if needed
		if ( isInterrupt() || prim ) {
			__disable_irq(); // By assumption, should be redundant (no interrupt pre-emption), but whatever...
			if (__HAL_LPTIM_GET_FLAG(lptim_ptr, LPTIM_FLAG_ARRM) != RESET && __HAL_LPTIM_GET_IT_SOURCE(lptim_ptr, LPTIM_IT_ARRM) != RESET) {
				__HAL_LPTIM_CLEAR_FLAG(lptim_ptr, LPTIM_FLAG_ARRM);
				HAL_LPTIM_AutoReloadMatchCallback(lptim_ptr);
			}
			if (!prim) __enable_irq();
		}

		// If counter32 incremented (timer fired), assume we are inconsistent
		if (counter_temp == *counter32) {
			ret = (((uint64_t)(counter_temp)) << 16) + read;
			break;
		}
		else timeout--;
	}
	if (timeout == 0) __BKPT();

	return ret;
}

// Returns tick count in 1 us ticks
// Do a special check to guarantee monotonic time
// In particular, if my_get_counter_lptim() cannot get a consistent reading it will return 0
// In this case the best we can do is return the last known good time.
// This should be very very rare, maybe never in practice.
uint64_t lptim_get_counter_us(int lptim) {
	uint64_t ret;
	if (lptim == LPTIM_VT) {
		ret = my_get_counter_lptim(&lse_counter32_vt, vt_lptim);
		if (ret >= lse_counter64_vt_prev) lse_counter64_vt_prev = ret;
		else ret = lse_counter64_vt_prev;
	}
	else {
		ret = my_get_counter_lptim(&lse_counter32, my_lptim);
		if (ret >= lse_counter64_prev)    lse_counter64_prev = ret;
		else ret = lse_counter64_prev;
	}
	return ret * 1000000 / LSE_HZ;
}

// Compare match sub-handler called from HAL_LPTIM_IRQHandler()
void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim) {
	if( hlptim->Instance == my_lptim->Instance ) { // LPTIM2 for Debug and Task
		__HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);
		if (!cmp_set) return; // False fire
		cmp_set = false;
		lptim_task_cb();
	} else {
		__HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);
		lptimIRQHandler(); // The VT
	}
}

// Overflow sub-handler called from HAL_LPTIM_IRQHandler()
// LPTIM counter is 16-bit
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim) {
	if( hlptim->Instance == my_lptim->Instance )
		lse_counter32 += 1;
	else
		lse_counter32_vt += 1;
	__DMB();

	if (vt_fire_on_roll && hlptim->Instance == vt_lptim->Instance) {
		vt_fire_on_roll = false;
		lptimIRQHandler();
	}
}

// Main LPTIM1 IRQ handler
void LPTIM1_IRQHandler(void) {
  HAL_LPTIM_IRQHandler(&hlptim1);
}

// Main LPTIM2 IRQ handler
void LPTIM2_IRQHandler(void) {
  HAL_LPTIM_IRQHandler(&hlptim2);
}

int lptim_set_vt_fire_on_roll(void) {
	vt_fire_on_roll = true;
	return 0;
}

// For debug LPTIM2
static int my_start_lptim(LPTIM_HandleTypeDef *lptim) {
	HAL_StatusTypeDef ret;
	free_lock(&lptim_lock);
	ret = HAL_LPTIM_Counter_Start_IT(lptim, 0xFFFF);
	if (ret != HAL_OK) __BKPT();
	cmp_set = false;
	__HAL_LPTIM_COMPARE_SET(lptim, 0); // Only to setup CMPOK
	return 0;
}

// Starts LPTIM1 which is used by the VT
static int my_start_lptim_vt(LPTIM_HandleTypeDef *lptim) {
	HAL_StatusTypeDef ret;
	free_lock(&lptim_lock_vt);
	ret = HAL_LPTIM_Counter_Start_IT(lptim, 0xFFFF);
	if (ret != HAL_OK) __BKPT();
	__HAL_LPTIM_COMPARE_SET(lptim, 0); // Only to setup CMPOK
	return 0;
}

static uint32_t lptim_ms_to_ticks(uint32_t ms) {
	uint64_t ret = ms*(uint64_t)LSE_HZ/1000;
	if (ret >= 0xFFFFFFFF) return 0xFFFFFFFF;
	else return ret;
}

static uint32_t lptim_us_to_ticks(uint32_t us) {
	uint64_t ret = (uint64_t)us*(uint64_t)LSE_HZ/1000000;
	if (ret >= 0xFFFFFFFF) return 0xFFFFFFFF;
	else return ret;
}

// Sets a compare interrupt for 'dticks' (delta ticks) in the future
// Strictly speaking, compare is for "at least dticks" but attempt tight lower bound
// WARNING: Will override any existing compare
int lptim_set_compare_dticks(uint16_t dticks, int lptim) {
	uint32_t prim, lock_ret, now, now2, diff;
	int ret = lptim_err_none;

	uint32_t *lock;
	LPTIM_HandleTypeDef *lptim_ptr;

	switch (lptim) {
		case LPTIM_VT:  lock = &lptim_lock_vt; lptim_ptr = vt_lptim; break;
		case LPTIM_DEBUG:  lock = &lptim_lock; lptim_ptr = my_lptim; break;
		default: __BKPT(); lock = &lptim_lock; lptim_ptr = my_lptim;
	}

	now = HAL_LPTIM_ReadCounter(lptim_ptr); // Sample time at start

	if (dticks <= DELTA_TICKS_GUARD) return lptim_err_short; // Too short

	// take the lock
	lock_ret = get_lock(lock, lptim_lock_cmp);
	if (lock_ret) return lptim_err_busy; // busy, failed to take lock

	// Kill existing interrupt
	if (cmp_set && lptim == LPTIM_DEBUG) { cmp_set = false; __HAL_LPTIM_DISABLE_IT(lptim_ptr, LPTIM_IT_CMPM); }

	while ( __HAL_LPTIM_GET_FLAG(lptim_ptr, LPTIM_FLAG_CMPOK) == RESET ) ; // spin for CMP register to be ready
	prim = __get_PRIMASK(); __disable_irq(); // Disable IRQ for tight timing

	 // Calculate the ticks we lost to setup, usually 0
	now2 = HAL_LPTIM_ReadCounter(lptim_ptr);
	if (now2 >= now) diff = now2 - now;
	else diff = now2 - now + 0x10000; // Rolled
	if (dticks - diff <= DELTA_TICKS_GUARD) {
		ret = lptim_err_short;
		goto out;
	}

	// Set the new compare
	__HAL_LPTIM_ENABLE_IT(lptim_ptr, LPTIM_IT_CMPM);
	__HAL_LPTIM_COMPARE_SET(lptim_ptr, now+dticks+DELTA_TICKS_EXTRA);
	if (lptim == LPTIM_DEBUG) cmp_set = true;

out:
	if (!prim) __enable_irq(); // Only enable if they were not disabled at the start
	free_lock(lock);
	return ret;
}

// Sets a compare interrupt for 'ticks' value in the current or next epoch
// WARNING: Will override any existing compare
int lptim_set_compare_ticks(uint16_t ticks, bool is_next_epoch, int lptim) {
	uint32_t prim, lock_ret, now;
	int ret = lptim_err_none;

	uint32_t *lock;
	LPTIM_HandleTypeDef *lptim_ptr;

	switch (lptim) {
		case LPTIM_VT:  lock = &lptim_lock_vt; lptim_ptr = vt_lptim; break;
		case LPTIM_DEBUG:  lock = &lptim_lock; lptim_ptr = my_lptim; break;
		default: __BKPT(); lock = &lptim_lock; lptim_ptr = my_lptim;
	}

	if (ticks > 0xFFFF) ticks = 0xFFFF;

	// take the lock
	lock_ret = get_lock(lock, lptim_lock_cmp);
	if (lock_ret) return lptim_err_busy; // busy, failed to take lock

	// Kill existing interrupt
	if (cmp_set && lptim == LPTIM_DEBUG) { cmp_set = false; __HAL_LPTIM_DISABLE_IT(lptim_ptr, LPTIM_IT_CMPM); }

	while ( __HAL_LPTIM_GET_FLAG(lptim_ptr, LPTIM_FLAG_CMPOK) == RESET ) ; // spin for CMP register to be ready
	prim = __get_PRIMASK(); __disable_irq(); // Disable IRQ to ensure timing remains consistent

	now = HAL_LPTIM_ReadCounter(lptim_ptr) + DELTA_TICKS_EXTRA;
	// Make sure epoch and ticks are consistent
	if (now > ticks && !is_next_epoch) ret = lptim_err_short;
	if (now < ticks &&  is_next_epoch) ret = lptim_err_long;
	if (ret) goto out;

	// Set the new compare
	__HAL_LPTIM_ENABLE_IT(lptim_ptr, LPTIM_IT_CMPM);
	__HAL_LPTIM_COMPARE_SET(lptim_ptr, ticks);
	if (lptim == LPTIM_DEBUG) cmp_set = true;

out:
	if (!prim) __enable_irq(); // Only enable if they were not disabled at the start
	free_lock(lock);
	return ret;
}

int lptim_set_delay_ticks(uint16_t ticks, int lptim) {
	return lptim_set_compare_dticks(ticks, lptim);
}

int lptim_set_delay_ms(uint32_t ms, int lptim) {
	uint32_t ticks = lptim_ms_to_ticks(ms);
	if (ticks > 0xFFFF) return lptim_err_long;
	return lptim_set_compare_dticks(ticks, lptim);
}

int lptim_set_delay_us(uint32_t us, int lptim) {
	uint32_t ticks = lptim_us_to_ticks(us);
	if (ticks > 0xFFFF) return lptim_err_long;
	return lptim_set_compare_dticks(ticks, lptim);
}

// Milli-seconds. Overrides _weak HAL_Delay in ST driver
void HAL_Delay(uint32_t Delay) {

	// Original ST HAL code (mostly)
	if (my_lptim == NULL) {
		if ( isInterrupt() || __get_PRIMASK() ) { __BKPT(); return; } // Will be blocked, abort.
		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = Delay;

		/* Add a freq to guarantee minimum wait */
		if (wait < HAL_MAX_DELAY) { wait += (uint32_t)(HAL_GetTickFreq()); }

		while ((HAL_GetTick() - tickstart) < wait) { }
		return;
	}

	uint64_t target = lptim_get_counter_us(LPTIM_DEBUG) + Delay*1000;
	uint64_t now;
	do {
		wait_64_nop();
		now = lptim_get_counter_us(LPTIM_DEBUG);
	} while (now < target) ;
}

/* LPTIM 1+2 init function */
void MX_LPTIM_Init(void) {
	lse_counter32 = 0;
	lse_counter32_vt = 0;

	// LPTIM1
	hlptim1.Instance = LPTIM1;
	hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
	hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
	hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
	hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
	hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
	hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
	hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
	if (HAL_LPTIM_Init(&hlptim1) != HAL_OK) LPTIM_Error_Handler();
	//__HAL_RCC_LPTIM1_CLK_SLEEP_ENABLE(); // Might be needed in future

	// LPTIM2
	hlptim2.Instance = LPTIM2;
	hlptim2.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	hlptim2.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
	hlptim2.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
	hlptim2.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
	hlptim2.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
	hlptim2.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
	hlptim2.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
	hlptim2.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
	if (HAL_LPTIM_Init(&hlptim2) != HAL_OK) LPTIM_Error_Handler();
	//__HAL_RCC_LPTIM2_CLK_SLEEP_ENABLE(); // Might be needed in future

	//HAL_PWREx_ConfigD3Domain(PWR_D3_DOMAIN_RUN); // Might be needed in future

	vt_lptim = &hlptim1;
	my_lptim = &hlptim2;

	my_start_lptim_vt(vt_lptim);
	my_start_lptim(my_lptim);

	// HAL_Delay is now driven from LPTIM2 so we can turn off SysTick
	HAL_SuspendTick();
}

// Called from HAL_LPTIM_Init()
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* lptimHandle)
{

  if(lptimHandle->Instance==LPTIM1)
  {
    __HAL_RCC_LPTIM1_CLK_ENABLE();
    HAL_NVIC_SetPriority(LPTIM1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
  }
  else if(lptimHandle->Instance==LPTIM2)
  {
    __HAL_RCC_LPTIM2_CLK_ENABLE();
    HAL_NVIC_SetPriority(LPTIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LPTIM2_IRQn);
  }
}

void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* lptimHandle)
{
  if(lptimHandle->Instance==LPTIM1)
  {
    __HAL_RCC_LPTIM1_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
  }
  else if(lptimHandle->Instance==LPTIM2)
  {
    __HAL_RCC_LPTIM2_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(LPTIM2_IRQn);
  }
}
