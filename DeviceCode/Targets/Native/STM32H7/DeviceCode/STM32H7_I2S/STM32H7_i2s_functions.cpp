////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//  Implementation for STM32H7: Copyright (c) Oberon microsystems, Inc.
//
//  *** I2S Driver ***
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h> 
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"
#include "..\stm32h7xx_main.h"
#include "mfcc.h"

#include <limits.h>

I2S_HandleTypeDef hi2s3;

//#define MY_ENABLE_CACHE
#define BREAKPOINT() __asm__("BKPT")

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
	BREAKPOINT();
}


static void print_cpu_config(void) {
	PLL1_ClocksTypeDef pll1;
	uint32_t spi1;
	hal_printf("CPU Clock: %u MHz\r\n", HAL_RCC_GetSysClockFreq()/1000000);
#ifdef MY_ENABLE_CACHE
	hal_printf("CPU Cache: Enabled\r\n");
#else
	hal_printf("CPU Cache: Disabled\r\n");
#endif
	HAL_RCCEx_GetPLL1ClockFreq(&pll1);
	spi1 = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI123);
	hal_printf("P: %d MHz, Q: %d MHz, R: %d MHz spi1: %d MHz\r\n",
		pll1.PLL1_P_Frequency/1000000, pll1.PLL1_Q_Frequency/1000000, pll1.PLL1_R_Frequency/1000000, spi1/1000000);
}

// Version for H7 which only needs sign extension
static int16_t i2s24_to_pcm16_h7(uint32_t x) {
	// 24-bit to 16-bit
	int16_t ret = (int16_t) (x >> 8);
	return ret;
}

#define MY_FRAME_SIZE 1024
static uint32_t audio_data[MY_FRAME_SIZE];
static int16_t mel_input_data[MY_FRAME_SIZE];
#define AUDIO_LEN (sizeof(audio_data)/sizeof(audio_data[0])) 	// 32-bit samples
#define AUDIO_BYTES (sizeof(audio_data))						// Bytes of data


void HAL_I2S_MspInit(I2S_HandleTypeDef* i2sHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2sHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* I2S1 clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();
  
    //__HAL_RCC_GPIOA_CLK_ENABLE();
    //__HAL_RCC_GPIOD_CLK_ENABLE();
	
	/**I2S3 GPIO Configuration    
    PA15 (JTDI)     ------> I2S3_WS
    PC10      ------> I2S3_CK
	PC11      ------> I2S3_SDI
	PC12     ------> I2S3_SDO 
    */
	/*
	GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	*/
	
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 /*
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef* i2sHandle)
{

  if(i2sHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();
  
	/**I2S3 GPIO Configuration    
    PA15 (JTDI)     ------> I2S3_WS
    PB3      ------> I2S3_CK
    PD22     ------> I2S3_SDO 
    */
	
	//HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6);

    //HAL_GPIO_DeInit(GPIOD, GPIO_PIN_7);
	
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

    //HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
} 

BOOL I2S_Internal_Initialize()
{
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_24B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_8K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.FirstBit = I2S_FIRSTBIT_MSB;
  hi2s3.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
  //hi2s3.Init.IOSwap = I2S_IO_SWAP_DISABLE;
  //hi2s3.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
  //hi2s3.Init.FifoThreshold = I2S_FIFO_THRESHOLD_01DATA;
  hi2s3.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_DISABLE;
  //hi2s3.Init.SlaveExtendFREDetection = I2S_SLAVE_EXTEND_FRE_DETECTION_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
}

void I2S_Test()
{
	print_cpu_config();
	mfcc_init();
	while(1) {
		HAL_StatusTypeDef ret;
		do {
			ret = HAL_I2S_Receive(&hi2s3, (uint16_t *)audio_data, AUDIO_LEN/2, INT_MAX); // I CHANGED THE I2S ST DRIVER. REPLACE WITH FRESH.
		} while(ret == HAL_BUSY);
		if (ret != HAL_OK) BREAKPOINT();
		for (int i=0; i<AUDIO_LEN; i++) mel_input_data[i] = i2s24_to_pcm16_h7(audio_data[i]);
		mfcc_test(mel_input_data);
	}
}

BOOL I2S_Internal_Uninitialize()
{
  
}