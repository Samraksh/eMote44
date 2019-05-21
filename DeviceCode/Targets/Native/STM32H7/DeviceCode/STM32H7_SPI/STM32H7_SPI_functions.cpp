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

extern "C" {
void SPI1_IRQHandler(void)
{
	INTERRUPT_START;
	
	HAL_SPI_IRQHandler(&SpiHandle[0]);
	
	INTERRUPT_END;
}
}

//extern "C" void SPI1_IRQHandler(void)
//{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */
 // hal_printf(" 32 SPI1_IRQHandler \n");
  /* USER CODE END OTG_FS_IRQn 0 */
//  HAL_SPI_IRQHandler(&SpiHandle[0]);
  //HAL_SPI_IRQHandler(&SpiHandle);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */
 // hal_printf(" 35 SPI1_IRQHandler \n");
  /* USER CODE END OTG_FS_IRQn 1 */
//}

//void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//	hal_printf(" 41 SPI complete callback \n");
//}

//void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
//{
//	//hal_printf(" 41 SPI error callback \n");
//}
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
	
	GPIO_InitTypeDef  GPIO_InitStruct;
	
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
	//__NVIC_SetVector(SPIx_IRQn, (uint32_t)SPI1_IRQHandler);
    HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
	
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
		Error_Handler();

	}  
		
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
    
	//hal_printf(" 85 SPI_mod:%d \n", Transaction.SPI_mod);	
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



/*!
 * @brief Initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
void CPU_SPI_Init(UINT8 SPI_TYPE)
{  
	/*##-1- Configure the SPI peripheral */
	/* Set the SPI parameters */
	if (SPI_TYPE == SPI_TYPE_RADIO) {
		SpiHandle[SPI_TYPE].Instance = SPI1;

		SpiHandle[SPI_TYPE].Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;//SpiFrequency( 10000000 );
		SpiHandle[SPI_TYPE].Init.Direction      = SPI_DIRECTION_2LINES;
		SpiHandle[SPI_TYPE].Init.Mode           = SPI_MODE_MASTER;
		SpiHandle[SPI_TYPE].Init.CLKPolarity    = SPI_POLARITY_LOW;
		SpiHandle[SPI_TYPE].Init.CLKPhase       = SPI_PHASE_1EDGE;
		SpiHandle[SPI_TYPE].Init.DataSize       = SPI_DATASIZE_8BIT;
		SpiHandle[SPI_TYPE].Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;  
		SpiHandle[SPI_TYPE].Init.FirstBit       = SPI_FIRSTBIT_MSB;
		SpiHandle[SPI_TYPE].Init.NSS            = SPI_NSS_SOFT;
		SpiHandle[SPI_TYPE].Init.TIMode         = SPI_TIMODE_DISABLE;
		
		/*	LoRaSpiHandle.Init.CRCPolynomial = 7; */
	
	}
	
	__HAL_RCC_SPI1_CLK_ENABLE();//SPI_CLK_ENABLE(); 

	if(HAL_SPI_Init(&SpiHandle[SPI_TYPE]) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/*##-2- Configure the SPI GPIOs */
	CPU_SPI_IoInit(SPI_TYPE);
}

/*!
 * @brief De-initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
void CPU_SPI_DeInit(UINT8 SPI_TYPE)
{
	HAL_SPI_DeInit(&SpiHandle[SPI_TYPE]);

	if (SPI_TYPE == SPI_TYPE_RADIO) {
		/*##-1- Reset peripherals ####*/
		__HAL_RCC_SPI1_FORCE_RESET();
		__HAL_RCC_SPI1_RELEASE_RESET();
	}
	/*##-2- Configure the SPI GPIOs */
	CPU_SPI_IoDeInit(SPI_TYPE);
}

void CPU_SPI_IoInit(UINT8 SPI_TYPE)
{
	GPIO_InitTypeDef initStruct={0};

	if (SPI_TYPE == SPI_TYPE_RADIO) { 
		initStruct.Mode = GPIO_MODE_IT_RISING;
		initStruct.Pull = GPIO_PULLDOWN;
		initStruct.Speed = GPIO_SPEED_HIGH;

		CPU_GPIO_Init( RADIO_DIO_0_PORT, RADIO_DIO_0_PIN, &initStruct );
		CPU_GPIO_Init( RADIO_DIO_1_PORT, RADIO_DIO_1_PIN, &initStruct );
		CPU_GPIO_Init( RADIO_DIO_2_PORT, RADIO_DIO_2_PIN, &initStruct );
		CPU_GPIO_Init( RADIO_DIO_3_PORT, RADIO_DIO_3_PIN, &initStruct );

		initStruct.Mode = GPIO_MODE_AF_PP;
		initStruct.Pull = GPIO_NOPULL;
		initStruct.Speed = GPIO_SPEED_HIGH;
		initStruct.Alternate = GPIO_AF5_SPI1 ;

		CPU_GPIO_Init( RADIO_SCLK_PORT, RADIO_SCLK_PIN, &initStruct);		
		CPU_GPIO_Init( RADIO_MISO_PORT, RADIO_MISO_PIN, &initStruct);
		CPU_GPIO_Init( RADIO_MOSI_PORT, RADIO_MOSI_PIN, &initStruct);

		initStruct.Mode = GPIO_MODE_OUTPUT_PP;
		initStruct.Pull = GPIO_NOPULL;

		CPU_GPIO_Init(RADIO_NSS_PORT, RADIO_NSS_PIN, &initStruct );

		CPU_GPIO_Write(RADIO_NSS_PORT, RADIO_NSS_PIN, 1 );
	}
}

void CPU_SPI_IoDeInit(UINT8 SPI_TYPE)
{
	GPIO_InitTypeDef initStruct={0};

	if (SPI_TYPE == SPI_TYPE_RADIO) { 	
		initStruct.Mode = GPIO_MODE_IT_RISING ;
		initStruct.Pull = GPIO_PULLDOWN;

		CPU_GPIO_Init( RADIO_DIO_0_PORT, RADIO_DIO_0_PIN, &initStruct );
		CPU_GPIO_Init( RADIO_DIO_1_PORT, RADIO_DIO_1_PIN, &initStruct );
		CPU_GPIO_Init( RADIO_DIO_2_PORT, RADIO_DIO_2_PIN, &initStruct );
		CPU_GPIO_Init( RADIO_DIO_3_PORT, RADIO_DIO_3_PIN, &initStruct );	 
		
		initStruct.Mode = GPIO_MODE_OUTPUT_PP;

		initStruct.Pull = GPIO_NOPULL  ; 
		CPU_GPIO_Init ( RADIO_MOSI_PORT, RADIO_MOSI_PIN, &initStruct ); 
		CPU_GPIO_Write( RADIO_MOSI_PORT, RADIO_MOSI_PIN, 0 );

		initStruct.Pull = GPIO_PULLDOWN; 
		CPU_GPIO_Init ( RADIO_MISO_PORT, RADIO_MISO_PIN, &initStruct ); 
		CPU_GPIO_Write( RADIO_MISO_PORT, RADIO_MISO_PIN, 0 );

		initStruct.Pull = GPIO_NOPULL  ; 
		CPU_GPIO_Init ( RADIO_SCLK_PORT, RADIO_SCLK_PIN, &initStruct ); 
		CPU_GPIO_Write( RADIO_SCLK_PORT, RADIO_SCLK_PIN, 0 );
  
		initStruct.Pull = GPIO_NOPULL  ; 
		CPU_GPIO_Init ( RADIO_NSS_PORT, RADIO_NSS_PIN , &initStruct ); 
		CPU_GPIO_Write( RADIO_NSS_PORT, RADIO_NSS_PIN , 1 );	

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
	if (HAL_SPI_TransmitReceive( &SpiHandle[SPI_TYPE], ( UINT8 * ) &txData, ( UINT8* ) &rxData, 1, HAL_MAX_DELAY) != HAL_OK) {
		hal_printf("SPI Wrong\n\r");
		__asm__("BKPT"); // Something is terribly wrong. TODO: DELETE ME. SANITY CHECK FOR DEBUG.
	}	
	return rxData;
}

/* Exported functions ---------------------------------------------------------*/
/*!
 * @brief Initializes the given GPIO object
 *
 * @param  GPIOx: where x can be (A..E and H)
 * @param  GPIO_Pin: specifies the port bit to be written.
 *                   This parameter can be one of GPIO_PIN_x where x can be (0..15).
 *                   All port bits are not necessarily available on all GPIOs.
 * @param [IN] initStruct  GPIO_InitTypeDef intit structure
 * @retval none
 */
void CPU_GPIO_Init( GPIO_TypeDef* port, uint16_t GPIO_Pin, GPIO_InitTypeDef* initStruct)
{

  RCC_GPIO_CLK_ENABLE((uint32_t) port);

  initStruct->Pin = GPIO_Pin ;

  HAL_GPIO_Init( port, initStruct );
}

/*!
 * @brief Writes the given value to the GPIO output
 *
 * @param  GPIO_Pin: specifies the port bit to be written.
 *                   This parameter can be one of GPIO_PIN_x where x can be (0..15).
 *                   All port bits are not necessarily available on all GPIOs.
 * @param [IN] value New GPIO output value
 * @retval none
 */
void CPU_GPIO_Write( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,  uint32_t value )
{
  HAL_GPIO_WritePin( GPIOx, GPIO_Pin , (GPIO_PinState) value );
}

/*!
 * @brief Reads the current GPIO input value
 *
 * @param  GPIOx: where x can be (A..E and H) 
 * @param  GPIO_Pin: specifies the port bit to be written.
 *                   This parameter can be one of GPIO_PIN_x where x can be (0..15).
 *                   All port bits are not necessarily available on all GPIOs.
 * @retval value   Current GPIO input value
 */
uint32_t CPU_GPIO_Read( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin )
{
  return HAL_GPIO_ReadPin( GPIOx, GPIO_Pin);
}


