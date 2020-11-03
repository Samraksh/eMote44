#pragma once

#include <stm32h7xx_hal.h>
#include "../stm32h7xx.h"
#include "../stm32h7xx_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

extern SRAM_HandleTypeDef hsram1;
void MX_FMC_Init(void);
void HAL_SRAM_MspInit(SRAM_HandleTypeDef* hsram);
void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef* hsram);

#ifdef __cplusplus
}
#endif
