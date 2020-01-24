//#pragma once
#ifndef _STM32H7_USB_FUNCTIONS_H_
#define _STM32H7_USB_FUNCTIONS_H_ 1

void USB_Error_Handler(void);
int CPU_USB_Queue_Rx_Data(  char c );

#define USB_DM_Pin 						GPIO_PIN_11
#define USB_DM_GPIO_Port 				GPIOA

#define USB_DP_Pin 						GPIO_PIN_12
#define USB_DP_GPIO_Port 				GPIOA

#endif
