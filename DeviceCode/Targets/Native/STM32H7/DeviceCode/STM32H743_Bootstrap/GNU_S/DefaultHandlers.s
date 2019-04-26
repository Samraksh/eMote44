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
@@ STM32F746 Default Handlers
@@
    .syntax unified
    .arch armv7-m

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                
@ Dummy Exception Handlers (infinite loops which can be overloaded since they are exported with weak linkage )

    .section SectionForBootstrapOperations, "ax", %progbits

    .align 1
    .thumb_func
    .weak Default_Handler
    .type Default_Handler, %function
Default_Handler:
    b  .
    .size Default_Handler, . - Default_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro    def_irq_handler    handler_name
    .weak    \handler_name
    .set     \handler_name, Default_Handler
    .endm

    def_irq_handler NMI_Handler
    def_irq_handler HardFault_Handler
    def_irq_handler MemManage_Handler
    def_irq_handler BusFault_Handler
    def_irq_handler UsageFault_Handler
    def_irq_handler SVC_Handler
    def_irq_handler DebugMon_Handler
    def_irq_handler PendSV_Handler
    def_irq_handler SysTick_Handler

@ SOC Specific default handlers
	def_irq_handler     WWDG_IRQHandler                   @ Window WatchDog interrupt ( wwdg1_it)                                         
	def_irq_handler     PVD_AVD_IRQHandler                @ PVD/AVD through EXTI Line detection                        
	def_irq_handler     TAMP_STAMP_IRQHandler             @ Tamper and TimeStamps through the EXTI line            
	def_irq_handler     RTC_WKUP_IRQHandler               @ RTC Wakeup through the EXTI line                       
	def_irq_handler     FLASH_IRQHandler                  @ FLASH                                           
	def_irq_handler     RCC_IRQHandler                    @ RCC                                             
	def_irq_handler     EXTI0_IRQHandler                  @ EXTI Line0                                             
	def_irq_handler     EXTI1_IRQHandler                  @ EXTI Line1                                             
	def_irq_handler     EXTI2_IRQHandler                  @ EXTI Line2                                             
	def_irq_handler     EXTI3_IRQHandler                  @ EXTI Line3                                             
	def_irq_handler     EXTI4_IRQHandler                  @ EXTI Line4 
	def_irq_handler     DMA1_Stream0_IRQHandler           @ DMA1 Stream 0
	def_irq_handler     DMA1_Stream1_IRQHandler           @ DMA1 Stream 1                                   
	def_irq_handler     DMA1_Stream2_IRQHandler           @ DMA1 Stream 2                                   
	def_irq_handler     DMA1_Stream3_IRQHandler           @ DMA1 Stream 3                                   
	def_irq_handler     DMA1_Stream4_IRQHandler           @ DMA1 Stream 4                                   
	def_irq_handler     DMA1_Stream5_IRQHandler           @ DMA1 Stream 5                                   
	def_irq_handler     DMA1_Stream6_IRQHandler           @ DMA1 Stream 6  
	def_irq_handler     ADC_IRQHandler                    @ ADC1, ADC2                             
	def_irq_handler     FDCAN1_IT0_IRQHandler             @ FDCAN1 interrupt line 0                        
	def_irq_handler     FDCAN2_IT0_IRQHandler             @ FDCAN2 interrupt line 0                               
	def_irq_handler     FDCAN1_IT1_IRQHandler             @ FDCAN1 interrupt line 1                        
	def_irq_handler     FDCAN2_IT1_IRQHandler             @ FDCAN2 interrupt line 1                                               
	def_irq_handler     EXTI9_5_IRQHandler                @ External Line[9:5]s                                    
	def_irq_handler     TIM1_BRK_IRQHandler               @ TIM1 Break interrupt                   
	def_irq_handler     TIM1_UP_IRQHandler                @ TIM1 Update Interrupt                 
	def_irq_handler     TIM1_TRG_COM_IRQHandler           @ TIM1 Trigger and Commutation Interrupt 
	def_irq_handler     TIM1_CC_IRQHandler                @ TIM1 Capture Compare                                   
	def_irq_handler     TIM2_IRQHandler                   @ TIM2                                            
	def_irq_handler     TIM3_IRQHandler                   @ TIM3                                            
	def_irq_handler     TIM4_IRQHandler                   @ TIM4                                            
	def_irq_handler     I2C1_EV_IRQHandler                @ I2C1 Event                                             
	def_irq_handler     I2C1_ER_IRQHandler                @ I2C1 Error                                             
	def_irq_handler     I2C2_EV_IRQHandler                @ I2C2 Event                                             
	def_irq_handler     I2C2_ER_IRQHandler                @ I2C2 Error                                               
	def_irq_handler     SPI1_IRQHandler                   @ SPI1                                            
	def_irq_handler     SPI2_IRQHandler                   @ SPI2                                            
	def_irq_handler     USART1_IRQHandler                 @ USART1                                          
	def_irq_handler     USART2_IRQHandler                 @ USART2                                          
	def_irq_handler     USART3_IRQHandler                 @ USART3                                          
	def_irq_handler     EXTI15_10_IRQHandler              @ External Line[15:10]  
	def_irq_handler     RTC_Alarm_IRQHandler              @ RTC Alarm (A and B) through EXTI Line                                     
	def_irq_handler     TIM8_BRK_TIM12_IRQHandler         @ TIM8 Break Interrupt and TIM12 global interrupt                 
	def_irq_handler     TIM8_UP_TIM13_IRQHandler          @ TIM8 Update Interrupt and TIM13 global interrupt
	def_irq_handler     TIM8_TRG_COM_TIM14_IRQHandler     @ TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt
	def_irq_handler     TIM8_CC_IRQHandler                @ TIM8 Capture Compare Interrupt
	def_irq_handler     DMA1_Stream7_IRQHandler           @ DMA1 Stream7                                           
	def_irq_handler     FMC_IRQHandler                    @ FMC                             
	def_irq_handler     SDMMC1_IRQHandler                 @ SDMMC1                            
	def_irq_handler     TIM5_IRQHandler                   @ TIM5                            
	def_irq_handler     SPI3_IRQHandler                   @ SPI3                            
	def_irq_handler     UART4_IRQHandler                  @ UART4                           
	def_irq_handler     UART5_IRQHandler                  @ UART5                           
	def_irq_handler     TIM6_DAC_IRQHandler               @ TIM6 and DAC1&2 underrun errors           
	def_irq_handler     TIM7_IRQHandler                   @ TIM7           
	def_irq_handler     DMA2_Stream0_IRQHandler           @ DMA2 Stream 0                   
	def_irq_handler     DMA2_Stream1_IRQHandler           @ DMA2 Stream 1                   
	def_irq_handler     DMA2_Stream2_IRQHandler           @ DMA2 Stream 2                   
	def_irq_handler     DMA2_Stream3_IRQHandler           @ DMA2 Stream 3                   
	def_irq_handler     DMA2_Stream4_IRQHandler           @ DMA2 Stream 4                   
	def_irq_handler     ETH_IRQHandler                    @ Ethernet                        
	def_irq_handler     ETH_WKUP_IRQHandler               @ Ethernet Wakeup through EXTI line              
	def_irq_handler     FDCAN_CAL_IRQHandler              @ FDCAN calibration unit interrupt                    
	def_irq_handler     DMA2_Stream5_IRQHandler           @ DMA2 Stream 5                   
	def_irq_handler     DMA2_Stream6_IRQHandler           @ DMA2 Stream 6                   
	def_irq_handler     DMA2_Stream7_IRQHandler           @ DMA2 Stream 7                   
	def_irq_handler     USART6_IRQHandler                 @ USART6                           
	def_irq_handler     I2C3_EV_IRQHandler                @ I2C3 event                             
	def_irq_handler     I2C3_ER_IRQHandler                @ I2C3 error                             
	def_irq_handler     OTG_HS_EP1_OUT_IRQHandler         @ USB OTG HS End Point 1 Out                      
	def_irq_handler     OTG_HS_EP1_IN_IRQHandler          @ USB OTG HS End Point 1 In                       
	def_irq_handler     OTG_HS_WKUP_IRQHandler            @ USB OTG HS Wakeup through EXTI                         
	def_irq_handler     OTG_HS_IRQHandler                 @ USB OTG HS                    
	def_irq_handler     DCMI_IRQHandler                   @ DCMI                                           
	def_irq_handler     RNG_IRQHandler                    @ Rng
	def_irq_handler     FPU_IRQHandler                    @ FPU
	def_irq_handler     UART7_IRQHandler                  @ UART7
	def_irq_handler     UART8_IRQHandler                  @ UART8
	def_irq_handler     SPI4_IRQHandler                   @ SPI4
	def_irq_handler     SPI5_IRQHandler                   @ SPI5
	def_irq_handler     SPI6_IRQHandler                   @ SPI6
	def_irq_handler     SAI1_IRQHandler                   @ SAI1
	def_irq_handler     LTDC_IRQHandler                   @ LTDC
	def_irq_handler     LTDC_ER_IRQHandler                @ LTDC error
	def_irq_handler     DMA2D_IRQHandler                  @ DMA2D
	def_irq_handler     SAI2_IRQHandler                   @ SAI2
	def_irq_handler     QUADSPI_IRQHandler                @ QUADSPI
	def_irq_handler     LPTIM1_IRQHandler                 @ LPTIM1
	def_irq_handler     CEC_IRQHandler                    @ HDMI_CEC
	def_irq_handler     I2C4_EV_IRQHandler                @ I2C4 Event                             
	def_irq_handler     I2C4_ER_IRQHandler                @ I2C4 Error 
	def_irq_handler     SPDIF_RX_IRQHandler               @ SPDIF_RX
	def_irq_handler     OTG_FS_EP1_OUT_IRQHandler         @ USB OTG FS End Point 1 Out                      
	def_irq_handler     OTG_FS_EP1_IN_IRQHandler          @ USB OTG FS End Point 1 In                       
	def_irq_handler     OTG_FS_WKUP_IRQHandler            @ USB OTG FS Wakeup through EXTI                         
	def_irq_handler     OTG_FS_IRQHandler                 @ USB OTG FS                 
	def_irq_handler     DMAMUX1_OVR_IRQHandler            @ DMAMUX1 Overrun interrupt  
	def_irq_handler     HRTIM1_Master_IRQHandler          @ HRTIM Master Timer global Interrupts                              
	def_irq_handler     HRTIM1_TIMA_IRQHandler            @ HRTIM Timer A global Interrupt                                    
	def_irq_handler     HRTIM1_TIMB_IRQHandler            @ HRTIM Timer B global Interrupt                                    
	def_irq_handler     HRTIM1_TIMC_IRQHandler            @ HRTIM Timer C global Interrupt                                    
	def_irq_handler     HRTIM1_TIMD_IRQHandler            @ HRTIM Timer D global Interrupt                                    
	def_irq_handler     HRTIM1_TIME_IRQHandler            @ HRTIM Timer E global Interrupt                                    
	def_irq_handler     HRTIM1_FLT_IRQHandler             @ HRTIM Fault global Interrupt 
	def_irq_handler     DFSDM1_FLT0_IRQHandler            @ DFSDM Filter0 Interrupt   
	def_irq_handler     DFSDM1_FLT1_IRQHandler            @ DFSDM Filter1 Interrupt                                            
	def_irq_handler     DFSDM1_FLT2_IRQHandler            @ DFSDM Filter2 Interrupt                                            
	def_irq_handler     DFSDM1_FLT3_IRQHandler            @ DFSDM Filter3 Interrupt                                                                                    
	def_irq_handler     SAI3_IRQHandler                   @ SAI3 global Interrupt                                             
	def_irq_handler     SWPMI1_IRQHandler                 @ Serial Wire Interface 1 global interrupt                          
	def_irq_handler     TIM15_IRQHandler                  @ TIM15 global Interrupt                                            
	def_irq_handler     TIM16_IRQHandler                  @ TIM16 global Interrupt                                            
	def_irq_handler     TIM17_IRQHandler                  @ TIM17 global Interrupt                                            
	def_irq_handler     MDIOS_WKUP_IRQHandler             @ MDIOS Wakeup  Interrupt                                           
	def_irq_handler     MDIOS_IRQHandler                  @ MDIOS global Interrupt                                            
	def_irq_handler     JPEG_IRQHandler                   @ JPEG global Interrupt                                             
	def_irq_handler     MDMA_IRQHandler                   @ MDMA global Interrupt                                                
	def_irq_handler     SDMMC2_IRQHandler                 @ SDMMC2 global Interrupt                                           
	def_irq_handler     HSEM1_IRQHandler                  @ HSEM1 global Interrupt                                                
	def_irq_handler     ADC3_IRQHandler                   @ ADC3 global Interrupt                                              
	def_irq_handler     DMAMUX2_OVR_IRQHandler            @ DMAMUX Overrun interrupt                                           
	def_irq_handler     BDMA_Channel0_IRQHandler          @ BDMA Channel 0 global Interrupt                                    
	def_irq_handler     BDMA_Channel1_IRQHandler          @ BDMA Channel 1 global Interrupt                                    
	def_irq_handler     BDMA_Channel2_IRQHandler          @ BDMA Channel 2 global Interrupt                                    
	def_irq_handler     BDMA_Channel3_IRQHandler          @ BDMA Channel 3 global Interrupt                                    
	def_irq_handler     BDMA_Channel4_IRQHandler          @ BDMA Channel 4 global Interrupt                                    
	def_irq_handler     BDMA_Channel5_IRQHandler          @ BDMA Channel 5 global Interrupt                                    
	def_irq_handler     BDMA_Channel6_IRQHandler          @ BDMA Channel 6 global Interrupt                                    
	def_irq_handler     BDMA_Channel7_IRQHandler          @ BDMA Channel 7 global Interrupt                                    
	def_irq_handler     COMP1_IRQHandler                  @ COMP1 global Interrupt                                            
	def_irq_handler     LPTIM2_IRQHandler                 @ LP TIM2 global interrupt                                          
	def_irq_handler     LPTIM3_IRQHandler                 @ LP TIM3 global interrupt                                          
	def_irq_handler     LPTIM4_IRQHandler                 @ LP TIM4 global interrupt                                          
	def_irq_handler     LPTIM5_IRQHandler                 @ LP TIM5 global interrupt                                          
	def_irq_handler     LPUART1_IRQHandler                @ LP UART1 interrupt                                                                            
	def_irq_handler     CRS_IRQHandler                    @ Clock Recovery Global Interrupt                                           
	def_irq_handler     SAI4_IRQHandler                   @ SAI4 global interrupt                                    
	def_irq_handler     WAKEUP_PIN_IRQHandler             @ Interrupt for all 6 wake-up pins 

    .global  FAULT_SubHandler
    .extern  FAULT_Handler             @ void FAULT_Handler(UINT32*, UINT32)

    .global  HARD_Breakpoint
    .extern  HARD_Breakpoint_Handler   @ void HARD_Breakpoint_Handler(UINT32*)

@ This serves as an adapter from the Cortex-M exception signature
@ to map back to the original CLR API designed around the older ARM
@ mode exception architecture.
    .section i.FAULT_SubHandler, "ax", %progbits
    .thumb_func

FAULT_SubHandler:
        @ on entry, we have an exception frame on the stack:
        @ SP+00: R0
        @ SP+04: R1
        @ SP+08: R2
        @ SP+12: R3
        @ SP+16: R12
        @ SP+20: LR
        @ SP+24: PC
        @ SP+28: PSR
        @ R0-R12 are not overwritten yet
            add      sp,sp,#16             @ remove R0-R3
            push     {r0-r11}              @ store R0-R11
            mov      r0,sp
        @ R0+00: R0-R12
        @ R0+52: LR
        @ R0+56: PC
        @ R0+60: PSR
            mrs      r1,IPSR               @ exception number
            b        FAULT_Handler
        @ never expect to return

    .section    i.HARD_Breakpoint, "ax", %progbits
HARD_Breakpoint:
    @ on entry, were are being called from C/C++ in Thread mode
        add      sp,sp,#-4            @ space for PSR
        push     {r14}                @ store original PC
        push     {r0-r12,r14}         @ store R0 - R12, LR
        mov      r0,sp
        mrs      r1,XPSR
        str      r1,[r0,#60]          @ store PSR
    @ R0+00: R0-R12
    @ R0+52: LR
    @ R0+56: PC
    @ R0+60: PSR
        b        HARD_Breakpoint_Handler
    @ never expect to return

.end
