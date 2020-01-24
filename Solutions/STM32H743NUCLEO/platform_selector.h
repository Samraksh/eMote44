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

// Add pause after reset, otherwise JTAG reset will have some fly-through
// Can remove for production
#define STARTUP_DELAY_MS 200

//
// Processor and features
//
/////////////////////////////////////////////////////////

//#define __MAC_CSMA__
#define __MAC_OMAC__

#define __RADIO_SX1276__
//#define __RADIO_SI4468__
//#define __RADIO_RF231__
/////////////////////////////////////////////////////////
//
// Constants
//

// System Clock
//#define SYSTEM_CLOCK_HZ                  400000000  // 400 MHz
//#define SYSTEM_CYCLE_CLOCK_HZ            400000000  // 400 MHz
#define SYSTEM_CLOCK_HZ                  200000000  // 200 MHz
#define SYSTEM_CYCLE_CLOCK_HZ            200000000  // 200 MHz
#define SYSTEM_APB1_CLOCK_HZ             25000000  // 25 MHz
#define SYSTEM_APB2_CLOCK_HZ             50000000  // 50 MHz


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

// 00000000|00000000|TTTTTTTT|cccppppp| ( transport == USB_TRANSPORT )
//    |--------+--------+--------+--------|
// 
// where:
//    T => Transport type
//              USART_TRANSPORT => 1
//                USB_TRANSPORT => 2
//             SOCKET_TRANSPORT => 3
//              DEBUG_TRANSPORT => 4
//                LCD_TRANSPORT => 5
//        FLASH_WRITE_TRANSPORT => 6
//          MESSAGING_TRANSPORT => 7
//            GENERIC_TRANSPORT => 8
//    p => port instance number 
//        Port instances in the handle are 1 based. (e.g. p == 0 is invalid except when T == 0 )
//    c -> Controller instance number ( USB_TRANSPORT only )
#define USB_SERIAL_PORT			0x221 // USB transport, controller instance 1, port 1

#define DEBUG_TEXT_PORT                 USB_SERIAL_PORT
#define STDIO                           USB_SERIAL_PORT
#define DEBUGGER_PORT                   USB_SERIAL_PORT
#define MESSAGING_PORT                  USB_SERIAL_PORT

#define USART_DEFAULT_PORT              4 // COM3
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
#define TOTAL_USART_PORT                8
//                                         "COM1"    "COM2"    "COM3"
//                                         USART1    USART2    USART3
//#define STM32H7_UART_TXD_PINS           { _P_NONE_, _P(A, 2), _P(D, 8) }
//#define STM32H7_UART_RXD_PINS           { _P_NONE_, _P(A, 3), _P(D, 9) }
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
#define LED2                            _P(B, 1)  // Blue
#define LED3                            _P(B, 2)  // Red

//#define GPIO_0 _P(A,3)
//#define GPIO_1 _P(C,2)
//#define GPIO_2 _P(B,1)
//#define GPIO_3 _P(F,3)
//#define GPIO_4 _P(F,4)
//#define GPIO_5 _P(B,6)

#define DISABLED_PIN	(GPIO_PIN)120

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
const UINT8 OMACClockSpecifier = RTC_32BIT; //??

const UINT8 g_CountOfHardwareTimers = 2;
const UINT8 g_HardwareTimerIDs[g_CountOfHardwareTimers] = { DEFAULT_TIMER, LOW_DRIFT_TIMER };
const UINT8 g_VirtualTimerPerHardwareTimer = 40;
//const UINT32 g_HardwareTimerFrequency[g_CountOfHardwareTimers] = { 48000000, 32768};
const UINT32 g_HardwareTimerFrequency[g_CountOfHardwareTimers] = { SYSTEM_APB1_CLOCK_HZ * 2, 32768 };

// timers that are run within interrupt context
#define VIRT_TIMER_EVENTS 			1
#define VIRT_TIMER_REALTIME 		2
#define VIRT_TIMER_OMAC_SCHEDULER	3

#define VIRT_TIMER_OMAC_RECEIVER_ACK 	5
#define VIRT_TIMER_SLEEP 6
// The following definition will be used within the code as the decision point in deciding if the timer is to be run within interrupt context or continuation
// Adjust this marker appropriately ( <= marker is interrupt context, > marker is continuation)
#define VIRT_TIMER_INTERRUPT_CONTEXT_MARKER 6

#define VIRT_TIMER_SX1276_PacketLoadTimerName 7
#define VIRT_TIMER_SX1276_PacketTxTimerName 8
#define VIRT_TIMER_SX1276_CADTimer 9

#define VIRT_TIMER_TIME 			10

#define VIRT_TIMER_TIME_TEST	32
#define VIRT_TIMER_RTC_TEST	37

// timers that are run within continuations (all C# user timers are run outside an interrupt context also)
#define VIRT_TIMER_LED_GREEN 		38	
#define VIRT_TIMER_LED_RED 			39

#define VIRT_TX_TIMEOUT_TIMER 12
#define VIRT_RX_TIMEOUT_TIMER 13
#define VIRT_RX_TIMEOUT_SYNC_WORD 14

#define VIRT_TIMER_MAC_SENDPKT 15
#define VIRT_TIMER_MAC_BEACON 16
#define VIRT_TIMER_MAC_FLUSHBUFFER 17
//#define VIRT_TIMER_OMAC_SCHEDULER 18

#define LocalClockMonitor_TIMER1 19

#define VIRT_TIMER_OMAC_SCHEDULER_FAILSAFE 20
#define VIRT_TIMER_OMAC_SCHEDULER_RADIO_STOP_RETRY 21
#define VIRT_TIMER_OMAC_RECEIVER 22
//#define VIRT_TIMER_OMAC_RECEIVER_ACK 23
#define VIRT_TIMER_OMAC_DISCOVERY 24
#define VIRT_TIMER_OMAC_TRANSMITTER 25
#define VIRT_TIMER_OMAC_TIMESYNC 26

#define VIRT_CONT_TEST_TIMER1 33
#define VIRT_CONT_TEST_TIMER2 34

#define VIRT_CONT_TEST_TIMER1 35
#define VIRT_CONT_TEST_TIMER2 36


#if defined(__MAC_OMAC__)
#define NEIGHBORCLOCKMONITORPIN 			DISABLED_PIN
#define LOCALCLOCKMONITORPIN 				DISABLED_PIN



#define DATATX_NEXT_EVENT					DISABLED_PIN//22
#define DATARX_NEXT_EVENT					DISABLED_PIN//23
#define DATATX_DATA_PIN						DISABLED_PIN//24//22
#define DATARX_HANDLE_END_OF_RX				DISABLED_PIN
#define OMAC_CONTINUATION					DISABLED_PIN//23
#define DATARX_SEND_SW_ACK					DISABLED_PIN//23//22
#define OMAC_DRIVING_RADIO_SEND				DISABLED_PIN//22
#define DATARX_EXEC_EVENT					DISABLED_PIN//22//23
#define SI4468_HANDLE_SLEEP					DISABLED_PIN//0//23
//#define SCHED_RX_EXEC_PIN 					(GPIO_PIN)06171015//22 //THis is a duplicate definition
#define OMAC_DISCO_EXEC_EVENT				DISABLED_PIN//23//0
#define EMOTE_NET_MAC_GETNEXTPACKET			DISABLED_PIN
#define EMOTE_NET_MANAGED_CALLBACK			DISABLED_PIN//0
#define OMAC_DATARXPIN						DISABLED_PIN//0
#define OMAC_DRIVING_RADIO_RECV				DISABLED_PIN//22//23
#define OMAC_DRIVING_RADIO_SLEEP			DISABLED_PIN//22//0
#define OMAC_SCHED_POST_POST_EXEC			DISABLED_PIN//22//0
#define OMAC_DISCO_POST_EXEC				DISABLED_PIN//23//0
#define DATATX_RECV_SW_ACK					DISABLED_PIN//23
#define OMAC_TIMESYNC_NEXT_EVENT			DISABLED_PIN//24
#define DISCO_NEXT_EVENT					DISABLED_PIN
#define DISCO_BEACON_N						DISABLED_PIN//25
#define SCHED_NEXT_EVENT					DISABLED_PIN//0
#define OMAC_DISCO_BEACON_ACK_HANDLER_PIN   DISABLED_PIN 
#define OMAC_DISCO_BEACONNTIMERHANDLER_PIN  DISABLED_PIN

#define OMAC_TESTING_SCHEDULER_PIN 			DISABLED_PIN//24
#define OMAC_TESTING_VTIMER_PIN 			DISABLED_PIN
#define VT_CALLBACK 						DISABLED_PIN					//J11_PIN7


//Acknowledgements from radio
#define SEND_ACK_PIN						DISABLED_PIN //29//120
#define DATA_RX_INTERRUPT_PIN				DISABLED_PIN //120
#define DATA_TX_ACK_PIN						DISABLED_PIN //120

//Acknowledgements from other node
#define OMAC_TX_DATAACK_PIN 				DISABLED_PIN //120 //120 //23  //120							//J11_pin3 0
#define OMAC_RX_DATAACK_PIN 				DISABLED_PIN //120 // 120 //23  //120							//J11_pin3 0

//Radio Control
#define RADIOCONTROL_STATEPIN 				DISABLED_PIN //120 //23 //120 // 120 //120 				//J11_pin6 //This 	(GPIO_PIN)3  did not work

//TX Related
#define RADIOCONTROL_SEND_PIN 				DISABLED_PIN 				//J11_pin4
#define RADIOCONTROL_SENDTS_PIN 			DISABLED_PIN //(<--2) 				//J11_pin5
#define DISCO_SYNCSENDPIN 					DISABLED_PIN // 24  //120						//J12_PIN1
#define DATATX_PIN 							DISABLED_PIN //(<--2) //29 //120 //120 //2							//J12_PIN3
#define RC_TX_TIMESYNCREQ 					DISABLED_PIN
#define RC_TX_DATA 							DISABLED_PIN	//29 (<--29)
//#define DATATX_DATA_PIN				DISABLED_PIN //25
#define DATARX_TIMESTAMP_PIN				DISABLED_PIN //29
#define DATATX_POSTEXEC						DISABLED_PIN
#define DATATX_RECV_HW_ACK					DISABLED_PIN //(<--4)
#define FAST_RECOVERY_SEND					DISABLED_PIN //(<--0)
#define DATATX_SEND_ACK_HANDLER				DISABLED_PIN
#define OMAC_RADIOCONTROL_RADIO_SEND_TOGGLER			DISABLED_PIN //SI4468_Radio_TX_Instance //DISABLED_PIN
#define DATATX_SendACKHandler_PIN_TOGGLER				DISABLED_PIN //(GPIO_PIN)22 //SCHED_TX_EXEC_PIN //DISABLED_PIN
#define DATATX_ReceiveDATAACK_PIN_TOGGLER				DISABLED_PIN //DISABLED_PIN
#define DATATX_CCA_PIN_TOGGLER				DISABLED_PIN
#define DATATX_TIMING_ERROR_PIN_TOGGLER 	DISABLED_PIN
#define DTH_STATE_PIN_TOGGLER				DISABLED_PIN //SCHED_TX_EXEC_PIN

//RX related
#define OMAC_RXPIN 							DISABLED_PIN //(GPIO_PIN)0 //120 //23  //120							//J11_pin3 0
//#define OMAC_DATARXPIN 	DISABLED_PIN //0 //26	//120 //2					//J12_pin5
#define OMAC_TIMESYNCREQRXPIN 				DISABLED_PIN //23 //30
#define DISCO_SYNCRECEIVEPIN 				DISABLED_PIN //25 //120					//J12_PIN2
#define DATARECEPTION_SLOTPIN 				DISABLED_PIN //30 //31 //2				//J12_PIN4
#define DATARECEPTION_RADIOONPIN 			DISABLED_PIN //30 //31 //2				//J12_PIN4
#define DATARX_TIMING_ERROR_PIN_TOGGLER 	DISABLED_PIN

//Timesync related
#define TIMESYNC_GENERATE_MESSAGEPIN 		DISABLED_PIN
#define TIMESYNC_RECEIVEPIN 				DISABLED_PIN //24 			//J12_pin5

//Scheduler Related
#define SCHED_START_STOP_PIN 				DISABLED_PIN //4
#define SCHED_RX_EXEC_PIN 				    DISABLED_PIN //(GPIO_PIN)25 //25 //4
#define SCHED_TX_EXEC_PIN 					DISABLED_PIN //(GPIO_PIN)24 //24 //4
#define SCHED_DISCO_EXEC_PIN 				DISABLED_PIN //0 //4
#define SCHED_TSREQ_EXEC_PIN 				DISABLED_PIN //DISABLED_PIN //23 //4
#define DATARX_NEXTEVENT 					DISABLED_PIN
#define DATATX_NEXTEVENT 					DISABLED_PIN
#define DATATX_SCHED_DATA_PKT 				DISABLED_PIN
#define DATA_RX_END_OF_RECEPTION			DISABLED_PIN

//MISC
#define DELAY_BETWEEN_DATATX_SCHED_ACTUAL	DISABLED_PIN	//J12_pin5
#define OMAC_DEBUG_PIN 						DISABLED_PIN			 			//J11_PIN5
#define VTIMER_CALLBACK_LATENCY_PIN			DISABLED_PIN //(<--31)


#define RX_RADIO_TURN_ON 					DISABLED_PIN //_P(D,3)//
#define RX_RADIO_TURN_OFF					DISABLED_PIN //_P(D,5)//


#endif //__MAC_OMAC__


/* Definition for USARTx Pins */
#define USART2_TX_PIN               GPIO_PIN_2
#define USART2_TX_GPIO_PORT         GPIOA
#define USART2_TX_AF                GPIO_AF7_USART2
#define USART2_RX_PIN               GPIO_PIN_6
#define USART2_RX_GPIO_PORT         GPIOD
#define USART2_RX_AF                GPIO_AF7_USART2

#define USART3_TX_PIN               GPIO_PIN_8
#define USART3_TX_GPIO_PORT         GPIOD
#define USART3_TX_AF                GPIO_AF7_USART3
#define USART3_RX_PIN               GPIO_PIN_9
#define USART3_RX_GPIO_PORT         GPIOD
#define USART3_RX_AF                GPIO_AF7_USART3

#define UART5_TX_PIN               GPIO_PIN_13
#define UART5_TX_GPIO_PORT         GPIOB
#define UART5_TX_AF                GPIO_AF14_UART5
#define UART5_RX_PIN               GPIO_PIN_12
#define UART5_RX_GPIO_PORT         GPIOB
#define UART5_RX_AF                GPIO_AF14_UART5

/*!
 * \brief GPIOs Macro
 */

#define RCC_GPIO_CLK_ENABLE( __GPIO_PORT__ )              \
do {                                                    \
    switch( __GPIO_PORT__)                                \
    {                                                     \
      case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_ENABLE(); break;    \
      case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_ENABLE(); break;    \
      case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_ENABLE(); break;    \
	  case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_ENABLE(); break;    \
	  case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_ENABLE(); break;    \
	  case GPIOF_BASE: __HAL_RCC_GPIOF_CLK_ENABLE(); break;    \
	  case GPIOG_BASE: __HAL_RCC_GPIOG_CLK_ENABLE(); break;    \
      case GPIOH_BASE: default:  __HAL_RCC_GPIOH_CLK_ENABLE(); \
    }                                                    \
  } while(0)  

#define RCC_GPIO_CLK_DISABLE( __GPIO_PORT__ )              \
do {                                                    \
    switch( __GPIO_PORT__)                                \
    {                                                     \
      case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_DISABLE(); break;    \
      case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_DISABLE(); break;    \
	  case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_DISABLE(); break;    \
      case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_DISABLE(); break;    \
	  case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_DISABLE(); break;    \
	  case GPIOF_BASE: __HAL_RCC_GPIOF_CLK_DISABLE(); break;    \
	  case GPIOG_BASE: __HAL_RCC_GPIOG_CLK_DISABLE(); break;    \
      case GPIOH_BASE: default:  __HAL_RCC_GPIOH_CLK_DISABLE(); \
    }                                                    \
  } while(0) 

#define RADIO_LORA 1
#define RADIO_SI446X 0
#define RADIO_RF231 0

// Definition for LoRa I/O
#ifdef RADIO_LORA

#define RADIO_RESET_PORT                          GPIOA
#define RADIO_RESET_PIN                           GPIO_PIN_8

#define RADIO_MOSI_PORT                           GPIOA
#define RADIO_MOSI_PIN                            GPIO_PIN_7

#define RADIO_MISO_PORT                           GPIOA
#define RADIO_MISO_PIN                            GPIO_PIN_6

#define RADIO_SCLK_PORT                           GPIOA
#define RADIO_SCLK_PIN                            GPIO_PIN_5

#define RADIO_NSS_PORT                            GPIOC
#define RADIO_NSS_PIN                             GPIO_PIN_1

#define RADIO_DIO_0_PORT                          GPIOB
#define RADIO_DIO_0_PIN                           GPIO_PIN_5

#define RADIO_DIO_1_PORT                          GPIOB
#define RADIO_DIO_1_PIN                           GPIO_PIN_6

#define RADIO_DIO_2_PORT                          GPIOB
#define RADIO_DIO_2_PIN                           GPIO_PIN_7

#define RADIO_DIO_3_PORT                          GPIOB
#define RADIO_DIO_3_PIN                           GPIO_PIN_8

#define RADIO_ANT_SWITCH_PORT              		  GPIOA
#define RADIO_ANT_SWITCH_PIN              		  GPIO_PIN_9

#endif

#define ACCEL_CS_PORT						GPIOG
#define ACCEL_CS_PIN						GPIO_PIN_6

#define ACCEL_INT_1_PORT					GPIOG
#define ACCEL_INT_1_PIN						GPIO_PIN_7

#define ACCEL_INT_2_PORT					GPIOG
#define ACCEL_INT_2_PIN						GPIO_PIN_8

/* Definition for SPIx clock resources */

#define SPIx                        SPI1
#define SPIx_CLK_ENABLE()           __HAL_RCC_SPI1_CLK_ENABLE()
#define DMAx_CLK_ENABLE()           __HAL_RCC_DMA2_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define SPIx_FORCE_RESET()          __HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()        __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                GPIO_PIN_5
#define SPIx_SCK_GPIO_PORT          GPIOA
#define SPIx_SCK_AF                 GPIO_AF5_SPI1
#define SPIx_MISO_PIN               GPIO_PIN_6
#define SPIx_MISO_GPIO_PORT         GPIOA
#define SPIx_MISO_AF                GPIO_AF5_SPI1
#define SPIx_MOSI_PIN               GPIO_PIN_7
#define SPIx_MOSI_GPIO_PORT         GPIOA
#define SPIx_MOSI_AF                GPIO_AF5_SPI1

/* Definition for SPIx's NVIC */
#define SPIx_IRQn                        SPI1_IRQn
#define SPIx_IRQHandler                  SPI1_IRQHandler

#define OMAC_DISCO_SEQ_NUMBER			97

//#define BUFFERSIZE                       (COUNTOF(aTxBuffer2) - 1)
/* Size of Trasmission buffer */
//#define TXBUFFERSIZE                (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
//#define RXBUFFERSIZE                TXBUFFERSIZE
  
/* Exported macro ------------------------------------------------------------*/
//#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */

//
// Constants
/////////////////////////////////////////////////////////
#include <processor_selector.h>
#include <stm32h7xx_hal.h> 
#endif // PLATFORM_ARM_STM32H743NUCLEO
