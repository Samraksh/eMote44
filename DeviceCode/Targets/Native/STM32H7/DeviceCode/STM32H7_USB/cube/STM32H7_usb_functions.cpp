////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//  Implementation for STM32F4: Copyright (c) Oberon microsystems, Inc.
//
//  *** USB OTG Full Speed Device Mode Driver ***
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <tinyhal.h>
#include <pal\com\usb\USB.h>

#include "STM32H7_usb_functions.h"
#include "usb_device.h"

USB_CONTROLLER_STATE * CPU_USB_GetState( int Controller )
{
}

HRESULT CPU_USB_Initialize( int Controller )
{
	GLOBAL_LOCK (irq);
	
	hal_printf("STM32H7_usb_functions.cpp 27");
	//HAL_Init();
	MX_USB_DEVICE_Init();
	hal_printf("STM32H7_usb_functions.cpp 29");
  
	return S_OK;
}

HRESULT CPU_USB_Uninitialize( int Controller )
{
	return S_OK;
}

BOOL CPU_USB_StartOutput( USB_CONTROLLER_STATE* State, int ep )
{
    return TRUE;
}

BOOL CPU_USB_RxEnable( USB_CONTROLLER_STATE* State, int ep )
{
	return TRUE;
}

BOOL CPU_USB_GetInterruptState( ) 
{
    return FALSE;
}

BOOL CPU_USB_ProtectPins( int Controller, BOOL On )
{
    return TRUE;
}

void Debug_Print_in_HAL(const char* format){
	hal_printf(format);
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}