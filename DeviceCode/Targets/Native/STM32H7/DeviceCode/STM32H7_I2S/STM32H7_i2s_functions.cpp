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
#include "..\stm32h7xx.h"
#include "..\stm32h7xx_main.h"

I2S_HandleTypeDef I2SHandle;

BOOL I2S_Internal_Initialize()
{
	
}

BOOL I2S_Internal_Uninitialize()
{
  
}

void I2S_Internal_XActionStart( I2S_HAL_XACTION* xAction, bool repeatedStart )
{
 
}

void I2S_Internal_XActionStop()
{
  
}

void I2S_Internal_GetClockRate( UINT32 rateKhz, UINT8& clockRate, UINT8& clockRate2)
{
  
}

void I2S_Internal_GetPins(GPIO_PIN& scl, GPIO_PIN& sda)
{
}

