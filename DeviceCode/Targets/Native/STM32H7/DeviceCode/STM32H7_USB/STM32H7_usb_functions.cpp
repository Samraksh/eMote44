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

#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
/**
  * @brief  This function handles USB-On-The-Go FS/HS global interrupt request.
  * @param  None
  * @retval None
  */
  
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
 

extern "C" void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

USBD_HandleTypeDef hUsbDeviceFS;

USB_CONTROLLER_STATE * CPU_USB_GetState( int Controller )
{
}

HRESULT CPU_USB_Initialize( int Controller )
{
	//GLOBAL_LOCK (irq);
	
	/* Init Device Library */
	//hal_printf(" 48 USB.cpp \n");
	//HAL_Delay(10u);
	USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);

	/* Add Supported Class */
	//hal_printf(" 52 USB.cpp \n");
	USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);

	/* Add CDC Interface Class */
	//hal_printf(" 56 USB.cpp \n");
	USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);

	/* Start Device Process */
	//hal_printf(" 60 USB.cpp \n");
	USBD_Start(&hUsbDeviceFS);

	HAL_PWREx_EnableUSBVoltageDetector();
	
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
	//hal_printf(format);
}

void *USBD_static_malloc(uint32_t size)
{
  static uint8_t mem[sizeof(USBD_CDC_HandleTypeDef)];
  return mem;
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}