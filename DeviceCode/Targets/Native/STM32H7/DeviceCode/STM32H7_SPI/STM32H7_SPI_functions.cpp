////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// 
// This file is part of the Microsoft .NET Micro Framework Porting Kit Code Samples and is unsupported. 
// Copyright (C) Microsoft Corporation. All rights reserved. Use of this sample source code is subject to 
// the terms of the Microsoft license agreement under which you licensed this sample source code. 
// 
// THIS SAMPLE CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, 
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"
#include "..\stm32h7xx_main.h"

//--//

SPI_HandleTypeDef SpiHandle[6];
//SPI_HandleTypeDef SpiHandle;
/* Buffer used for transmission */
uint8_t aTxBuffer2[]=" abc";
/* Buffer used for reception */
uint8_t aRxBuffer2[BUFFERSIZE];

void SPI1_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */
  hal_printf(" 32 SPI1_IRQHandler \n");
  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_SPI_IRQHandler(&SpiHandle[0]);
  //HAL_SPI_IRQHandler(&SpiHandle);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */
  hal_printf(" 35 SPI1_IRQHandler \n");
  /* USER CODE END OTG_FS_IRQn 1 */
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	hal_printf(" 41 SPI complete callback \n");
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	//hal_printf(" 41 SPI error callback \n");
}
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED_RED on */
  //BSP_LED_On(LED_RED);
	//hal_printf(" 153 Error_handler usart_functions.cpp \n");
 
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  if (hspi->Instance == SPIx)
  {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    /* Enable SPI clock */
    SPIx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    GPIO_InitStruct.Alternate = SPIx_MISO_AF;
    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

    /*##-3- Configure the NVIC for SPI #########################################*/
    /* NVIC for SPI */
	__NVIC_SetVector(SPIx_IRQn, (uint32_t)SPI1_IRQHandler);
    HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
	//hal_printf(" 78 IRQ_Handler value %x \n", (uint32_t)SPI1_IRQHandler); 
	//hal_printf(" 78 IRQ_Handler value %x \n", (uint32_t *)SCB->VTOR);
	//hal_printf(" 78 IRQ_Handler value %x \n", (uint32_t)__NVIC_GetVector(SPIx_IRQn));
  }
  //hal_printf(" 80 spi_functions.cpp \n"); 
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  if(hspi->Instance == SPIx)
  {
    /*##-1- Reset peripherals ##################################################*/
    SPIx_FORCE_RESET();
    SPIx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* Deconfigure SPI SCK */
    HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
    /* Deconfigure SPI MISO */
    HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
    /* Deconfigure SPI MOSI */
    HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);

    /*##-3- Disable the NVIC for SPI ###########################################*/
    HAL_NVIC_DisableIRQ(SPIx_IRQn);
  }
}

BOOL CPU_SPI_Initialize()
{	
  return TRUE;
}

void CPU_SPI_Uninitialize()
{
}

BOOL CPU_SPI_nWrite16_nRead16( const SPI_CONFIGURATION& Configuration, UINT16* Write16, INT32 WriteCount, UINT16* Read16, INT32 ReadCount, INT32 ReadStartOffset )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();
	
	SPI_XACTION_16 Transaction;
    Transaction.Read16          = Read16;
    Transaction.ReadCount       = ReadCount;
    Transaction.ReadStartOffset = ReadStartOffset;
    Transaction.Write16         = Write16;
    Transaction.WriteCount      = WriteCount;
    Transaction.SPI_mod         = Configuration.SPI_mod;
    if(!CPU_SPI_Xaction_nWrite16_nRead16( Transaction ))
    {
        return FALSE;
    }
	
	return TRUE;
}

BOOL CPU_SPI_nWrite8_nRead8( const SPI_CONFIGURATION& Configuration, UINT8* Write8, INT32 WriteCount, UINT8* Read8, INT32 ReadCount, INT32 ReadStartOffset )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();	
	
	SPI_XACTION_8 Transaction;
    Transaction.Read8           = Read8;
    Transaction.ReadCount       = ReadCount;
    Transaction.ReadStartOffset = ReadStartOffset;
    Transaction.Write8          = Write8;
    Transaction.WriteCount      = WriteCount;
    Transaction.SPI_mod         = Configuration.SPI_mod;
	
	if(!CPU_SPI_Xaction_nWrite8_nRead8( Transaction ))
    {
       return FALSE;
    }
	
	return TRUE;
}

BOOL CPU_SPI_Xaction_Start( const SPI_CONFIGURATION& Configuration )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();
	
	SpiHandle[Configuration.SPI_mod].Instance               = SPIx;
	SpiHandle[Configuration.SPI_mod].Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	SpiHandle[Configuration.SPI_mod].Init.Direction         = SPI_DIRECTION_2LINES;
	SpiHandle[Configuration.SPI_mod].Init.CLKPhase          = SPI_PHASE_1EDGE;
	SpiHandle[Configuration.SPI_mod].Init.CLKPolarity       = SPI_POLARITY_LOW;
	SpiHandle[Configuration.SPI_mod].Init.DataSize          = SPI_DATASIZE_8BIT;
	SpiHandle[Configuration.SPI_mod].Init.FirstBit          = SPI_FIRSTBIT_MSB;
	SpiHandle[Configuration.SPI_mod].Init.TIMode            = SPI_TIMODE_DISABLE;
	SpiHandle[Configuration.SPI_mod].Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	SpiHandle[Configuration.SPI_mod].Init.CRCPolynomial     = 7;
	SpiHandle[Configuration.SPI_mod].Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
	SpiHandle[Configuration.SPI_mod].Init.NSS               = SPI_NSS_SOFT;
	SpiHandle[Configuration.SPI_mod].Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
	SpiHandle[Configuration.SPI_mod].Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;  //Recommanded setting to avoid glitches

	//#ifdef MASTER_BOARD
	SpiHandle[Configuration.SPI_mod].Init.Mode = SPI_MODE_MASTER;
	//#else
	//  SpiHandle.Init.Mode = SPI_MODE_SLAVE;
	//#endif MASTER_BOARD

	if(HAL_SPI_Init(&SpiHandle[Configuration.SPI_mod]) != HAL_OK)
	{
	/* Initialization Error */
		Error_Handler();
		hal_printf(" 108 spi_functions.cpp \n"); 
	}  
	// SpiHandle.Instance               = SPIx;
	// SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	// SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
	// SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
	// SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
	// SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
	// SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	// SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
	// SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	// SpiHandle.Init.CRCPolynomial     = 7;
	// SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
	// SpiHandle.Init.NSS               = SPI_NSS_SOFT;
	// SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
	// SpiHandle.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;  /* Recommanded setting to avoid glitches */

	//#ifdef MASTER_BOARD
	// SpiHandle.Init.Mode = SPI_MODE_MASTER;
	//#else
	// SpiHandle.Init.Mode = SPI_MODE_SLAVE;
	//#endif /* MASTER_BOARD */

	// if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
	// {
	// /* Initialization Error */
		// Error_Handler();
	//hal_printf(" 108 spi_functions.cpp \n"); 
	// }  
	
    return TRUE;
}

BOOL CPU_SPI_Xaction_Stop( const SPI_CONFIGURATION& Configuration )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();
	if(HAL_SPI_DeInit(&SpiHandle[Configuration.SPI_mod]) != HAL_OK)
	//if(HAL_SPI_DeInit(&SpiHandle) != HAL_OK)
	{
	/* Initialization Error */
		Error_Handler();
	//hal_printf(" 108 spi_functions.cpp \n"); 
	} 
	
    return TRUE;
}

BOOL CPU_SPI_Xaction_nWrite16_nRead16( SPI_XACTION_16& Transaction )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();
	
	UINT16* outBuf = Transaction.Write16;
    UINT16* inBuf  = Transaction.Read16;
	
	//HAL_SPI_TransmitReceive_IT(&SpiHandle[Transaction.SPI_mod], (uint8_t)outBuf, (uint8_t)inBuf,  (uint16_t)Transaction.WriteCount);
    return TRUE;
}

BOOL CPU_SPI_Xaction_nWrite8_nRead8( SPI_XACTION_8& Transaction )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();
	
	UINT8* outBuf = Transaction.Write8;
    UINT8* inBuf = Transaction.Read8;
	
	//HAL_SPI_Transmit_IT(&SpiHandle, (uint8_t *)outBuf, 4);
    
	hal_printf(" 85 SPI_mod:%d \n", Transaction.SPI_mod);	
	//HAL_SPI_TransmitReceive_IT(&SpiHandle, (uint8_t *)outBuf, (uint8_t *)inBuf , (uint16_t)Transaction.WriteCount);
	//HAL_SPI_Transmit_IT(&SpiHandle[Transaction.SPI_mod], (uint8_t *)outBuf, (uint16_t)Transaction.WriteCount);
	HAL_SPI_TransmitReceive_IT(&SpiHandle[Transaction.SPI_mod], (uint8_t *)outBuf, (uint8_t *)inBuf , (uint16_t)Transaction.WriteCount);
    //HAL_SPI_Transmit_IT(&SpiHandle, (uint8_t *)aTxBuffer2, BUFFERSIZE);// (uint8_t *)aRxBuffer2), BUFFERSIZE);
  
	return TRUE;
}

UINT32 CPU_SPI_PortsCount()
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();	
	//HAL_SPI_Transmit_IT(&SpiHandle, (uint8_t *)aTxBuffer2, BUFFERSIZE);
    return 0;
}

void CPU_SPI_GetPins( UINT32 spi_mod, GPIO_PIN& msk, GPIO_PIN& miso, GPIO_PIN& mosi )
{
    msk  = GPIO_PIN_NONE;
    miso = GPIO_PIN_NONE;
    mosi = GPIO_PIN_NONE;
    return;
}

UINT32 CPU_SPI_MinClockFrequency( UINT32 spi_mod )
{
    return 0;
}

UINT32 CPU_SPI_MaxClockFrequency( UINT32 spi_mod )
{
    return 0;
}

UINT32 CPU_SPI_ChipSelectLineCount( UINT32 spi_mod )
{
    return 0;
}


