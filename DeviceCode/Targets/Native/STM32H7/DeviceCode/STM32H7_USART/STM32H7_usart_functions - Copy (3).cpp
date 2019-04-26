////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//  Implementation for STM32H7: Copyright (c) Oberon microsystems, Inc.
//
//  *** STM32H7 Serial Driver ***
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"
#include "..\stm32h7xx_main.h"

UART_HandleTypeDef UsartHandle;

/* Buffer used for transmission */
uint8_t aTxBuffer[] = " **** USART_SlaveMode ****  **** USART_SlaveMode ****  **** USART_SlaveMode **** ";

/* Buffer used for reception */
uint8_t aRxBuffer[RXBUFFERSIZE];

static void Error_Handler(void);  

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	  if(huart->Instance==USART3)
	  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();
  
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration    
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX 
    */
    GPIO_InitStruct.Pin = STLK_RX_Pin|STLK_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
  hal_printf(" 33 stm32h7xx_hal_msp.cpp \n");
  }
  //GPIO_InitTypeDef  GPIO_InitStruct;
  
  
  //if(husart->Instance == USARTx)
  //{
    /*##-1- Enable peripherals and GPIO Clocks ###############################*/
    /* Enable GPIO TX/RX clock */
    //USARTx_TX_GPIO_CLK_ENABLE();
    //USARTx_RX_GPIO_CLK_ENABLE();
    // hal_printf(" 41 stm32h7xx_hal_msp.cpp \n");
    /* Enable USARTx clock */
    //USARTx_CLK_ENABLE(); 
    
    /*##-2- Configure peripheral GPIO ########################################*/  
    /* USART TX GPIO pin configuration  */
    //GPIO_InitStruct.Pin       = USARTx_TX_PIN;
    //GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    //GPIO_InitStruct.Pull      = GPIO_NOPULL;
    //GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    //GPIO_InitStruct.Alternate = USARTx_TX_AF;
    
    //HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
    
    /* USART RX GPIO pin configuration  */
    //GPIO_InitStruct.Pin = USARTx_RX_PIN;
    //GPIO_InitStruct.Alternate = USARTx_RX_AF;
    
    //HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
    
    /* USART CK GPIO pin configuration  */
  // GPIO_InitStruct.Pin = USARTx_CK_PIN;
  //  GPIO_InitStruct.Alternate = USARTx_CK_AF;
    
  //  HAL_GPIO_Init(USARTx_CK_GPIO_PORT, &GPIO_InitStruct);
  //}
}
  
BOOL CPU_USART_Initialize( int ComPortNum, int BaudRate, int Parity, int DataBits, int StopBits, int FlowValue )
{
	  UsartHandle.Instance = USART3;
	  UsartHandle.Init.BaudRate = 115200;
	  UsartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	  UsartHandle.Init.StopBits = UART_STOPBITS_1;
	  UsartHandle.Init.Parity = UART_PARITY_NONE;
	  UsartHandle.Init.Mode = UART_MODE_TX_RX;
	  UsartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	  UsartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	  UsartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	  UsartHandle.Init.Prescaler = UART_PRESCALER_DIV1;
	  UsartHandle.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	  UsartHandle.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	  UsartHandle.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	  UsartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	  if (HAL_UART_Init(&UsartHandle) != HAL_OK)
	  {
		Error_Handler();
	  } 
  
  
/* 	  UsartHandle.Instance          = USARTx;
		UsartHandle.Init.SlaveMode    = USART_SLAVEMODE_ENABLE;
	   UsartHandle.Init.WordLength   = USART_WORDLENGTH_8B;
	  UsartHandle.Init.StopBits     = USART_STOPBITS_1;
	  UsartHandle.Init.Parity       = USART_PARITY_NONE;
	  UsartHandle.Init.Mode         = USART_MODE_TX_RX;
	  UsartHandle.Init.CLKPolarity  = USART_POLARITY_LOW;
	  UsartHandle.Init.CLKPhase     = USART_PHASE_1EDGE;
	  UsartHandle.Init.CLKLastBit   = USART_LASTBIT_ENABLE;
	  UsartHandle.Init.Prescaler    = USART_PRESCALER_DIV1;
	  UsartHandle.Init.NSS          = USART_NSS_SW;
	  UsartHandle.Init.FIFOMode     = USART_FIFOMODE_DISABLE;
	  */
	  hal_printf(" 50 usart_functions.cpp \n");
/* 
	  if(HAL_USART_Init(&UsartHandle) != HAL_OK)
	  {
		hal_printf(" 55 Error usart_functions.cpp \n");
		Error_Handler();
	  }
	  hal_printf(" 57 usart_functions.cpp \n"); */
	  /*##-2- Start the transmission/reception process ###########################*/
	  if(HAL_UART_Transmit_IT(&UsartHandle, (uint8_t*)aTxBuffer, TXBUFFERSIZE)!= HAL_OK)
	  {
		Error_Handler();
		  hal_printf(" 138 usart_functions.cpp \n");
	  }  
	  hal_printf(" 140 usart_functions.cpp \n"); 
    return TRUE;
}

BOOL CPU_USART_Uninitialize( int ComPortNum )
{
    return TRUE;
}

BOOL CPU_USART_TxBufferEmpty( int ComPortNum )
{
    return TRUE;
}

BOOL CPU_USART_TxShiftRegisterEmpty( int ComPortNum )
{
    return TRUE;
}

void CPU_USART_WriteCharToTxBuffer( int ComPortNum, UINT8 c )
{
}

void CPU_USART_TxBufferEmptyInterruptEnable( int ComPortNum, BOOL Enable )
{
}

BOOL CPU_USART_TxBufferEmptyInterruptState( int ComPortNum )
{
    return TRUE;
}

void CPU_USART_RxBufferFullInterruptEnable( int ComPortNum, BOOL Enable )
{
}

BOOL CPU_USART_RxBufferFullInterruptState( int ComPortNum )
{
    return TRUE;
}

void CPU_USART_ProtectPins( int ComPortNum, BOOL On )
{
}

UINT32 CPU_USART_PortsCount()
{
    return 0;
}

void CPU_USART_GetPins( int ComPortNum, GPIO_PIN& rxPin, GPIO_PIN& txPin,GPIO_PIN& ctsPin, GPIO_PIN& rtsPin )
{   
    rxPin = GPIO_PIN_NONE; 
    txPin = GPIO_PIN_NONE; 
    ctsPin= GPIO_PIN_NONE; 
    rtsPin= GPIO_PIN_NONE; 

    return;
}

BOOL CPU_USART_SupportNonStandardBaudRate ( int ComPortNum )
{
    return FALSE;
}

void CPU_USART_GetBaudrateBoundary( int ComPortNum, UINT32& maxBaudrateHz, UINT32& minBaudrateHz )
{
    maxBaudrateHz = 0;
    minBaudrateHz = 0;
}

BOOL CPU_USART_IsBaudrateSupported( int ComPortNum, UINT32 & BaudrateHz )
{   
    return FALSE;
}

BOOL CPU_USART_TxHandshakeEnabledState( int comPort )
{
    return TRUE;
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
hal_printf(" 153 Error_handler usart_functions.cpp \n");
 
}

void Debug_Print_in_HAL(const char* format){
	hal_printf(format);
}
