#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "usbd_def.h"

void MX_USB_DEVICE_Init(void);
void MX_USB_DEVICE_DeInit(void);

#ifdef __cplusplus
}
#endif