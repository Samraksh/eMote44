#include <stdbool.h>
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"
#include "lptim.h"

#define LPTIM_DEBUG
#define DELTA_TICKS_GUARD 2 // <= this amt is "now"
#define DELTA_TICKS_EXTRA 0  // Manually compensate this many ticks on set compare

static LPTIM_HandleTypeDef hlptim1;
static LPTIM_HandleTypeDef hlptim2;

// Set the LPTIM used for the clocking in this file.
// Anticipate giving LPTIM1 to VT and its code doing it own thing other than init for now.
static LPTIM_HandleTypeDef *my_lptim; // Will init to LPTIM2
static LPTIM_HandleTypeDef *vt_lptim; // Will init to LPTIM1

static volatile bool cmp_set;
static volatile uint32_t lse_counter32;		// At 32.768 kHz, rolls at ~36.4 hours
static volatile uint32_t lse_counter32_vt;	// At 32.768 kHz, rolls at ~36.4 hours
static uint32_t lptim_lock;
static uint32_t lptim_lock_vt;

typedef enum {
	lptim_lock_none	=0,
	lptim_lock_cmp	=1,
} lptim_lock_id_t;

static void LPTIM_Error_Handler(void) {
#ifdef LPTIM_DEBUG
		__BKPT();
#endif
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


// Returns LSE native tick count (32-bit)
static uint32_t my_get_counter_lptim(void) {
	uint32_t ret, read, read2, prim;
	unsigned timeout = 10;

	while(timeout) {
		prim = __get_PRIMASK(); __disable_irq();
		__DMB();
		read = HAL_LPTIM_ReadCounter(my_lptim);
		ret = lse_counter32 + read;
		if (!prim) __enable_irq();

		read2 = HAL_LPTIM_ReadCounter(my_lptim);
		if (read <= read2) break;
		else timeout--; // read2 < read case, inconsistent, do it again
	}
	if (timeout == 0) __BKPT();

	return ret;
}

static uint32_t my_get_counter_lptim_vt(void) {
	uint32_t ret, read, read2, prim;
	unsigned timeout = 10;

	while(timeout) {
		prim = __get_PRIMASK(); __disable_irq();
		__DMB();
		read = HAL_LPTIM_ReadCounter(vt_lptim);
		ret = lse_counter32_vt + read;
		if (!prim) __enable_irq();

		read2 = HAL_LPTIM_ReadCounter(vt_lptim);
		if (read <= read2) break;
		else timeout--; // read2 < read case, inconsistent, do it again
	}
	if (timeout == 0) __BKPT();

	return ret;
}

// Returns tick count in 1 us ticks
uint64_t my_get_counter_lptim_us(int lptim) {
	if (lptim == lptim_vt)
		return (uint64_t)my_get_counter_lptim_vt() * 1000000 / 32768;
	else
		return (uint64_t)my_get_counter_lptim()    * 1000000 / 32768;
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
		lse_counter32 += 0x10000;
	else
		lse_counter32_vt += 0x10000;
}

// Main LPTIM1 IRQ handler
void LPTIM1_IRQHandler(void) {
  HAL_LPTIM_IRQHandler(&hlptim1);
}

// Main LPTIM2 IRQ handler
void LPTIM2_IRQHandler(void) {
  HAL_LPTIM_IRQHandler(&hlptim2);
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
	if (ms >= 131072) return 0xFFFFFFFF; // saturate
	else return ms*32768/1000;
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
		case lptim_vt:  lock = &lptim_lock_vt; lptim_ptr = vt_lptim; break;
		case lptim_debug:  lock = &lptim_lock; lptim_ptr = my_lptim; break;
		default: __BKPT(); lock = &lptim_lock; lptim_ptr = my_lptim;
	}

	now = HAL_LPTIM_ReadCounter(lptim_ptr); // Sample time at start

	if (dticks <= DELTA_TICKS_GUARD) return lptim_err_short; // Too short

	// take the lock
	lock_ret = get_lock(lock, lptim_lock_cmp);
	if (lock_ret) return lptim_err_busy; // busy, failed to take lock

	// Kill existing interrupt
	if (cmp_set && lptim == lptim_debug) { cmp_set = false; __HAL_LPTIM_DISABLE_IT(lptim_ptr, LPTIM_IT_CMPM); }

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
	if (lptim == lptim_debug) cmp_set = true;

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
		case lptim_vt:  lock = &lptim_lock_vt; lptim_ptr = vt_lptim; break;
		case lptim_debug:  lock = &lptim_lock; lptim_ptr = my_lptim; break;
		default: __BKPT(); lock = &lptim_lock; lptim_ptr = my_lptim;
	}

	if (ticks > 0xFFFF) ticks = 0xFFFF;

	// take the lock
	lock_ret = get_lock(lock, lptim_lock_cmp);
	if (lock_ret) return lptim_err_busy; // busy, failed to take lock

	// Kill existing interrupt
	if (cmp_set && lptim == lptim_debug) { cmp_set = false; __HAL_LPTIM_DISABLE_IT(lptim_ptr, LPTIM_IT_CMPM); }

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
	if (lptim == lptim_debug) cmp_set = true;

out:
	if (!prim) __enable_irq(); // Only enable if they were not disabled at the start
	free_lock(lock);
	return ret;
}

int set_lptim_set_delay_ms(uint32_t ms, int lptim) {
	uint32_t ticks = lptim_ms_to_ticks(ms);
	return lptim_set_compare_dticks(ticks, lptim);
}

static uint32_t lptim_us_to_ticks(uint32_t us) {
	if (us >= 131072000) return 0xFFFFFFFF; // saturate
	else return us*32768/1000000;
}

int set_lptim_set_delay_us(uint32_t us, int lptim) {
	uint32_t ticks = lptim_us_to_ticks(us);
	return lptim_set_compare_dticks(ticks, lptim);
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
