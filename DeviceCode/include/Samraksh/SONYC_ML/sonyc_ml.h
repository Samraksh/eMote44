#pragma once

// To simplify 2D array pointers and syntax
// Naturally generate as 51x64 but ML wants 64x51
typedef float myTypeA[64][51];
typedef float myTypeB[51][64];
void my_transpose(myTypeA A, myTypeB B);
void amp_to_db(float *f, unsigned len);
int maxarg_float(const float *f, unsigned len);

float * mfcc_test(const int32_t *data);
//float * mfcc_test_float(const float *data);
void mfcc_init(void);

#define HOP_SIZE 160
#define FFT_SIZE 1024
#define HALF_FFT (FFT_SIZE/2)
#define NUM_HOPS 51
#define NUM_BINS 64
#define FRAME_SIZE 8000
#define FRAME_WORD_SIZE 4

#ifdef __cplusplus
extern "C" {
#endif

void MX_X_CUBE_AI_Init(void);
int aiRun(const void *in_data, void *out_data);
int aiRun2(const void *in_data, void *out_data);

#ifdef __cplusplus
}
#endif
