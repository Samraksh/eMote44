/**
  ******************************************************************************
  * File Name          : LPTIM.c
  * Description        : This file provides code for the configuration
  *                      of the LPTIM instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lptim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

static LPTIM_HandleTypeDef hlptim1;
static volatile uint32_t rtc_counter32; // At 32.768 kHz, rolls at ~36.4 hours

// Returns LSE native tick count (32-bit)
uint32_t my_get_counter_lptim(void) {
	uint32_t ret, read, read2;
	unsigned timeout = 10;

	while(timeout) {
		__disable_irq();
		__DSB(); // paranoid
		read = HAL_LPTIM_ReadCounter(&hlptim1);
		ret = rtc_counter32 + read;
		__enable_irq();

		read2 = HAL_LPTIM_ReadCounter(&hlptim1);
		if (read <= read2) break;
		else timeout--; // read2 < read case, inconsistent, do it again
	}
	if (timeout == 0) __BKPT();

	return ret;
}

// Returns tick count in 1 us ticks
uint64_t my_get_counter_lptim_us(void) {
	return (uint64_t)my_get_counter_lptim() * 1000000 / 32768;
}

// Overflow sub-handler called from HAL_LPTIM_IRQHandler()
// LPTIM counter is 16-bit
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim) {
	rtc_counter32 += 0x10000;
}

// Main LPTIM1 IRQ handler
void LPTIM1_IRQHandler(void) {
  HAL_LPTIM_IRQHandler(&hlptim1);
}

static int my_start_lptim(void) {
	HAL_StatusTypeDef ret;
	ret = HAL_LPTIM_Counter_Start_IT(&hlptim1, 0xFFFF);
	if (ret != HAL_OK) __BKPT();
	return 0;
}

/* LPTIM1 init function */
void MX_LPTIM1_Init(void)
{
  rtc_counter32 = 0;
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    __BKPT(); // TODO: HANDLE ERROR
  }
  my_start_lptim();

}

// Called from HAL_LPTIM_Init()
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* lptimHandle)
{

  if(lptimHandle->Instance==LPTIM1)
  {
  /* USER CODE BEGIN LPTIM1_MspInit 0 */

  /* USER CODE END LPTIM1_MspInit 0 */
    /* LPTIM1 clock enable */
    __HAL_RCC_LPTIM1_CLK_ENABLE();

    /* LPTIM1 interrupt Init */
    HAL_NVIC_SetPriority(LPTIM1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
  /* USER CODE BEGIN LPTIM1_MspInit 1 */

  /* USER CODE END LPTIM1_MspInit 1 */
  }
}

void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* lptimHandle)
{

  if(lptimHandle->Instance==LPTIM1)
  {
  /* USER CODE BEGIN LPTIM1_MspDeInit 0 */

  /* USER CODE END LPTIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPTIM1_CLK_DISABLE();

    /* LPTIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
  /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

  /* USER CODE END LPTIM1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
