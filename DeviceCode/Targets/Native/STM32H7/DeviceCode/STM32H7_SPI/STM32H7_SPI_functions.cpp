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
//#include "..\stm32h7xx_main.h"

//--//

SPI_HandleTypeDef hspi1;

// We don't currently use SPI interrupts
/*
extern "C" {
void SPI1_IRQHandler(void)
{
	INTERRUPT_START;
	HAL_SPI_IRQHandler(&SpiHandle[0]);
	
	INTERRUPT_END;
}
}
*/

static void Error_Handler(void)
{
	__BKPT();
}

// Don't do anything until we get the bus in CPU_SPI_Init()
BOOL CPU_SPI_Initialize()
{	
  return TRUE;
}

void CPU_SPI_Uninitialize()
{

}

BOOL CPU_SPI_nWrite16_nRead16( const SPI_CONFIGURATION& Configuration, UINT16* Write16, INT32 WriteCount, UINT16* Read16, INT32 ReadCount, INT32 ReadStartOffset )
{
	Error_Handler(); // Not used
	/*
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
	*/
	return TRUE;
}

BOOL CPU_SPI_nWrite8_nRead8( const SPI_CONFIGURATION& Configuration, UINT8* Write8, INT32 WriteCount, UINT8* Read8, INT32 ReadCount, INT32 ReadStartOffset )
{
	Error_Handler();
	/*
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
	*/
	return TRUE;
}

BOOL CPU_SPI_Xaction_Start( const SPI_CONFIGURATION& Configuration )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();
	Error_Handler();
    return TRUE;
}

BOOL CPU_SPI_Xaction_Stop( const SPI_CONFIGURATION& Configuration )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();
	Error_Handler();
    return TRUE;
}

BOOL CPU_SPI_Xaction_nWrite16_nRead16( SPI_XACTION_16& Transaction )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();
	Error_Handler();
	/*
	UINT16* outBuf = Transaction.Write16;
    UINT16* inBuf  = Transaction.Read16;
	//HAL_SPI_TransmitReceive_IT(&SpiHandle[Transaction.SPI_mod], (uint8_t)outBuf, (uint8_t)inBuf,  (uint16_t)Transaction.WriteCount);
	*/
    return TRUE;
}

BOOL CPU_SPI_Xaction_nWrite8_nRead8( SPI_XACTION_8& Transaction )
{
	NATIVE_PROFILE_HAL_PROCESSOR_SPI();
	Error_Handler();
	/*
	UINT8* outBuf = Transaction.Write8;
    UINT8* inBuf = Transaction.Read8;
	//HAL_SPI_Transmit_IT(&SpiHandle, (uint8_t *)outBuf, 4);
    
	//hal_printf(" 85 SPI_mod:%d \n", Transaction.SPI_mod);	
	//HAL_SPI_TransmitReceive_IT(&SpiHandle, (uint8_t *)outBuf, (uint8_t *)inBuf , (uint16_t)Transaction.WriteCount);
	//HAL_SPI_Transmit_IT(&SpiHandle[Transaction.SPI_mod], (uint8_t *)outBuf, (uint16_t)Transaction.WriteCount);
	HAL_SPI_TransmitReceive_IT(&SpiHandle[Transaction.SPI_mod], (uint8_t *)outBuf, (uint8_t *)inBuf , (uint16_t)Transaction.WriteCount);
    //HAL_SPI_Transmit_IT(&SpiHandle, (uint8_t *)aTxBuffer2, BUFFERSIZE);// (uint8_t *)aRxBuffer2), BUFFERSIZE);
	*/
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

void CPU_SPI_Init(UINT8 SPI_TYPE)
{
	// SPI_TYPE_RADIO is only option we understand
	if (SPI_TYPE != SPI_TYPE_RADIO) { Error_Handler(); return; }

	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 0x0;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
	hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
	hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
	hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
	hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
	hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
	hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
	hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
	hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) { Error_Handler(); }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// nSS pin PC1
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET); // default un-asserted
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1);
  }
}

void CPU_SPI_DeInit(UINT8 SPI_TYPE)
{
	switch(SPI_TYPE) {
		case SPI_TYPE_RADIO: if (HAL_SPI_DeInit(&hspi1) != HAL_OK) { Error_Handler(); } break;
		default: Error_Handler();
	}
}

/*!
 * @brief Sends outData and receives inData
 *
 * @param [IN] outData Byte to be sent
 * @retval inData      Received byte.
 */
UINT16 CPU_SPI_InOut(UINT8 SPI_TYPE, UINT16 txData )
{
	UINT16 rxData ;
	
	//HAL_SPI_TransmitReceive( &SpiHandle[SPI_TYPE], ( UINT8 * ) &txData, ( UINT8* ) &rxData, 1, HAL_MAX_DELAY);
	HAL_SPI_TransmitReceive( &hspi1, ( UINT8 * ) &txData, ( UINT8* ) &rxData, 1, HAL_MAX_DELAY);
	
	return rxData;
}



