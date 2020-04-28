////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//  Implementation for the MCBSTM32F400 board (STM32F4): Copyright (c) Oberon microsystems, Inc.
//
//  *** STM32H743NUCLEO Board specific IO Port Initialization ***
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include <stm32h7xx.h>

// Define the generic port table, only one generic extensionn port type supported
// and that is the ITM hardware trace port on Channel 0.
//extern GenericPortTableEntry const Itm0GenericPort;
//extern GenericPortTableEntry const* const g_GenericPorts[TOTAL_GENERIC_PORTS] = { &Itm0GenericPort };


void BootstrapCode_GPIO()
{
    // Enable GPIO clocks for ports A - G
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN | RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIOCEN |
                    RCC_AHB4ENR_GPIODEN | RCC_AHB4ENR_GPIOEEN | RCC_AHB4ENR_GPIOFEN |
                    RCC_AHB4ENR_GPIOGEN;
}
