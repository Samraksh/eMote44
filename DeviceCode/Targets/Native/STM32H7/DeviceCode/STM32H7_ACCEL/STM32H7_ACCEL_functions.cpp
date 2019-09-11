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


/*!
 * @brief Initializes the Acclerometer object and MCU peripheral
 *
 * @param [IN] none
 */
void CPU_ACCEL_Init()
{  	
	GPIO_InitTypeDef initStruct={0};
	
	initStruct.Mode = GPIO_MODE_OUTPUT_PP;
	initStruct.Pull = GPIO_NOPULL;
	initStruct.Speed = GPIO_SPEED_LOW;
	//initStruct.Alternate = GPIO_AF5_SPI1 ;

	CPU_GPIO_Write( ACCEL_CS_PORT, ACCEL_CS_PIN, 1 );
	CPU_GPIO_Init( ACCEL_CS_PORT, ACCEL_CS_PIN, &initStruct);
	
	CPU_GPIO_Init( ACCEL_INT_1_PORT, ACCEL_INT_1_PIN, &initStruct);
	CPU_GPIO_Init( ACCEL_INT_2_PORT, ACCEL_INT_2_PIN, &initStruct);

}

/*!
 * @brief De-initializes the Acclerometer object and MCU peripheral
 *
 * @param [IN] none
 */
void CPU_ACCEL_DeInit()
{

}