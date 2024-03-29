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
//#include "..\stm32h7xx_main.h"

UART_HandleTypeDef UsartHandle[8];

static void Error_Handler(void);  

extern "C" void USART3_IRQHandler(void)
{
 INTERRUPT_START;
 if (UsartHandle[2].Instance->ISR & USART_ISR_RXNE_RXFNE) {
    char c = (char)READ_REG(UsartHandle[2].Instance->RDR); // read RX data
    USART_AddCharToRxBuffer(2, c);
 }

 if (UsartHandle[2].Instance->ISR & USART_ISR_TXE_TXFNF) {

    char c;
    if (USART_RemoveCharFromTxBuffer(2, c)) {
        UsartHandle[2].Instance->TDR = c;  // write TX data
    } else {
        UsartHandle[2].Instance->CR1 &= ~USART_CR1_TXEIE; // TX int disable
    }
 }
 INTERRUPT_END;
  /* USER CODE BEGIN OTG_FS_IRQn 0 */
   //hal_printf(" 32 USART3_IRQHandler.cpp \n");
  /* USER CODE END OTG_FS_IRQn 0 */
  //HAL_UART_IRQHandler(&UsartHandle[2]);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */
   //hal_printf(" 35 USART3_IRQHandler.cpp \n");
  /* USER CODE END OTG_FS_IRQn 1 */
}

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
		GPIO_InitStruct.Pin = USARTx_RX_PIN | USARTx_TX_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		//hal_printf(" 33 stm32h7xx_hal_msp.cpp \n");
				
		//__NVIC_SetVector(USART3_IRQn, (uint32_t)USART3_IRQHandler);
		//HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART3_IRQn);
  
	}
}
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  HAL_NVIC_DisableIRQ(USART3_IRQn);
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
			UsartHandle[ComPortNum].Instance = LPUART1;
			break;
		case 1:
			UsartHandle[ComPortNum].Instance = USART2;
			break;
		case 2:
			UsartHandle[ComPortNum].Instance = USART3;			
			break;
		case 3:
			UsartHandle[ComPortNum].Instance = UART4;
			break;
		case 4:
			UsartHandle[ComPortNum].Instance = UART5;
			break;
		case 5:
			UsartHandle[ComPortNum].Instance = USART6;
			break;
		case 6:
			UsartHandle[ComPortNum].Instance = UART7;
			break;
		case 7:
			UsartHandle[ComPortNum].Instance = UART8;
			break;
	}
	
	UsartHandle[ComPortNum].Init.BaudRate = BaudRate;
	UsartHandle[ComPortNum].Init.WordLength = UART_WORDLENGTH_8B;
	
	if (StopBits == USART_STOP_BITS_NONE) UsartHandle[ComPortNum].Init.StopBits = UART_STOPBITS_0_5;
	else if (StopBits == USART_STOP_BITS_ONE) UsartHandle[ComPortNum].Init.StopBits = UART_STOPBITS_1;
	else if (StopBits == USART_STOP_BITS_TWO) UsartHandle[ComPortNum].Init.StopBits = UART_STOPBITS_2;
	else if (StopBits == USART_STOP_BITS_ONEPOINTFIVE) UsartHandle[ComPortNum].Init.StopBits = UART_STOPBITS_1_5;
	 
	if (Parity == USART_PARITY_NONE) UsartHandle[ComPortNum].Init.Parity = UART_PARITY_NONE;
	else if (Parity == USART_PARITY_ODD) UsartHandle[ComPortNum].Init.Parity = UART_PARITY_ODD;
	else if (Parity == USART_PARITY_EVEN) UsartHandle[ComPortNum].Init.Parity = UART_PARITY_EVEN;
	
	UsartHandle[ComPortNum].Init.Mode = UART_MODE_TX_RX;
	if (FlowValue == 0 || FlowValue & USART_FLOW_NONE) UsartHandle[ComPortNum].Init.HwFlowCtl = UART_HWCONTROL_NONE;
	else if (FlowValue & USART_FLOW_HW_IN_EN && FlowValue & USART_FLOW_HW_OUT_EN) UsartHandle[ComPortNum].Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
	else if (FlowValue & USART_FLOW_HW_OUT_EN) UsartHandle[ComPortNum].Init.HwFlowCtl = UART_HWCONTROL_CTS;
	else if (FlowValue & USART_FLOW_HW_IN_EN) UsartHandle[ComPortNum].Init.HwFlowCtl = UART_HWCONTROL_RTS;

	UsartHandle[ComPortNum].Init.OverSampling = UART_OVERSAMPLING_16;
	//  UsartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	//  UsartHandle.Init.Prescaler = UART_PRESCALER_DIV1;
	//  UsartHandle.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	//  UsartHandle.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	//  UsartHandle.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	UsartHandle[ComPortNum].AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	if(HAL_UART_DeInit(&UsartHandle[ComPortNum]) != HAL_OK)
	{
		Error_Handler();
	}  
  
	if (HAL_UART_Init(&UsartHandle[ComPortNum]) != HAL_OK)
	{
		Error_Handler();
    }
	// Enable RX interrupt
	SET_BIT(UsartHandle[ComPortNum].Instance->CR1, USART_CR1_RXNEIE);
    return TRUE;
}

BOOL CPU_USART_Uninitialize( int ComPortNum )
{
	GLOBAL_LOCK(irq);
	
	if(HAL_UART_DeInit(&UsartHandle[ComPortNum]) != HAL_OK)
	{
		Error_Handler();
	}  
    
    return TRUE;
}

BOOL CPU_USART_TxBufferEmpty( int ComPortNum )
{
	if (UsartHandle[ComPortNum].Instance->ISR & USART_ISR_TXE_TXFNF)
		return TRUE;
    return FALSE;
}

BOOL CPU_USART_TxShiftRegisterEmpty( int ComPortNum )
{
    if (UsartHandle[ComPortNum].Instance->ISR & USART_ISR_TC)
		return TRUE;
    return FALSE;
}

void CPU_USART_WriteCharToTxBuffer( int ComPortNum, UINT8 c )
{
	#ifdef DEBUG
	ASSERT(CPU_USART_TxBufferEmpty(ComPortNum));
	#endif
    UsartHandle[2].Instance->TDR = c;	
}

void CPU_USART_TxBufferEmptyInterruptEnable( int ComPortNum, BOOL Enable )
{
	if (Enable) 
    {
		SET_BIT(UsartHandle[ComPortNum].Instance->CR1, USART_CR1_TXEIE);
	}
	else
	{
		CLEAR_BIT(UsartHandle[ComPortNum].Instance->CR1, USART_CR1_TXEIE);
	}
}

BOOL CPU_USART_TxBufferEmptyInterruptState( int ComPortNum )
{
	if (UsartHandle[ComPortNum].Instance->CR1 & USART_CR1_TXEIE)
        return TRUE;

    return FALSE;
}

void CPU_USART_RxBufferFullInterruptEnable( int ComPortNum, BOOL Enable )
{
	if (Enable) 
    {
		SET_BIT(UsartHandle[ComPortNum].Instance->CR1, USART_CR1_RXNEIE);
	}
	else
	{
		CLEAR_BIT(UsartHandle[ComPortNum].Instance->CR1, USART_CR1_RXNEIE);
	}
}

BOOL CPU_USART_RxBufferFullInterruptState( int ComPortNum )
{
    if (UsartHandle[ComPortNum].Instance->CR1 & USART_CR1_RXNEIE)
        return TRUE;

    return FALSE;
}

BOOL CPU_USART_TxHandshakeEnabledState( int comPort )
{
    return TRUE;
}

void CPU_USART_ProtectPins( int ComPortNum, BOOL On )
{
	if (On)
    {
        CPU_USART_RxBufferFullInterruptEnable(ComPortNum, FALSE);
        CPU_USART_TxBufferEmptyInterruptEnable(ComPortNum, FALSE);
    }
    else
    {
        CPU_USART_TxBufferEmptyInterruptEnable(ComPortNum, TRUE);
        CPU_USART_RxBufferFullInterruptEnable(ComPortNum, TRUE);
    }
}

UINT32 CPU_USART_PortsCount()
{
     return TOTAL_USART_PORT;
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

void Debug_Print_in_HAL(const char* format){
	hal_printf(format);
}
