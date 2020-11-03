#define ARM_MATH_CM7 // Inform arm_math.h we are a Cortex M7.
#define __FPU_PRESENT 1 // Ditto
#include "Samraksh/SONYC_ML/arm_math.h"

#include <tinyhal.h>
#include <stdbool.h>
#include "sonyc_util.h"

// Not a real header, just has the taps array in a separate file
#include "filt_taps_fir.h" // const static float filt_taps_fir_rom[769]

static float filt_taps_fir[769] __attribute__ (( section (".ram_d1"), aligned(32) ));

#define NUM_TAPS (sizeof(filt_taps_fir) / sizeof(filt_taps_fir[0]))
#define BLOCK_SIZE 32000 // samples per run

static bool sonyc_filter_is_init = false;

static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1] __attribute__ (( section (".ram_d1"), aligned(32) ));
static arm_fir_instance_f32 S;

// IIR STUFF from http://iowahills.com/Example%20Code/IIRSecondOrderImplementation.txt
#define NUM_IIR_SECTIONS 3
static float RegX1[NUM_IIR_SECTIONS], RegX2[NUM_IIR_SECTIONS], RegY1[NUM_IIR_SECTIONS], RegY2[NUM_IIR_SECTIONS];
static float a2[NUM_IIR_SECTIONS], a1[NUM_IIR_SECTIONS], a0[NUM_IIR_SECTIONS], b2[NUM_IIR_SECTIONS], b1[NUM_IIR_SECTIONS], b0[NUM_IIR_SECTIONS]; // The 2nd order IIR coefficients.
static void init_iir_coefs(void) {
	a0[0] = 1.000000000000000000;
	a1[0] = 0.17994383;
	a2[0] = 0.00809495;
	b0[0] = 0.34360061;
	b1[0] = 0.68720122;
	b2[0] = 0.34360061;

	a0[1] = 1.000000000000000000;
	a1[1] = -1.84398492;
	a2[1] = 0.84681175;
	b0[1] = 1.0;
	b1[1] = -2.0;
	b2[1] = 1.0;

	a0[2] = 1.000000000000000000;
	a1[2] = -1.99192882;
	a2[2] = 0.9919451;
	b0[2] = 1.0;
	b1[2] = -2.0;
	b2[2] = 1.0;
	memset(RegX1, 0, sizeof(RegX1));
	memset(RegX2, 0, sizeof(RegX1));
	memset(RegY1, 0, sizeof(RegX1));
	memset(RegY2, 0, sizeof(RegX1));
}
// Form 1 Biquad Section Calc, called by RunIIRBiquadForm1.
static float SectCalcForm1(int k, float x) {
	float y, CenterTap;

	CenterTap = x * b0[k] + b1[k] * RegX1[k] + b2[k] * RegX2[k];
	y = a0[k] * CenterTap - a1[k] * RegY1[k] - a2[k] * RegY2[k];

	RegX2[k] = RegX1[k];
	RegX1[k] = x;
	RegY2[k] = RegY1[k];
	RegY1[k] = y;

	return(y);
}
// Form 1 Biquad
// This uses 2 sets of shift registers, RegX on the input side and RegY on the output side.
static void RunIIRBiquadForm1(float *Input, float *Output, int NumSigPts)
{
 float y;
 int j, k;

 for(j=0; j<NumSigPts; j++)
  {
   y = SectCalcForm1(0, Input[j]);
   for(k=1; k<NUM_IIR_SECTIONS; k++)
	{
	 y = SectCalcForm1(k, y);
	}
   Output[j] = y;
  }
}
// END IIR STUFF

int sonyc_fir_tap_change(unsigned tap, float val) {
	if (tap >= 769) return -1;
	filt_taps_fir[tap] = val;
	return 0;
}

int sonyc_init_filters(void) {
	memcpy(filt_taps_fir, filt_taps_fir_rom, sizeof(filt_taps_fir));
	memset(firStateF32, 0, sizeof(firStateF32));
	arm_fir_init_f32(&S, NUM_TAPS, (float *)filt_taps_fir, firStateF32, BLOCK_SIZE);
	init_iir_coefs();
	sonyc_filter_is_init = true;
	return 0;
}

// About 16.5ms @ 480 MHz for a 32000 sample block of data @ -Og
int sonyc_iir_filter_go(const float *src, float *dest, unsigned blocksize) {
	if (!sonyc_filter_is_init) return -1;
	RunIIRBiquadForm1((float *)src, dest, blocksize);
	//arm_biquad_cascade_df1_f32(&sonyc_a_weight_iir, (float *)src, dest, BLOCK_SIZE);
	return 0;
}

// About 117.5ms @ 480 MHz for a 32000 sample block of data @ -Og
int sonyc_comp_filter_go(const float *src, float *dest, unsigned blocksize) {
	if (!sonyc_filter_is_init) return -1;
	arm_fir_f32(&S, (float *)src, dest, blocksize);
	return 0;
}

void sonyc_iir_filter_reset(void) {
	memset(RegX1, 0, sizeof(RegX1));
	memset(RegX2, 0, sizeof(RegX1));
	memset(RegY1, 0, sizeof(RegX1));
	memset(RegY2, 0, sizeof(RegX1));
}

void sonyc_fir_filter_reset(void) {
	memset(firStateF32, 0, sizeof(firStateF32));
}