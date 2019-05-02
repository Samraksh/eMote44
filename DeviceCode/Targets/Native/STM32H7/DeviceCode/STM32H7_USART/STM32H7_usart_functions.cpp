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

static UART_HandleTypeDef UsartHandle;
static uint8_t aTxBuffer[] = " ABCDEF ";
static uint8_t aRxBuffer[RXBUFFERSIZE];

static void Error_Handler(void);

extern "C" void USART3_IRQHandler(void) {
  HAL_UART_IRQHandler(&UsartHandle);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	hal_printf(" 41 HAL_UART_TxCpltCallback.cpp \n");
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(huart->Instance==USART3)
	{
		__HAL_RCC_USART3_CLK_ENABLE();

		__HAL_RCC_GPIOD_CLK_ENABLE();
		/**USART3 GPIO Configuration    
		PD8     ------> USART3_TX
		PD9     ------> USART3_RX 
		*/
		GPIO_InitStruct.Pin = USARTx_RX_PIN | USARTx_TX_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		HAL_NVIC_EnableIRQ(USART3_IRQn);
	}
}
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure USART6 Tx as alternate function  */
  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  /* Configure USART6 Rx as alternate function  */
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);
}
  
BOOL CPU_USART_Initialize( int ComPortNum, int BaudRate, int Parity, int DataBits, int StopBits, int FlowValue )
{
	GLOBAL_LOCK(irq);
	
	switch (ComPortNum) { 
		case 0:
			UsartHandle.Instance = LPUART1;
			break;
		case 1:
			UsartHandle.Instance = USART2;
			break;
		case 2:
			UsartHandle.Instance = USART3;
			break;
		case 3:
			UsartHandle.Instance = UART4;
			break;
		case 4:
			UsartHandle.Instance = UART5;
			break;
		case 5:
			UsartHandle.Instance = USART6;
			break;
		case 6:
			UsartHandle.Instance = UART7;
			break;
		case 7:
			UsartHandle.Instance = UART8;
			break;
	}
	
	UsartHandle.Init.BaudRate = BaudRate;
	UsartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	
	if (StopBits == USART_STOP_BITS_NONE) UsartHandle.Init.StopBits = UART_STOPBITS_0_5;
	else if (StopBits == USART_STOP_BITS_ONE) UsartHandle.Init.StopBits = UART_STOPBITS_1;
	else if (StopBits == USART_STOP_BITS_TWO) UsartHandle.Init.StopBits = UART_STOPBITS_2;
	else if (StopBits == USART_STOP_BITS_ONEPOINTFIVE) UsartHandle.Init.StopBits = UART_STOPBITS_1_5;
	 
	if (Parity == USART_PARITY_NONE) UsartHandle.Init.Parity = UART_PARITY_NONE;
	else if (Parity == USART_PARITY_ODD) UsartHandle.Init.Parity = UART_PARITY_ODD;
	else if (Parity == USART_PARITY_EVEN) UsartHandle.Init.Parity = UART_PARITY_EVEN;
	
	UsartHandle.Init.Mode = UART_MODE_TX_RX;
	if (FlowValue == 0 || FlowValue & USART_FLOW_NONE) UsartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	else if (FlowValue & USART_FLOW_HW_IN_EN && FlowValue & USART_FLOW_HW_OUT_EN) UsartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
	else if (FlowValue & USART_FLOW_HW_OUT_EN) UsartHandle.Init.HwFlowCtl = UART_HWCONTROL_CTS;
	else if (FlowValue & USART_FLOW_HW_IN_EN) UsartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS;

	UsartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	UsartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UsartHandle.Init.Prescaler = UART_PRESCALER_DIV1;
	UsartHandle.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	UsartHandle.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	UsartHandle.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	UsartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	if(HAL_UART_DeInit(&UsartHandle) != HAL_OK)
	{
		Error_Handler();
	}  
  
	if (HAL_UART_Init(&UsartHandle) != HAL_OK)
	{
		Error_Handler();
    }
    return TRUE;
}

BOOL CPU_USART_Uninitialize( int ComPortNum )
{
	GLOBAL_LOCK(irq);
	
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
	if(HAL_UART_Transmit_IT(&UsartHandle, (uint8_t*)aTxBuffer, TXBUFFERSIZE)!= HAL_OK)
	{
		Error_Handler();   
	}
	
}

// FIXME
// Only working well enough for hal_printf() etc for now
void CPU_USART_TxBufferEmptyInterruptEnable( int ComPortNum, BOOL Enable ) {
	if ( !(ComPortNum == 2 && Enable == TRUE) ) { __BKPT(); return; }
	UART_HandleTypeDef *huart = &UsartHandle;

	// We are going to cheat and poll to send all the bytes
	char c;
	BOOL ret = USART_RemoveCharFromTxBuffer(ComPortNum, c);

	while (ret == TRUE) {
		while( __HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) == RESET ); // Spin
		huart->Instance->TDR = (uint8_t)c & (uint8_t)0xFFU;
		ret = USART_RemoveCharFromTxBuffer(ComPortNum, c);
	}
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
	__BKPT();
}

void Debug_Print_in_HAL(const char* format){
	hal_printf(format);
}
