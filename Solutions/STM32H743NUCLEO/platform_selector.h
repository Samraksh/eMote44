/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file is part of the Microsoft .NET Micro Framework Porting Kit Code Samples and is unsupported.
// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use these files except in compliance with the License.
// You may obtain a copy of the License at:
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing
// permissions and limitations under the License.
// 
// Based on the Implementation for (STM32F4) by Oberon microsystems, Inc.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////
//
// Processor and features
//

#if defined(PLATFORM_ARM_STM32H743NUCLEO)

#define HAL_SYSTEM_NAME "STM32H743NUCLEO"

#define PLATFORM_ARM_STM32H7

#define STM32H743xx

//
// Processor and features
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
// Constants
//

// System Clock
//#define SYSTEM_CLOCK_HZ                  400000000  // 400 MHz
//#define SYSTEM_CYCLE_CLOCK_HZ            400000000  // 400 MHz
#define SYSTEM_CLOCK_HZ                  96000000  // 96 MHz
#define SYSTEM_CYCLE_CLOCK_HZ            96000000  // 96 MHz
#define SYSTEM_APB1_CLOCK_HZ             100000000  // 100 MHz
#define SYSTEM_APB2_CLOCK_HZ             100000000  // `00 MHz


// System Clock
//#define SYSTEM_CLOCK_HZ                  192000000  // 192 MHz
//#define SYSTEM_CYCLE_CLOCK_HZ            192000000  // 192 MHz
//#define SYSTEM_APB1_CLOCK_HZ              48000000  //  48 MHz
//#define SYSTEM_APB2_CLOCK_HZ              96000000  //  96 MHz

// Maximum frequency
//#define SYSTEM_CLOCK_HZ                  216000000  // 216 MHz
//#define SYSTEM_CYCLE_CLOCK_HZ            216000000  // 216 MHz
//#define SYSTEM_APB1_CLOCK_HZ              54000000  //  54 MHz
//#define SYSTEM_APB2_CLOCK_HZ             108000000  // 108 MHz

// FIXME: HSEBYP is not currently supported
#define SYSTEM_CRYSTAL_CLOCK_HZ            8000000  // 8 MHz external clock (MCO from ST-LINK)

#define CLOCK_COMMON_FACTOR                1000000  // GCD(SYSTEM_CLOCK_HZ, 1M)

#define SLOW_CLOCKS_PER_SECOND             1000000  // 1 MHz
#define SLOW_CLOCKS_TEN_MHZ_GCD            1000000  // GCD(SLOW_CLOCKS_PER_SECOND, 10M)
#define SLOW_CLOCKS_MILLISECOND_GCD           1000  // GCD(SLOW_CLOCKS_PER_SECOND, 1k)

// Memory
#define FLASH_MEMORY_Base               0x08000000
#define FLASH_MEMORY_Size               0x00100000  // 2M
#define SRAM1_MEMORY_Base               0x20000000
#define SRAM1_MEMORY_Size               0x00020000  // 128K
//DTCMRAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 128K
//RAM_D1 (xrw)      : ORIGIN = 0x24000000, LENGTH = 512K
//RAM_D2 (xrw)      : ORIGIN = 0x30000000, LENGTH = 288K
//RAM_D3 (xrw)      : ORIGIN = 0x38000000, LENGTH = 64K
//FLASH (rx)       : ORIGIN = 0x8000000, LENGTH = 2048K



// Memory
//#define FLASH_MEMORY_Base               0x08000000
//#define FLASH_MEMORY_Size               0x00100000  // 1M
//#define SRAM1_MEMORY_Base               0x20000000
//#define SRAM1_MEMORY_Size               0x00050000  // 320K
//      DTCM               0x20000000 - 0x2000FFFF  //  64K
//      SRAM1              0x20010000 - 0x2004BFFF  // 240K
//      SRAM2              0x2004C000 - 0x2004FFFF  //  16K

// System Configuration

#define SUPPLY_VOLTAGE_MV               3300  // 3.3V supply

#define TOTAL_GENERIC_PORTS             1 // ITM channel 0

#define TOTAL_USB_CONTROLLER            1
#define USB_MAX_QUEUES                  4 // 4 endpoints (EP0 + 3)
#define USB_ALLOW_CONFIGURATION_OVERRIDE 1

#define DEBUG_TEXT_PORT                 COM3
#define STDIO                           COM3
#define DEBUGGER_PORT                   COM3
#define MESSAGING_PORT                  COM3

#define USART_DEFAULT_PORT              2 // COM3
#define USART_DEFAULT_BAUDRATE          115200

// System Timer Configuration
#define STM32H7_32B_TIMER 2
#define STM32H7_16B_TIMER 3

// Peripherals and Pin Configuration

// General Purpose I/O
#define GPIO_PORTA  0
#define GPIO_PORTB  1
#define GPIO_PORTC  2
#define GPIO_PORTD  3
#define GPIO_PORTE  4
#define GPIO_PORTF  5
#define GPIO_PORTG  6
// Note: PH0 and PH1 intentionally omitted

#define TOTAL_GPIO_PORT                 (GPIO_PORTG + 1)
#define TOTAL_GPIO_PINS                 (TOTAL_GPIO_PORT*16)

#define PORT_PIN(port,pin)              (((int)port)*16 + (pin))
#define _P(port, pin)                   PORT_PIN(GPIO_PORT##port, pin)
#define _P_NONE_                        GPIO_PIN_NONE

// Serial
#define TOTAL_USART_PORT                3
//                                         "COM1"    "COM2"    "COM3"
//                                         USART1    USART2    USART3
#define STM32H7_UART_TXD_PINS           { _P_NONE_, _P(A, 2), _P(D, 8) }
#define STM32H7_UART_RXD_PINS           { _P_NONE_, _P(A, 3), _P(D, 9) }
// USART1 cannot be used: USART1_RX on PA10 conflicts with USB, PB7 conflicts with LED2
// USART3 is connected to ST-LINK (PD8, PD9) and available as a virtual COM port

#define STM32H7_SPI_SCLK_PINS           { _P(A, 5), _P_NONE_,_P_NONE_, _P_NONE_, _P(F, 7), _P_NONE_ }
#define STM32H7_SPI_MISO_PINS           { _P(A, 6), _P_NONE_,_P_NONE_, _P_NONE_, _P(F, 8), _P_NONE_ }
#define STM32H7_SPI_MOSI_PINS           { _P(A, 7), _P_NONE_,_P_NONE_, _P_NONE_, _P(F, 9), _P_NONE_ }

#define SPI_TYPE_RADIO				0


// User & Wake-up Button
#define USER_BUTTON                     _P(C,13)

// User LEDs
#define LED1                            _P(B, 0)  // Green
#define LED2                            _P(B, 7)  // Blue
#define LED3                            _P(B,14)  // Red

#define GPIO_0 _P(A,3)
#define GPIO_1 _P(C,2)
#define GPIO_2 _P(B,1)
#define GPIO_3 _P(F,3)
#define GPIO_4 _P(F,4)
#define GPIO_5 _P(B,6)

// TinyBooter entry using GPIO
#define TINYBOOTER_ENTRY_GPIO_PIN       USER_BUTTON               // 'User' button
#define TINYBOOTER_ENTRY_GPIO_STATE     TRUE                      // Active high
#define TINYBOOTER_ENTRY_GPIO_RESISTOR  RESISTOR_DISABLED         // No internal resistor, there is external pull-down (R58,R59)

const UINT8 SYSTEM_TIME = 0;
const UINT8 ADVTIMER_32BIT = 1;
const UINT8 DEFAULT_TIMER = ADVTIMER_32BIT;
const UINT8 TIMER_32BIT = ADVTIMER_32BIT;
const UINT8 RTC_32BIT = 4;
const UINT8 LOW_DRIFT_TIMER = RTC_32BIT;
const UINT8 VT_DEFAULT_TIMER = ADVTIMER_32BIT;

const UINT8 g_CountOfHardwareTimers = 2;
const UINT8 g_HardwareTimerIDs[g_CountOfHardwareTimers] = {SYSTEM_TIME, DEFAULT_TIMER };
const UINT8 g_VirtualTimerPerHardwareTimer = 16;
const UINT32 g_HardwareTimerFrequency[g_CountOfHardwareTimers] = {48000000, 48000000};

// timers that are run within interrupt context
#define VIRT_TIMER_EVENTS 			1
#define VIRT_TIMER_REALTIME 		2

// The following definition will be used within the code as the decision point in deciding if the timer is to be run within interrupt context or continuation
// Adjust this marker appropriately ( <= marker is interrupt context, > marker is continuation)
#define VIRT_TIMER_INTERRUPT_CONTEXT_MARKER 5

// timers that are run within continuations (all C# user timers are run outside an interrupt context also)
#define VIRT_TIMER_LED_GREEN 		10
#define VIRT_TIMER_LED_RED 			11

#define VIRT_TX_TIMEOUT_TIMER 12
#define VIRT_RX_TIMEOUT_TIMER 13
#define VIRT_RX_TIMEOUT_SYNC_WORD 14

//
// Constants
/////////////////////////////////////////////////////////

#include <processor_selector.h>

#endif // PLATFORM_ARM_STM32H743NUCLEO
