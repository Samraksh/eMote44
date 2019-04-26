////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//  Implementation for STM32H7: Copyright (c) Oberon microsystems, Inc.
//
//  *** STM32H7 Bootstrap ***
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"

#ifndef FLASH
#define FLASH               ((FLASH_TypeDef *) FLASH_R_BASE)
#endif

#define __PLLM1__ 4
#define __PLLN1__ 400
#define __PLLP1__ 2
#define __PLLQ1__ 4
#define __PLLR1__ 2

///////////////////////////////////////////////////////////////////////////////

#pragma arm section code = "SectionForBootstrapOperations"

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  //SCB_EnableDCache();
  SCB_InvalidateDCache();
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  //while(1)
  //{
  //}
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE BYPASS)
  *            SYSCLK(Hz)                     = 400000000 (CPU Clock)
  *            HCLK(Hz)                       = 200000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 4
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /*!< Supply configuration update enable */
  MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);
 
	
  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
 
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    Error_Handler();
  } 
	
/* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                 RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  if(ret != HAL_OK)
  {
    Error_Handler();	
  }
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;//|RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  //PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  
  /*activate CSI clock mondatory for I/O Compensation Cell*/
  //__HAL_RCC_CSI_ENABLE() ;

  /* Enable SYSCFG clock mondatory for I/O Compensation Cell */
  __HAL_RCC_SYSCFG_CLK_ENABLE() ;

  /* Enables the I/O Compensation Cell */
  HAL_EnableCompensationCell();
}

/* IO initialization implemented in solution DeviceCode\Init */
void BootstrapCode_GPIO();


extern "C"
{

void __section("SectionForBootstrapOperations") STM32H7_BootstrapCode()
{
	// assure interupts off
    __disable_irq();
	
	SystemInit();

	CPU_CACHE_Enable();
	
	HAL_Init();
	
	SystemClock_Config();
	
	__NVIC_SetVector(SysTick_IRQn, (uint32_t)SysTick_Handler);
	
	SysTick_Config(SysTick_IRQn);
	
}
	
    //
    // L1 caches
    //
    // UNDONE: Move to CPU cache library, i.e. inside CPU_CACHE_Enable(). But, the API
    // currently does not support cache type parameter and is not called from anywhere.
    //
    // UNDONE: Optional configuration, e.g. #ifdef STM32H7_ENABLE_DCACHE or similar.
    //
    // UNDONE: Cache configuration based on flash memory interfaces (AN4667):
    // If the access to the Flash memory is done via AXI/AHB (address 0x08000000),
    // instruction and/or data caches should be enabled.
    // If the access to the Flash memory is done via ITCM bus (address 0x02000000),
    // the ART accelerator should be enabled (along with ART-Prefetch).
    //

    //SCB_EnableICache(); // Invalidate and enable L1 Instruction cache

    // UNDONE: FIXME: D-Cache temporarily disabled, as it causes problems with setting interrupt vectors (located in cache-able RAM)
    //SCB_EnableDCache(); // Invalidate and enable L1 Data cache


 

    // allow unaligned memory access (Cortex-M7 has STKALIGN read-only)
    //SCB->CCR &= ~SCB_CCR_UNALIGN_TRP_Msk;

	/*!< Supply configuration update enable */
	//MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

	/* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
	//MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, (PWR_D3CR_VOS_1 | PWR_D3CR_VOS_0));

	//while(!(PWR->D3CR & PWR_D3CR_VOSRDY));

	//RCC->CR |= RCC_CR_HSEON;
	//while(!(RCC->CR & RCC_CR_HSERDY));

	//MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSE);   
	//RCC->CR &= ~(RCC_CR_PLL1ON);
	
	//RCC->PLL1DIVR = ( (((__PLLN1__) - 1U )& RCC_PLL1DIVR_N1) | ((((__PLLP1__) -1U ) << 9U) & RCC_PLL1DIVR_P1) | \
                                ((((__PLLQ1__) -1U) << 16U)& RCC_PLL1DIVR_Q1) | ((((__PLLR1__) - 1U) << 24U)& RCC_PLL1DIVR_R1) );
	
	//MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE, RCC_PLLCFGR_PLL1RGE_2);

	//MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1VCOSEL, ((uint32_t)0x00000000));
	
	//SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVP1EN);
	
	//SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVQ1EN);
	
	//SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVR1EN);
	
	//SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLL1FRACEN);
	
	//SET_BIT(RCC->CR, RCC_CR_PLL1ON);
	
    //while(!(RCC->CR & RCC_CR_PLLRDY));

	//MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_4WS);
	
	// HCLK configuration 
	//MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_HPRE, RCC_D1CFGR_HPRE_DIV2);
	// SYSCLK Configuration
	//MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1CPRE, RCC_D1CFGR_D1CPRE_DIV1);
	//MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL1);   
	
	//MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1PPRE,  RCC_D1CFGR_D1PPRE_DIV2);
	//MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1, RCC_D2CFGR_D2PPRE1_DIV2);
	//MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2, RCC_D2CFGR_D2PPRE2_DIV2);
	//MODIFY_REG(RCC->D3CFGR, RCC_D2CFGR_D2PPRE1, RCC_D3CFGR_D3PPRE_DIV1);
	
	//RCC->AHB1ENR = 0;   
	//RCC->AHB2ENR = 0;
	//RCC->AHB3LPENR = (RCC_AHB3LPENR_DTCM1LPEN);
    //RCC->APB4ENR = RCC_APB4ENR_SYSCFGEN; // SYSCFG clock used for IO
	
	/*RCC->AHB3LPENR = (RCC_AHB3LPENR_DTCM1LPEN);
	
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_D2SRAM1EN);
	
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_D2SRAM2EN);
	
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_D2SRAM3EN);
	*/
	// Enables the I/O Compensation Cell
	//SET_BIT(RCC->CR, RCC_CR_CSION);
	
	// The I/O compensation cell can be used only when the device supply
	//SET_BIT(RCC->APB4ENR, RCC_APB4ENR_SYSCFGEN); // SYSCFG clock used for IO
	
	// voltage ranges from 2.4 to 3.6 V.
 	//SET_BIT(SYSCFG->CCCSR, SYSCFG_CCCSR_EN); 
		
	//SET_BIT(RCC->PLLCFGR, (RCC_PLLCFGR_DIVQ1EN));
	//MODIFY_REG(RCC->D2CCIP2R, RCC_D2CCIP2R_USBSEL, RCC_D2CCIP2R_USBSEL_0);
	//MODIFY_REG(RCC->D2CCIP2R, RCC_D2CCIP2R_USART28SEL, ((uint32_t)0x00000000));
	//SET_BIT(PWR->CR3, PWR_CR3_USB33DEN);


__section("SectionForBootstrapOperations") void BootstrapCode()
{
    STM32H7_BootstrapCode();

    PrepareImageRegions();

    //BootstrapCode_GPIO();
}

__section("SectionForBootstrapOperations") void BootstrapCodeMinimal()
{
    STM32H7_BootstrapCode();

    //BootstrapCode_GPIO();
}

}

