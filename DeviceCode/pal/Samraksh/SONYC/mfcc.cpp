/*
 * Copyright (C) 2018 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Description: MFCC feature extraction to match with TensorFlow MFCC Op
 */

#include <tinyhal.h>
#include <string.h>
#include <cstdlib>

#include <Samraksh\SONYC_ML\sonyc_ml.h>

#include "mfcc.h"

#define debug_printf hal_printf

#define NO_HW_BREAKPOINTS
#ifdef NO_HW_BREAKPOINTS
#define __BKPT() ((void)0)
#endif

static MFCC *my_mfcc;

// static bool isNaN(float x)
// {
    // return !(x == x);
// }

void mfcc_init() {
	static MFCC x(MY_FFT_SIZE);
	//debug_printf("MFCC init: Frame: %u Bins: %u\r\n", MY_FFT_SIZE, NUM_FBANK_BINS);
	//my_mfcc = new MFCC(MY_FFT_SIZE);
	my_mfcc = &x;
}

float * mfcc_test(const int32_t *data) {
	my_mfcc->mfcc_compute(data);
	return my_mfcc->get_mel_energies();
}

// float * mfcc_test_float(const float *data) {
	// my_mfcc->mfcc_compute_float(data);
	// return my_mfcc->get_mel_energies();
// }

// Sizes match first param (target)
void my_transpose(myTypeA A, myTypeB B) {
	const int szA = 64;
	const int szB = 51;
	for(int i=0; i<szA; i++)
		for(int j=0; j<szB; j++)
			A[i][j] = B[j][i];
}

int maxarg_float(const float *f, unsigned len) {
	if (f == NULL) return -1;
	float max = f[0];
	unsigned ret = 0;
	for(int i=1; i<len; i++) {
		if (f[i] > max) { max=f[i]; ret=i; }
	}
	return ret;
}

// in-place change amplitude to dB
void amp_to_db(float *f, unsigned len) {
	const float AMIN = 1E-10;
	const float DR = -80.0;
	float log_max = DR;

	if (f == NULL) { __BKPT(); return; }

	// First pass and find the max
	for (int i=0; i<len; i++) {
		float power;
		float log;
		f[i] = fabs(f[i]);
		power = f[i]*f[i];
		if (power < AMIN) power = AMIN;
		log = 10.0 * log10(power);
		if (log > log_max) log_max = log;
		f[i] = log;
	}

	// Second pass, adjust with max and apply DR limit
	for (int i=0; i<len; i++) {
		f[i] -= log_max;
		if (f[i] < DR) f[i] = DR;
	}
}

MFCC::MFCC(int frame_len)
:frame_len(frame_len)
{

  // Round-up to nearest power of 2.
  //frame_len_padded = pow(2,ceil((log(frame_len)/log(2))));

  frame = new float[frame_len_padded];
  buffer = new float[frame_len_padded];
  mel_energies = new float[NUM_FBANK_BINS];

  //create window function
  window_func = new float[frame_len];
  for (int i = 0; i < frame_len; i++)
    window_func[i] = 0.5 - 0.5*cos(M_2PI * ((float)i) / (frame_len));

  //create mel filterbank
  fbank_filter_first = new int32_t[NUM_FBANK_BINS];
  fbank_filter_last = new int32_t[NUM_FBANK_BINS];;
  mel_fbank = create_mel_fbank();

  //initialize FFT
  rfft = new arm_rfft_fast_instance_f32;
  if (arm_rfft_fast_init_f32(rfft, frame_len_padded) == ARM_MATH_ARGUMENT_ERROR) {
	  debug_printf("FFT INIT FAILED\r\n");
	  __BKPT();
  }

}

MFCC::~MFCC() {
  delete []frame;
  delete []buffer;
  delete []mel_energies;
  delete []window_func;
  delete []fbank_filter_first;
  delete []fbank_filter_last;
  delete rfft;
  for(int i=0;i<NUM_FBANK_BINS;i++)
    delete mel_fbank[i];
  delete mel_fbank;
}

float ** MFCC::create_mel_fbank() {

  int32_t bin, i;

  int32_t num_fft_bins = frame_len_padded/2;
  float fft_bin_width = ((float)SAMP_FREQ) / frame_len_padded;
  float mel_low_freq = MelScale(MEL_LOW_FREQ);
  float mel_high_freq = MelScale(MEL_HIGH_FREQ);
  float mel_freq_delta = (mel_high_freq - mel_low_freq) / (NUM_FBANK_BINS+1);

  float *this_bin = new float[num_fft_bins];

  float ** mel_fbank =  new float*[NUM_FBANK_BINS];

  for (bin = 0; bin < NUM_FBANK_BINS; bin++) {

    float left_mel = mel_low_freq + bin * mel_freq_delta;
    float center_mel = mel_low_freq + (bin + 1) * mel_freq_delta;
    float right_mel = mel_low_freq + (bin + 2) * mel_freq_delta;

    int32_t first_index = -1, last_index = -1;

    for (i = 0; i < num_fft_bins; i++) {

      float freq = (fft_bin_width * i);  // center freq of this fft bin.
      float mel = MelScale(freq);
      this_bin[i] = 0.0;

      if (mel > left_mel && mel < right_mel) {
        float weight;
        if (mel <= center_mel) {
          weight = (mel - left_mel) / (center_mel - left_mel);
        } else {
          weight = (right_mel-mel) / (right_mel-center_mel);
        }
        this_bin[i] = weight;
        if (first_index == -1)
          first_index = i;
        last_index = i;
      }
    }

    fbank_filter_first[bin] = first_index;
    fbank_filter_last[bin] = last_index;
    mel_fbank[bin] = new float[last_index-first_index+1];

    int32_t j = 0;
    //copy the part we care about
    for (i = first_index; i <= last_index; i++) {
      mel_fbank[bin][j++] = this_bin[i];
    }
  }
  //delete []this_bin;
  return mel_fbank;
}

/* Debugging use only
void MFCC::mfcc_compute_float(const float * audio_data) {

  int32_t i, j, bin;

  for (i = 0; i < frame_len; i++) {
    frame[i] = audio_data[i];
  }

  //Fill up remaining with zeros
  memset(&frame[frame_len], 0, sizeof(float) * (frame_len_padded-frame_len));

  for (i = 0; i < frame_len; i++) {
    frame[i] *= window_func[i];
  }

  //Compute FFT
  arm_rfft_fast_f32(rfft, frame, buffer, 0);

  //Convert to power spectrum
  //frame is stored as [real0, realN/2-1, real1, im1, real2, im2, ...]
  int32_t half_dim = frame_len_padded/2;
  float first_energy = buffer[0] * buffer[0],
        last_energy =  buffer[1] * buffer[1];  // handle this special case
  for (i = 1; i < half_dim; i++) {
    float real = buffer[i*2], im = buffer[i*2 + 1];
    buffer[i] = real*real + im*im;
  }
  buffer[0] = first_energy;
  buffer[half_dim] = last_energy;

  float sqrt_data;
  //Apply mel filterbanks
  for (bin = 0; bin < NUM_FBANK_BINS; bin++) {
    j = 0;
    float mel_energy = 0;
    int32_t first_index = fbank_filter_first[bin];
    int32_t last_index = fbank_filter_last[bin];
    for (i = first_index; i <= last_index; i++) {
	  if (i == -1) __BKPT();
      arm_sqrt_f32(buffer[i],&sqrt_data);
      mel_energy += (sqrt_data) * mel_fbank[bin][j++];
    }
	#ifdef _DEBUG
	if (isNaN(mel_energy)) __BKPT();
	#endif
    mel_energies[bin] = mel_energy;
  }
}
*/

void MFCC::mfcc_compute(const int32_t * audio_data) {

  int32_t i, j, bin;

  //TensorFlow way of normalizing .wav data to (-1,1)
  for (i = 0; i < frame_len; i++) {
    frame[i] = (float)audio_data[i]/(1<<23);
  }
  //Fill up remaining with zeros
  memset(&frame[frame_len], 0, sizeof(float) * (frame_len_padded-frame_len));

  for (i = 0; i < frame_len; i++) {
    frame[i] *= window_func[i];
  }


  //Compute FFT
  arm_rfft_fast_f32(rfft, frame, buffer, 0);

  //Convert to power spectrum
  //frame is stored as [real0, realN/2-1, real1, im1, real2, im2, ...]
  int32_t half_dim = frame_len_padded/2;
  float first_energy = buffer[0] * buffer[0],
        last_energy =  buffer[1] * buffer[1];  // handle this special case
  for (i = 1; i < half_dim; i++) {
    float real = buffer[i*2], im = buffer[i*2 + 1];
    buffer[i] = real*real + im*im;
  }
  buffer[0] = first_energy;
  buffer[half_dim] = last_energy;

  float sqrt_data;
  //Apply mel filterbanks
  for (bin = 0; bin < NUM_FBANK_BINS; bin++) {
    j = 0;
    float mel_energy = 0;
    int32_t first_index = fbank_filter_first[bin];
    int32_t last_index = fbank_filter_last[bin];
    for (i = first_index; i <= last_index; i++) {
      arm_sqrt_f32(buffer[i],&sqrt_data);
      mel_energy += (sqrt_data) * mel_fbank[bin][j++];
    }
    mel_energies[bin] = mel_energy;
  }
}
