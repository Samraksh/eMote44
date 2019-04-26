@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@ This file is part of the Microsoft .NET Micro Framework Porting Kit Code Samples and is unsupported. 
@@ Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
@@ 
@@ Licensed under the Apache License, Version 2.0 (the "License")@ you may not use these files except in compliance with the License.
@@ You may obtain a copy of the License at:
@@ 
@@ http://www.apache.org/licenses/LICENSE-2.0
@@ 
@@ Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
@@ WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing
@@ permissions and limitations under the License.
@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@
@@ STM32H743 Vector Table
@@

    .syntax unified
    .arch armv7-m

    @ Initial Stack pointer for power on reset
    .extern 	__initial_sp

    @ Import standard Cortex-M handlers
    .extern		Reset_Handler
    .extern		NMI_Handler
    .extern 	HardFault_Handler
    .extern 	MemManage_Handler
    .extern 	BusFault_Handler
    .extern 	UsageFault_Handler
    .extern 	SVC_Handler
    .extern 	DebugMon_Handler
    .extern 	PendSV_Handler
    .extern 	SysTick_Handler

    @ Import external interrupt handlers (SOC Specific)
	.extern     WWDG_IRQHandler                   @ Window WatchDog interrupt ( wwdg1_it)                                         
	.extern     PVD_AVD_IRQHandler                @ PVD/AVD through EXTI Line detection                        
	.extern     TAMP_STAMP_IRQHandler             @ Tamper and TimeStamps through the EXTI line            
	.extern     RTC_WKUP_IRQHandler               @ RTC Wakeup through the EXTI line                       
	.extern     FLASH_IRQHandler                  @ FLASH                                           
	.extern     RCC_IRQHandler                    @ RCC                                             
	.extern     EXTI0_IRQHandler                  @ EXTI Line0                                             
	.extern     EXTI1_IRQHandler                  @ EXTI Line1                                             
	.extern     EXTI2_IRQHandler                  @ EXTI Line2                                             
	.extern     EXTI3_IRQHandler                  @ EXTI Line3                                             
	.extern     EXTI4_IRQHandler                  @ EXTI Line4 
	.extern     DMA1_Stream0_IRQHandler           @ DMA1 Stream 0
	.extern     DMA1_Stream1_IRQHandler           @ DMA1 Stream 1                                   
	.extern     DMA1_Stream2_IRQHandler           @ DMA1 Stream 2                                   
	.extern     DMA1_Stream3_IRQHandler           @ DMA1 Stream 3                                   
	.extern     DMA1_Stream4_IRQHandler           @ DMA1 Stream 4                                   
	.extern     DMA1_Stream5_IRQHandler           @ DMA1 Stream 5                                   
	.extern     DMA1_Stream6_IRQHandler           @ DMA1 Stream 6  
	.extern     ADC_IRQHandler                    @ ADC1, ADC2                             
	.extern     FDCAN1_IT0_IRQHandler             @ FDCAN1 interrupt line 0                        
	.extern     FDCAN2_IT0_IRQHandler             @ FDCAN2 interrupt line 0                               
	.extern     FDCAN1_IT1_IRQHandler             @ FDCAN1 interrupt line 1                        
	.extern     FDCAN2_IT1_IRQHandler             @ FDCAN2 interrupt line 1                                               
	.extern     EXTI9_5_IRQHandler                @ External Line[9:5]s                                    
	.extern     TIM1_BRK_IRQHandler               @ TIM1 Break interrupt                   
	.extern     TIM1_UP_IRQHandler                @ TIM1 Update Interrupt                 
	.extern     TIM1_TRG_COM_IRQHandler           @ TIM1 Trigger and Commutation Interrupt 
	.extern     TIM1_CC_IRQHandler                @ TIM1 Capture Compare                                   
	.extern     TIM2_IRQHandler                   @ TIM2                                            
	.extern     TIM3_IRQHandler                   @ TIM3                                            
	.extern     TIM4_IRQHandler                   @ TIM4                                            
	.extern     I2C1_EV_IRQHandler                @ I2C1 Event                                             
	.extern     I2C1_ER_IRQHandler                @ I2C1 Error                                             
	.extern     I2C2_EV_IRQHandler                @ I2C2 Event                                             
	.extern     I2C2_ER_IRQHandler                @ I2C2 Error                                               
	.extern     SPI1_IRQHandler                   @ SPI1                                            
	.extern     SPI2_IRQHandler                   @ SPI2                                            
	.extern     USART1_IRQHandler                 @ USART1                                          
	.extern     USART2_IRQHandler                 @ USART2                                          
	.extern     USART3_IRQHandler                 @ USART3                                          
	.extern     EXTI15_10_IRQHandler              @ External Line[15:10]  
	.extern     RTC_Alarm_IRQHandler              @ RTC Alarm (A and B) through EXTI Line                                     
	.extern     TIM8_BRK_TIM12_IRQHandler         @ TIM8 Break Interrupt and TIM12 global interrupt                 
	.extern     TIM8_UP_TIM13_IRQHandler          @ TIM8 Update Interrupt and TIM13 global interrupt
	.extern     TIM8_TRG_COM_TIM14_IRQHandler     @ TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt
	.extern     TIM8_CC_IRQHandler                @ TIM8 Capture Compare Interrupt
	.extern     DMA1_Stream7_IRQHandler           @ DMA1 Stream7                                           
	.extern     FMC_IRQHandler                    @ FMC                             
	.extern     SDMMC1_IRQHandler                 @ SDMMC1                            
	.extern     TIM5_IRQHandler                   @ TIM5                            
	.extern     SPI3_IRQHandler                   @ SPI3                            
	.extern     UART4_IRQHandler                  @ UART4                           
	.extern     UART5_IRQHandler                  @ UART5                           
	.extern     TIM6_DAC_IRQHandler               @ TIM6 and DAC1&2 underrun errors           
	.extern     TIM7_IRQHandler                   @ TIM7           
	.extern     DMA2_Stream0_IRQHandler           @ DMA2 Stream 0                   
	.extern     DMA2_Stream1_IRQHandler           @ DMA2 Stream 1                   
	.extern     DMA2_Stream2_IRQHandler           @ DMA2 Stream 2                   
	.extern     DMA2_Stream3_IRQHandler           @ DMA2 Stream 3                   
	.extern     DMA2_Stream4_IRQHandler           @ DMA2 Stream 4                   
	.extern     ETH_IRQHandler                    @ Ethernet                        
	.extern     ETH_WKUP_IRQHandler               @ Ethernet Wakeup through EXTI line              
	.extern     FDCAN_CAL_IRQHandler              @ FDCAN calibration unit interrupt                    
	.extern     DMA2_Stream5_IRQHandler           @ DMA2 Stream 5                   
	.extern     DMA2_Stream6_IRQHandler           @ DMA2 Stream 6                   
	.extern     DMA2_Stream7_IRQHandler           @ DMA2 Stream 7                   
	.extern     USART6_IRQHandler                 @ USART6                           
	.extern     I2C3_EV_IRQHandler                @ I2C3 event                             
	.extern     I2C3_ER_IRQHandler                @ I2C3 error                             
	.extern     OTG_HS_EP1_OUT_IRQHandler         @ USB OTG HS End Point 1 Out                      
	.extern     OTG_HS_EP1_IN_IRQHandler          @ USB OTG HS End Point 1 In                       
	.extern     OTG_HS_WKUP_IRQHandler            @ USB OTG HS Wakeup through EXTI                         
	.extern     OTG_HS_IRQHandler                 @ USB OTG HS                    
	.extern     DCMI_IRQHandler                   @ DCMI                                           
	.extern     RNG_IRQHandler                    @ Rng
	.extern     FPU_IRQHandler                    @ FPU
	.extern     UART7_IRQHandler                  @ UART7
	.extern     UART8_IRQHandler                  @ UART8
	.extern     SPI4_IRQHandler                   @ SPI4
	.extern     SPI5_IRQHandler                   @ SPI5
	.extern     SPI6_IRQHandler                   @ SPI6
	.extern     SAI1_IRQHandler                   @ SAI1
	.extern     LTDC_IRQHandler                   @ LTDC
	.extern     LTDC_ER_IRQHandler                @ LTDC error
	.extern     DMA2D_IRQHandler                  @ DMA2D
	.extern     SAI2_IRQHandler                   @ SAI2
	.extern     QUADSPI_IRQHandler                @ QUADSPI
	.extern     LPTIM1_IRQHandler                 @ LPTIM1
	.extern     CEC_IRQHandler                    @ HDMI_CEC
	.extern     I2C4_EV_IRQHandler                @ I2C4 Event                             
	.extern     I2C4_ER_IRQHandler                @ I2C4 Error 
	.extern     SPDIF_RX_IRQHandler               @ SPDIF_RX
	.extern     OTG_FS_EP1_OUT_IRQHandler         @ USB OTG FS End Point 1 Out                      
	.extern     OTG_FS_EP1_IN_IRQHandler          @ USB OTG FS End Point 1 In                       
	.extern     OTG_FS_WKUP_IRQHandler            @ USB OTG FS Wakeup through EXTI                         
	.extern     OTG_FS_IRQHandler                 @ USB OTG FS                 
	.extern     DMAMUX1_OVR_IRQHandler            @ DMAMUX1 Overrun interrupt  
	.extern     HRTIM1_Master_IRQHandler          @ HRTIM Master Timer global Interrupts                              
	.extern     HRTIM1_TIMA_IRQHandler            @ HRTIM Timer A global Interrupt                                    
	.extern     HRTIM1_TIMB_IRQHandler            @ HRTIM Timer B global Interrupt                                    
	.extern     HRTIM1_TIMC_IRQHandler            @ HRTIM Timer C global Interrupt                                    
	.extern     HRTIM1_TIMD_IRQHandler            @ HRTIM Timer D global Interrupt                                    
	.extern     HRTIM1_TIME_IRQHandler            @ HRTIM Timer E global Interrupt                                    
	.extern     HRTIM1_FLT_IRQHandler             @ HRTIM Fault global Interrupt 
	.extern     DFSDM1_FLT0_IRQHandler            @ DFSDM Filter0 Interrupt   
	.extern     DFSDM1_FLT1_IRQHandler            @ DFSDM Filter1 Interrupt                                            
	.extern     DFSDM1_FLT2_IRQHandler            @ DFSDM Filter2 Interrupt                                            
	.extern     DFSDM1_FLT3_IRQHandler            @ DFSDM Filter3 Interrupt                                                                                    
	.extern     SAI3_IRQHandler                   @ SAI3 global Interrupt                                             
	.extern     SWPMI1_IRQHandler                 @ Serial Wire Interface 1 global interrupt                          
	.extern     TIM15_IRQHandler                  @ TIM15 global Interrupt                                            
	.extern     TIM16_IRQHandler                  @ TIM16 global Interrupt                                            
	.extern     TIM17_IRQHandler                  @ TIM17 global Interrupt                                            
	.extern     MDIOS_WKUP_IRQHandler             @ MDIOS Wakeup  Interrupt                                           
	.extern     MDIOS_IRQHandler                  @ MDIOS global Interrupt                                            
	.extern     JPEG_IRQHandler                   @ JPEG global Interrupt                                             
	.extern     MDMA_IRQHandler                   @ MDMA global Interrupt                                                
	.extern     SDMMC2_IRQHandler                 @ SDMMC2 global Interrupt                                           
	.extern     HSEM1_IRQHandler                  @ HSEM1 global Interrupt                                                
	.extern     ADC3_IRQHandler                   @ ADC3 global Interrupt                                              
	.extern     DMAMUX2_OVR_IRQHandler            @ DMAMUX Overrun interrupt                                           
	.extern     BDMA_Channel0_IRQHandler          @ BDMA Channel 0 global Interrupt                                    
	.extern     BDMA_Channel1_IRQHandler          @ BDMA Channel 1 global Interrupt                                    
	.extern     BDMA_Channel2_IRQHandler          @ BDMA Channel 2 global Interrupt                                    
	.extern     BDMA_Channel3_IRQHandler          @ BDMA Channel 3 global Interrupt                                    
	.extern     BDMA_Channel4_IRQHandler          @ BDMA Channel 4 global Interrupt                                    
	.extern     BDMA_Channel5_IRQHandler          @ BDMA Channel 5 global Interrupt                                    
	.extern     BDMA_Channel6_IRQHandler          @ BDMA Channel 6 global Interrupt                                    
	.extern     BDMA_Channel7_IRQHandler          @ BDMA Channel 7 global Interrupt                                    
	.extern     COMP1_IRQHandler                  @ COMP1 global Interrupt                                            
	.extern     LPTIM2_IRQHandler                 @ LP TIM2 global interrupt                                          
	.extern     LPTIM3_IRQHandler                 @ LP TIM3 global interrupt                                          
	.extern     LPTIM4_IRQHandler                 @ LP TIM4 global interrupt                                          
	.extern     LPTIM5_IRQHandler                 @ LP TIM5 global interrupt                                          
	.extern     LPUART1_IRQHandler                @ LP UART1 interrupt                                                                            
	.extern     CRS_IRQHandler                    @ Clock Recovery Global Interrupt                                           
	.extern     SAI4_IRQHandler                   @ SAI4 global interrupt                                    
	.extern     WAKEUP_PIN_IRQHandler             @ Interrupt for all 6 wake-up pins 
    
    .global  ARM_Vectors

@ Vector Table For the application
@
@ bootloaders place this at offset 0 and the hardware uses
@ it from there at power on reset. Applications (or the boot
@  loader itself) can place a copy in RAM to allow dynamically
@ "hooking" interrupts at run-time
@
@ It is expected ,though not required, that the .externed handlers
@ have a default empty implementation declared with WEAK linkage
@ thus allowing applications to override the default by simply
@ defining a function with the same name and proper behavior
@ [ NOTE:
@   This standardized handler naming is an essential part of the
@   CMSIS-Core specification. It is relied upon by the CMSIS-RTX
@   implementation as well as much of the mbed framework.
@ ]
    .section VectorTable
    .align 9

@ The first 16 entries are all architecturally defined by ARM
ARM_Vectors:
	.long     __initial_sp                      @ Top of Stack
	.long     Reset_Handler                     @ Reset Handler
	.long     NMI_Handler                       @ NMI Handler
	.long     HardFault_Handler                 @ Hard Fault Handler
	.long     MemManage_Handler                 @ MPU Fault Handler
	.long     BusFault_Handler                  @ Bus Fault Handler
	.long     UsageFault_Handler                @ Usage Fault Handler
	.long     0                                 @ Reserved
	.long     0                                 @ Reserved
	.long     0                                 @ Reserved
	.long     0                                 @ Reserved
	.long     SVC_Handler                       @ SVCall Handler
	.long     DebugMon_Handler                  @ Debug Monitor Handler
	.long     0                                 @ Reserved
	.long     PendSV_Handler                    @ PendSV Handler
	.long     SysTick_Handler                   @ SysTick Handler

@ External Interrupts
	.long     WWDG_IRQHandler                   @ Window WatchDog interrupt ( wwdg1_it)                                         
	.long     PVD_AVD_IRQHandler                @ PVD/AVD through EXTI Line detection                        
	.long     TAMP_STAMP_IRQHandler             @ Tamper and TimeStamps through the EXTI line            
	.long     RTC_WKUP_IRQHandler               @ RTC Wakeup through the EXTI line                       
	.long     FLASH_IRQHandler                  @ FLASH                                           
	.long     RCC_IRQHandler                    @ RCC                                             
	.long     EXTI0_IRQHandler                  @ EXTI Line0                                             
	.long     EXTI1_IRQHandler                  @ EXTI Line1                                             
	.long     EXTI2_IRQHandler                  @ EXTI Line2                                             
	.long     EXTI3_IRQHandler                  @ EXTI Line3                                             
	.long     EXTI4_IRQHandler                  @ EXTI Line4 
	.long     DMA1_Stream0_IRQHandler           @ DMA1 Stream 0
	.long     DMA1_Stream1_IRQHandler           @ DMA1 Stream 1                                   
	.long     DMA1_Stream2_IRQHandler           @ DMA1 Stream 2                                   
	.long     DMA1_Stream3_IRQHandler           @ DMA1 Stream 3                                   
	.long     DMA1_Stream4_IRQHandler           @ DMA1 Stream 4                                   
	.long     DMA1_Stream5_IRQHandler           @ DMA1 Stream 5                                   
	.long     DMA1_Stream6_IRQHandler           @ DMA1 Stream 6  
	.long     ADC_IRQHandler                    @ ADC1, ADC2                             
	.long     FDCAN1_IT0_IRQHandler             @ FDCAN1 interrupt line 0                        
	.long     FDCAN2_IT0_IRQHandler             @ FDCAN2 interrupt line 0                               
	.long     FDCAN1_IT1_IRQHandler             @ FDCAN1 interrupt line 1                        
	.long     FDCAN2_IT1_IRQHandler             @ FDCAN2 interrupt line 1                                               
	.long     EXTI9_5_IRQHandler                @ External Line[9:5]s                                    
	.long     TIM1_BRK_IRQHandler               @ TIM1 Break interrupt                   
	.long     TIM1_UP_IRQHandler                @ TIM1 Update Interrupt                 
	.long     TIM1_TRG_COM_IRQHandler           @ TIM1 Trigger and Commutation Interrupt 
	.long     TIM1_CC_IRQHandler                @ TIM1 Capture Compare                                   
	.long     TIM2_IRQHandler                   @ TIM2                                            
	.long     TIM3_IRQHandler                   @ TIM3                                            
	.long     TIM4_IRQHandler                   @ TIM4                                            
	.long     I2C1_EV_IRQHandler                @ I2C1 Event                                             
	.long     I2C1_ER_IRQHandler                @ I2C1 Error                                             
	.long     I2C2_EV_IRQHandler                @ I2C2 Event                                             
	.long     I2C2_ER_IRQHandler                @ I2C2 Error                                               
	.long     SPI1_IRQHandler                   @ SPI1                                            
	.long     SPI2_IRQHandler                   @ SPI2                                            
	.long     USART1_IRQHandler                 @ USART1                                          
	.long     USART2_IRQHandler                 @ USART2                                          
	.long     USART3_IRQHandler                 @ USART3                                          
	.long     EXTI15_10_IRQHandler              @ External Line[15:10]  
	.long     RTC_Alarm_IRQHandler              @ RTC Alarm (A and B) through EXTI Line
	.long     0                                 @ Reserved                                          
	.long     TIM8_BRK_TIM12_IRQHandler         @ TIM8 Break Interrupt and TIM12 global interrupt                 
	.long     TIM8_UP_TIM13_IRQHandler          @ TIM8 Update Interrupt and TIM13 global interrupt
	.long     TIM8_TRG_COM_TIM14_IRQHandler     @ TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt
	.long     TIM8_CC_IRQHandler                @ TIM8 Capture Compare Interrupt
	.long     DMA1_Stream7_IRQHandler           @ DMA1 Stream7                                           
	.long     FMC_IRQHandler                    @ FMC                             
	.long     SDMMC1_IRQHandler                 @ SDMMC1                            
	.long     TIM5_IRQHandler                   @ TIM5                            
	.long     SPI3_IRQHandler                   @ SPI3                            
	.long     UART4_IRQHandler                  @ UART4                           
	.long     UART5_IRQHandler                  @ UART5                           
	.long     TIM6_DAC_IRQHandler               @ TIM6 and DAC1&2 underrun errors           
	.long     TIM7_IRQHandler                   @ TIM7           
	.long     DMA2_Stream0_IRQHandler           @ DMA2 Stream 0                   
	.long     DMA2_Stream1_IRQHandler           @ DMA2 Stream 1                   
	.long     DMA2_Stream2_IRQHandler           @ DMA2 Stream 2                   
	.long     DMA2_Stream3_IRQHandler           @ DMA2 Stream 3                   
	.long     DMA2_Stream4_IRQHandler           @ DMA2 Stream 4                   
	.long     ETH_IRQHandler                    @ Ethernet                        
	.long     ETH_WKUP_IRQHandler               @ Ethernet Wakeup through EXTI line              
	.long     FDCAN_CAL_IRQHandler              @ FDCAN calibration unit interrupt                        
	.long     0                                 @ Reserved                              
	.long     0                                 @ Reserved 
	.long     0                                 @ Reserved 
	.long     0                                 @ Reserved                      
	.long     DMA2_Stream5_IRQHandler           @ DMA2 Stream 5                   
	.long     DMA2_Stream6_IRQHandler           @ DMA2 Stream 6                   
	.long     DMA2_Stream7_IRQHandler           @ DMA2 Stream 7                   
	.long     USART6_IRQHandler                 @ USART6                           
	.long     I2C3_EV_IRQHandler                @ I2C3 event                             
	.long     I2C3_ER_IRQHandler                @ I2C3 error                             
	.long     OTG_HS_EP1_OUT_IRQHandler         @ USB OTG HS End Point 1 Out                      
	.long     OTG_HS_EP1_IN_IRQHandler          @ USB OTG HS End Point 1 In                       
	.long     OTG_HS_WKUP_IRQHandler            @ USB OTG HS Wakeup through EXTI                         
	.long     OTG_HS_IRQHandler                 @ USB OTG HS                    
	.long     DCMI_IRQHandler                   @ DCMI                            
	.long     0                                 @ Reserved                                     
	.long     RNG_IRQHandler                    @ Rng
	.long     FPU_IRQHandler                    @ FPU
	.long     UART7_IRQHandler                  @ UART7
	.long     UART8_IRQHandler                  @ UART8
	.long     SPI4_IRQHandler                   @ SPI4
	.long     SPI5_IRQHandler                   @ SPI5
	.long     SPI6_IRQHandler                   @ SPI6
	.long     SAI1_IRQHandler                   @ SAI1
	.long     LTDC_IRQHandler                   @ LTDC
	.long     LTDC_ER_IRQHandler                @ LTDC error
	.long     DMA2D_IRQHandler                  @ DMA2D
	.long     SAI2_IRQHandler                   @ SAI2
	.long     QUADSPI_IRQHandler                @ QUADSPI
	.long     LPTIM1_IRQHandler                 @ LPTIM1
	.long     CEC_IRQHandler                    @ HDMI_CEC
	.long     I2C4_EV_IRQHandler                @ I2C4 Event                             
	.long     I2C4_ER_IRQHandler                @ I2C4 Error 
	.long     SPDIF_RX_IRQHandler               @ SPDIF_RX
	.long     OTG_FS_EP1_OUT_IRQHandler         @ USB OTG FS End Point 1 Out                      
	.long     OTG_FS_EP1_IN_IRQHandler          @ USB OTG FS End Point 1 In                       
	.long     OTG_FS_WKUP_IRQHandler            @ USB OTG FS Wakeup through EXTI                         
	.long     OTG_FS_IRQHandler                 @ USB OTG FS                 
	.long     DMAMUX1_OVR_IRQHandler            @ DMAMUX1 Overrun interrupt  
	.long     HRTIM1_Master_IRQHandler          @ HRTIM Master Timer global Interrupts                              
	.long     HRTIM1_TIMA_IRQHandler            @ HRTIM Timer A global Interrupt                                    
	.long     HRTIM1_TIMB_IRQHandler            @ HRTIM Timer B global Interrupt                                    
	.long     HRTIM1_TIMC_IRQHandler            @ HRTIM Timer C global Interrupt                                    
	.long     HRTIM1_TIMD_IRQHandler            @ HRTIM Timer D global Interrupt                                    
	.long     HRTIM1_TIME_IRQHandler            @ HRTIM Timer E global Interrupt                                    
	.long     HRTIM1_FLT_IRQHandler             @ HRTIM Fault global Interrupt 
	.long     DFSDM1_FLT0_IRQHandler            @ DFSDM Filter0 Interrupt   
	.long     DFSDM1_FLT1_IRQHandler            @ DFSDM Filter1 Interrupt                                            
	.long     DFSDM1_FLT2_IRQHandler            @ DFSDM Filter2 Interrupt                                            
	.long     DFSDM1_FLT3_IRQHandler            @ DFSDM Filter3 Interrupt                                                                                    
	.long     SAI3_IRQHandler                   @ SAI3 global Interrupt                                             
	.long     SWPMI1_IRQHandler                 @ Serial Wire Interface 1 global interrupt                          
	.long     TIM15_IRQHandler                  @ TIM15 global Interrupt                                            
	.long     TIM16_IRQHandler                  @ TIM16 global Interrupt                                            
	.long     TIM17_IRQHandler                  @ TIM17 global Interrupt                                            
	.long     MDIOS_WKUP_IRQHandler             @ MDIOS Wakeup  Interrupt                                           
	.long     MDIOS_IRQHandler                  @ MDIOS global Interrupt                                            
	.long     JPEG_IRQHandler                   @ JPEG global Interrupt                                             
	.long     MDMA_IRQHandler                   @ MDMA global Interrupt                                             
	.long     0                                 @ Reserved                                               
	.long     SDMMC2_IRQHandler                 @ SDMMC2 global Interrupt                                           
	.long     HSEM1_IRQHandler                  @ HSEM1 global Interrupt                                             
	.long     0                                 @ Reserved                                              
	.long     ADC3_IRQHandler                   @ ADC3 global Interrupt                                              
	.long     DMAMUX2_OVR_IRQHandler            @ DMAMUX Overrun interrupt                                           
	.long     BDMA_Channel0_IRQHandler          @ BDMA Channel 0 global Interrupt                                    
	.long     BDMA_Channel1_IRQHandler          @ BDMA Channel 1 global Interrupt                                    
	.long     BDMA_Channel2_IRQHandler          @ BDMA Channel 2 global Interrupt                                    
	.long     BDMA_Channel3_IRQHandler          @ BDMA Channel 3 global Interrupt                                    
	.long     BDMA_Channel4_IRQHandler          @ BDMA Channel 4 global Interrupt                                    
	.long     BDMA_Channel5_IRQHandler          @ BDMA Channel 5 global Interrupt                                    
	.long     BDMA_Channel6_IRQHandler          @ BDMA Channel 6 global Interrupt                                    
	.long     BDMA_Channel7_IRQHandler          @ BDMA Channel 7 global Interrupt                                    
	.long     COMP1_IRQHandler                  @ COMP1 global Interrupt                                            
	.long     LPTIM2_IRQHandler                 @ LP TIM2 global interrupt                                          
	.long     LPTIM3_IRQHandler                 @ LP TIM3 global interrupt                                          
	.long     LPTIM4_IRQHandler                 @ LP TIM4 global interrupt                                          
	.long     LPTIM5_IRQHandler                 @ LP TIM5 global interrupt                                          
	.long     LPUART1_IRQHandler                @ LP UART1 interrupt                                                
	.long     0                                 @ Reserved                                                                              
	.long     CRS_IRQHandler                    @ Clock Recovery Global Interrupt                                   
	.long     0                                 @ Reserved                                              
	.long     SAI4_IRQHandler                   @ SAI4 global interrupt                                                
	.long     0                                 @ Reserved                                 
	.long     0                                 @ Reserved                                    
	.long     WAKEUP_PIN_IRQHandler             @ Interrupt for all 6 wake-up pins 
.end
