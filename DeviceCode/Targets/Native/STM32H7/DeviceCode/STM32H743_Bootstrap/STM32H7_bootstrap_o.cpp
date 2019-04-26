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

///////////////////////////////////////////////////////////////////////////////

//#define __PLLM1__ 1
//#define __PLLN1__ 24
//#define __PLLP1__ 2
//#define __PLLQ1__ 4
//#define __PLLR1__ 2

#define __PLLM1__ 4
#define __PLLN1__ 400
#define __PLLP1__ 2
#define __PLLQ1__ 4
#define __PLLR1__ 2

/* STM32H7 clock configuration */

/* #if SYSTEM_CRYSTAL_CLOCK_HZ % ONE_MHZ != 0
    #error SYSTEM_CRYSTAL_CLOCK_HZ must be a multiple of 1MHz
#endif
#if SYSTEM_CRYSTAL_CLOCK_HZ != 0
    #define RCC_PLLCFGR_PLLM_BITS (SYSTEM_CRYSTAL_CLOCK_HZ / ONE_MHZ * RCC_PLLCFGR_PLLM_0)
    #define RCC_PLLCFGR_PLLS_BITS (RCC_PLLCFGR_PLLSRC_HSE)
#else // 16MHz internal oscillator
    #define RCC_PLLCFGR_PLLM_BITS (16 * RCC_PLLCFGR_PLLM_0)
    #define RCC_PLLCFGR_PLLS_BITS (RCC_PLLCFGR_PLLSRC_HSI)
#endif

#if (SYSTEM_CLOCK_HZ * 2 >= 192000000)\
 && (SYSTEM_CLOCK_HZ * 2 <= 432000000)\
 && (SYSTEM_CLOCK_HZ * 2 % 48000000 == 0)
    #define RCC_PLLCFGR_PLLN_BITS (SYSTEM_CLOCK_HZ * 2 / ONE_MHZ * RCC_PLLCFGR_PLLN_0)
    #define RCC_PLLCFGR_PLLP_BITS (0)  // P = 2
    #define RCC_PLLCFGR_PLLQ_BITS (SYSTEM_CLOCK_HZ * 2 / 48000000 * RCC_PLLCFGR_PLLQ_0)
#elif (SYSTEM_CLOCK_HZ * 4 >= 192000000)\
   && (SYSTEM_CLOCK_HZ * 4 <= 432000000)\
   && (SYSTEM_CLOCK_HZ * 4 % 48000000 == 0)
    #define RCC_PLLCFGR_PLLN_BITS (SYSTEM_CLOCK_HZ * 4 / ONE_MHZ * RCC_PLLCFGR_PLLN_0)
    #define RCC_PLLCFGR_PLLP_BITS (RCC_PLLCFGR_PLLP_0)  // P = 4
    #define RCC_PLLCFGR_PLLQ_BITS (SYSTEM_CLOCK_HZ * 4 / 48000000 * RCC_PLLCFGR_PLLQ_0)
#elif (SYSTEM_CLOCK_HZ * 6 >= 192000000)\
   && (SYSTEM_CLOCK_HZ * 6 <= 432000000)\
   && (SYSTEM_CLOCK_HZ * 6 % 48000000 == 0)
    #define RCC_PLLCFGR_PLLN_BITS (SYSTEM_CLOCK_HZ * 6 / ONE_MHZ * RCC_PLLCFGR_PLLN_0)
    #define RCC_PLLCFGR_PLLP_BITS (RCC_PLLCFGR_PLLP_1)  // P = 6
    #define RCC_PLLCFGR_PLLQ_BITS (SYSTEM_CLOCK_HZ * 6 / 48000000 * RCC_PLLCFGR_PLLQ_0)
#elif (SYSTEM_CLOCK_HZ * 8 >= 192000000)\
   && (SYSTEM_CLOCK_HZ * 8 <= 432000000)\
   && (SYSTEM_CLOCK_HZ * 8 % 48000000 == 0)
    #define RCC_PLLCFGR_PLLN_BITS (SYSTEM_CLOCK_HZ * 8 / ONE_MHZ * RCC_PLLCFGR_PLLN_0)
    #define RCC_PLLCFGR_PLLP_BITS (RCC_PLLCFGR_PLLP_0 | RCC_PLLCFGR_PLLP_1)  // P = 8
    #define RCC_PLLCFGR_PLLQ_BITS (SYSTEM_CLOCK_HZ * 8 / 48000000 * RCC_PLLCFGR_PLLQ_0)
#else
    #error illegal SYSTEM_CLOCK_HZ frequency
#endif
 
#define RCC_PLLCFGR_PLL_BITS (RCC_PLLCFGR_PLLM_BITS \
                            | RCC_PLLCFGR_PLLN_BITS \
                            | RCC_PLLCFGR_PLLP_BITS \
                            | RCC_PLLCFGR_PLLQ_BITS \
                            | RCC_PLLCFGR_PLLS_BITS)
*/

#if SYSTEM_CLOCK_HZ == SYSTEM_CYCLE_CLOCK_HZ * 1
    #define RCC_CFGR_HPRE_DIV_BITS RCC_D1CFGR_HPRE_DIV1
#elif SYSTEM_CLOCK_HZ == SYSTEM_CYCLE_CLOCK_HZ * 2
    #define RCC_CFGR_HPRE_DIV_BITS RCC_D1CFGR_HPRE_DIV2
#elif SYSTEM_CLOCK_HZ == SYSTEM_CYCLE_CLOCK_HZ * 4
    #define RCC_CFGR_HPRE_DIV_BITS RCC_D1CFGR_HPRE_DIV4
#elif SYSTEM_CLOCK_HZ == SYSTEM_CYCLE_CLOCK_HZ * 8
    #define RCC_CFGR_HPRE_DIV_BITS RCC_D1CFGR_HPRE_DIV8
#elif SYSTEM_CLOCK_HZ == SYSTEM_CYCLE_CLOCK_HZ * 16
    #define RCC_CFGR_HPRE_DIV_BITS RCC_D1CFGR_HPRE_DIV16
#elif SYSTEM_CLOCK_HZ == SYSTEM_CYCLE_CLOCK_HZ * 64
    #define RCC_CFGR_HPRE_DIV_BITS RCC_D1CFGR_HPRE_DIV64
#elif SYSTEM_CLOCK_HZ == SYSTEM_CYCLE_CLOCK_HZ * 128
    #define RCC_CFGR_HPRE_DIV_BITS RCC_D1CFGR_HPRE_DIV128
#elif SYSTEM_CLOCK_HZ == SYSTEM_CYCLE_CLOCK_HZ * 256
    #define RCC_CFGR_HPRE_DIV_BITS RCC_D1CFGR_HPRE_DIV256
#elif SYSTEM_CLOCK_HZ == SYSTEM_CYCLE_CLOCK_HZ * 512
    #define RCC_CFGR_HPRE_DIV_BITS RCC_D1CFGR_HPRE_DIV512
#else
    #error SYSTEM_CLOCK_HZ must be SYSTEM_CYCLE_CLOCK_HZ * 1, 2, 4, 8, .. 256, or 512
#endif

#if SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB1_CLOCK_HZ * 1
    #define RCC_CFGR_PPRE1_DIV_BITS RCC_D1CFGR_D1PPRE_DIV1
#elif SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB1_CLOCK_HZ * 2
    #define RCC_CFGR_PPRE1_DIV_BITS RCC_D1CFGR_D1PPRE_DIV2
#elif SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB1_CLOCK_HZ * 4
    #define RCC_CFGR_PPRE1_DIV_BITS RCC_D1CFGR_D1PPRE_DIV4
#elif SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB1_CLOCK_HZ * 8
    #define RCC_CFGR_PPRE1_DIV_BITS RCC_D1CFGR_D1PPRE_DIV8
#elif SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB1_CLOCK_HZ * 16
    #define RCC_CFGR_PPRE1_DIV_BITS RCC_D1CFGR_D1PPRE_DIV16
#else
    #error SYSTEM_CYCLE_CLOCK_HZ must be SYSTEM_APB1_CLOCK_HZ * 1, 2, 4, 8, or 16
#endif

#if SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB2_CLOCK_HZ * 1
    #define RCC_CFGR_PPRE2_DIV_BITS RCC_D1CFGR_D1PPRE_DIV1
#elif SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB2_CLOCK_HZ * 2
    #define RCC_CFGR_PPRE2_DIV_BITS RCC_D1CFGR_D1PPRE_DIV2
#elif SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB2_CLOCK_HZ * 4
    #define RCC_CFGR_PPRE2_DIV_BITS RCC_D1CFGR_D1PPRE_DIV4
#elif SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB2_CLOCK_HZ * 8
    #define RCC_CFGR_PPRE2_DIV_BITS RCC_D1CFGR_D1PPRE_DIV8
#elif SYSTEM_CYCLE_CLOCK_HZ == SYSTEM_APB2_CLOCK_HZ * 16
    #define RCC_CFGR_PPRE2_DIV_BITS RCC_D1CFGR_D1PPRE_DIV16
#else
    #error SYSTEM_CYCLE_CLOCK_HZ must be SYSTEM_APB2_CLOCK_HZ * 1, 2, 4, 8, or 16
#endif

#if SUPPLY_VOLTAGE_MV < 2100
    #if SYSTEM_CYCLE_CLOCK_HZ <= 20000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_0WS // no wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 40000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_1WS // 1 wait state
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 60000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_2WS // 2 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 80000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_3WS // 3 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 100000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_4WS // 4 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 120000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_5WS // 5 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 140000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_6WS // 6 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 160000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_7WS // 7 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 180000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_8WS // 8 wait states
    #else
        #error SYSTEM_CYCLE_CLOCK_HZ must be <= 160MHz at < 2.1V
    #endif
#elif SUPPLY_VOLTAGE_MV < 2400
    #if SYSTEM_CYCLE_CLOCK_HZ <= 22000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_0WS // no wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 44000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_1WS // 1 wait state
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 66000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_2WS // 2 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 88000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_3WS // 3 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 110000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_4WS // 4 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 1328000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_5WS // 5 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 154000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_6WS // 6 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 176000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_7WS // 7 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 198000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_8WS // 8 wait states
    #else
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_9WS // 9 wait states
    #endif
#elif SUPPLY_VOLTAGE_MV < 2700
    #if SYSTEM_CYCLE_CLOCK_HZ <= 24000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_0WS // no wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 48000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_1WS // 1 wait state
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 72000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_2WS // 2 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 96000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_3WS // 3 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 120000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_4WS // 4 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 144000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_5WS // 5 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 168000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_6WS // 6 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 192000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_7WS // 7 wait states
    #else
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_8WS // 8 wait states
    #endif
#else
    #if SYSTEM_CYCLE_CLOCK_HZ <= 30000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_0WS // no wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 60000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_1WS // 1 wait state
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 90000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_2WS // 2 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 120000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_3WS // 3 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 150000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_4WS // 4 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 180000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_5WS // 5 wait states
    #elif SYSTEM_CYCLE_CLOCK_HZ <= 210000000
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_6WS // 6 wait states
    #else
        #define FLASH_ACR_LATENCY_BITS FLASH_ACR_LATENCY_7WS // 7 wait states
    #endif
#endif


#pragma arm section code = "SectionForBootstrapOperations"


/* IO initialization implemented in solution DeviceCode\Init */
void BootstrapCode_GPIO();


extern "C"
{

void __section("SectionForBootstrapOperations") STM32H7_BootstrapCode()
{
	//hal_printf("test 1");
	//debug_printf("test 1");
	//HAL_Init();
	//SYSCFG_SWITCH_PA0_OPEN;
    // assure interupts off
    __disable_irq();

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

    SCB_EnableICache(); // Invalidate and enable L1 Instruction cache

    // UNDONE: FIXME: D-Cache temporarily disabled, as it causes problems with setting interrupt vectors (located in cache-able RAM)
    SCB_EnableDCache(); // Invalidate and enable L1 Data cache

	HAL_Init();
	
    // enable FPU coprocessors (CP10, CP11)
    //SCB->CPACR |= 0x3 << 2 * 10 | 0x3 << 2 * 11; // full access

    // allow unaligned memory access (Cortex-M7 has STKALIGN read-only)
    //SCB->CCR &= ~SCB_CCR_UNALIGN_TRP_Msk;

	/*!< Supply configuration update enable */
	MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

	/* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
	MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, (PWR_D3CR_VOS_1 | PWR_D3CR_VOS_0));

	while(!(PWR->D3CR & PWR_D3CR_VOSRDY));

	RCC->CR |= RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY));

	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSE);   
	RCC->CR &= ~(RCC_CR_PLL1ON);
	
	RCC->PLL1DIVR = ( (((__PLLN1__) - 1U )& RCC_PLL1DIVR_N1) | ((((__PLLP1__) -1U ) << 9U) & RCC_PLL1DIVR_P1) | \
                                ((((__PLLQ1__) -1U) << 16U)& RCC_PLL1DIVR_Q1) | ((((__PLLR1__) - 1U) << 24U)& RCC_PLL1DIVR_R1) );
	
	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE, RCC_PLLCFGR_PLL1RGE_2);

	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1VCOSEL, ((uint32_t)0x00000000));
	
	SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVP1EN);
	
	SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVQ1EN);
	
	SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVR1EN);
	
	SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLL1FRACEN);
	
	SET_BIT(RCC->CR, RCC_CR_PLL1ON);
	
    while(!(RCC->CR & RCC_CR_PLLRDY));

	MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_4WS);
	
	// HCLK configuration 
	MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_HPRE, RCC_D1CFGR_HPRE_DIV2);
	// SYSCLK Configuration
	MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1CPRE, RCC_D1CFGR_D1CPRE_DIV1);
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL1);   
	
	MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1PPRE,  RCC_D1CFGR_D1PPRE_DIV2);
	MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1, RCC_D2CFGR_D2PPRE1_DIV2);
	MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2, RCC_D2CFGR_D2PPRE2_DIV2);
	MODIFY_REG(RCC->D3CFGR, RCC_D2CFGR_D2PPRE1, RCC_D3CFGR_D3PPRE_DIV1);
	
	//RCC->AHB1ENR = 0;   
	//RCC->AHB2ENR = 0;
	RCC->AHB3LPENR = (RCC_AHB3LPENR_DTCM1LPEN);
    //RCC->APB4ENR = RCC_APB4ENR_SYSCFGEN; // SYSCFG clock used for IO
	
	/*RCC->AHB3LPENR = (RCC_AHB3LPENR_DTCM1LPEN);
	
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_D2SRAM1EN);
	
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_D2SRAM2EN);
	
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_D2SRAM3EN);
	*/
	// Enables the I/O Compensation Cell
	SET_BIT(RCC->CR, RCC_CR_CSION);
	
	// The I/O compensation cell can be used only when the device supply
	SET_BIT(RCC->APB4ENR, RCC_APB4ENR_SYSCFGEN); // SYSCFG clock used for IO
	
	// voltage ranges from 2.4 to 3.6 V.
 	SET_BIT(SYSCFG->CCCSR, SYSCFG_CCCSR_EN); 
		
	SET_BIT(RCC->PLLCFGR, (RCC_PLLCFGR_DIVQ1EN));
	MODIFY_REG(RCC->D2CCIP2R, RCC_D2CCIP2R_USBSEL, RCC_D2CCIP2R_USBSEL_0);
	MODIFY_REG(RCC->D2CCIP2R, RCC_D2CCIP2R_USART28SEL, ((uint32_t)0x00000000));
	SET_BIT(PWR->CR3, PWR_CR3_USB33DEN);
}

__section("SectionForBootstrapOperations") void BootstrapCode()
{
    STM32H7_BootstrapCode();

    PrepareImageRegions();

    BootstrapCode_GPIO();
}

__section("SectionForBootstrapOperations") void BootstrapCodeMinimal()
{
    STM32H7_BootstrapCode();

    BootstrapCode_GPIO();
}

}

