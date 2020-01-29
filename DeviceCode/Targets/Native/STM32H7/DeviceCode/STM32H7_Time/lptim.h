#pragma once

#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"

//extern LPTIM_HandleTypeDef hlptim1;

#ifdef __cplusplus
extern "C" {
#endif

void MX_LPTIM1_Init(void);
uint32_t my_get_counter_lptim(void);
uint64_t my_get_counter_lptim_us(void);

#ifdef __cplusplus
}
#endif
