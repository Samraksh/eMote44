
#define ARM_MATH_CM7 // Inform arm_math.h we are a Cortex M7.
#define __FPU_PRESENT 1 // Ditto
#include "Samraksh/SONYC_ML/arm_math.h"

#include <stdbool.h>
#include "sonyc_util.h"

#include "filt_taps_fir.h" // const float filt_taps_fir[1025]

#define NUM_TAPS (sizeof(filt_taps_fir) / sizeof(filt_taps_fir[0]))
#define BLOCK_SIZE 8000 // samples per run


static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
static arm_fir_instance_f32 S;

#ifdef _DEBUG
static bool sonyc_filter_is_init = false;
#endif

int sonyc_init_comp_filter(void) {
	arm_fir_init_f32(&S, NUM_TAPS, (float *)filt_taps_fir, firStateF32, BLOCK_SIZE);
#ifdef _DEBUG
	sonyc_filter_is_init = true;
#endif
	return 0;
}

int sonyc_comp_filter_go(const float *src, float *dest) {
#ifdef _DEBUG
	if (!sonyc_filter_is_init)
		__BKPT();
#endif
	arm_fir_f32(&S, (float *)src, dest, BLOCK_SIZE);
	return 0;
}
