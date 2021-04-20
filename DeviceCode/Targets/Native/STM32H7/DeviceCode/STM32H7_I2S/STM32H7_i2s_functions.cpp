//#define I2S_FUNC_CPP_DEBUG
#ifdef I2S_FUNC_CPP_DEBUG
#pragma GCC push_options
#pragma GCC optimize ("Og")
#endif

#include <tinyhal.h>
//#include <limits.h>

#include <Samraksh\serial_frame_pal.h>
#include <Samraksh\SONYC_ML\sonyc_ml.h>
#include "Samraksh/SONYC_ML/sonyc_util.h" // for sonyc filter

#include <STM32H7_Time\lptim.h>

#define debug_printf hal_printf

//#define MKII_AUDIO_PROCESSING_DEBUG_STATS

//#define MKII_DO_NOT_FILTER_MIC
#ifndef MKII_DO_NOT_FILTER_MIC
#define DO_COMP_AND_A_WEIGHT_FILTERS
#endif

//#define SPL_CALC_SAMPLE_SKIP 256 // At front *and* back
//#define IIR_FILTER_SKIP 780 // At front *and* back

#ifndef SPL_CALC_SAMPLE_SKIP
#define SPL_CALC_SAMPLE_SKIP 0
#endif

#ifndef IIR_FILTER_SKIP
#define IIR_FILTER_SKIP 0
#endif

static HAL_CONTINUATION mic_cont;
static HAL_CONTINUATION mic_error_cont;
static I2S_HandleTypeDef hi2s3;
static DMA_HandleTypeDef hdma_spi3_rx;
static void stereo_to_mono(uint32_t *s, int32_t *m, uint32_t s_len_bytes) __attribute__ ((unused));
static void stereo_to_mono_float(uint32_t *s, float *m, uint32_t s_len_bytes) __attribute__ ((unused));

#define ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))

// DMA audio parameters
#define SAMPLE_DMA_SEC 2 // seconds to be buffered per FULL dma transfer
#define SAMP_RATE_HZ 32000UL // 32 kHz
#define ML_SAMP_RATE_HZ 8000UL // 8 kHz is the input ML rate

#define MONO_DATA_BYTES (ML_SAMP_RATE_HZ * SAMPLE_DMA_SEC / 2 * 4) // HALF transfer size, MONO, but AFTER DECIMATION
#define RAW_AUD_LEN (SAMP_RATE_HZ * SAMPLE_DMA_SEC * 2) // FULL DMA transfer size, in SAMPLES, pre-fixup
#define RAW_HALF_DMA_BYTES (RAW_AUD_LEN * 4 / 2) // 4 bytes per sample, HALF transfer
#define FILTER_BUF_SZ_SAMPS (SAMP_RATE_HZ * SAMPLE_DMA_SEC / 2)	// HALF transfer (e.g., 1s) of MONO samples

#if ( (RAW_AUD_LEN * 4) % 32 != 0)
	#error "raw_data[] size must be % 32 for DMA"
#endif

#define MIC_OFF_Pin GPIO_PIN_0
#define MIC_OFF_GPIO_Port GPIOC
#define MIC_POWER_CTRL_PIN _P(C,0)

#define NO_HW_BREAKPOINTS
#ifdef NO_HW_BREAKPOINTS
#define __BKPT() ((void)0)
#endif


/*
At given settings:

MONO HALF BYTES: 32000 bytes
RAW AUD LEN: 32000 samples (full)
RAW HALF DMA BYTES: 64000 bytes

DMA buffers should be in D2, same domain as I2S3

Place processing buffer (mono_data) in D1 AXI
Reserve DTCM for stack and misc
*/

// Mono-channel data packed to 24-bit
static float mono_data[FRAME_SIZE+FFT_SIZE] __attribute__ (( section (".ram_d2"), aligned(32) ));

// Stereo-channel 32-bis per frame (note mic data is 24-bit)
// 32 kHz * 2 channels * 2 seconds * 4 bytes per sample = 500 kByte
// Only could fit in ext_sram or use up almost all the D1, but save the D1 for higher performance stuff
//static int32_t raw_data[RAW_AUD_LEN]     __attribute__ (( section (".ext_sram"), aligned(32) ));
static int32_t raw_data[RAW_AUD_LEN/2]     __attribute__ (( section (".ext_sram"), aligned(32) )); // ARGH the DMA can't transfer > 1s @ 32 kHz per half, so we have to quarter it =(
static float raw_data_dma_extra[RAW_AUD_LEN/8]     __attribute__ (( section (".ram_d2"), aligned(32) )); // extra side buffer

// Mono-sized buffers for sending data through the filters
static float filter_buf0[FILTER_BUF_SZ_SAMPS] __attribute__ (( section (".ram_d1"), aligned(32) ));
static float filter_buf1[FILTER_BUF_SZ_SAMPS] __attribute__ (( section (".ram_d1"), aligned(32) ));

// ML outputs
static myTypeA output_swapped __attribute__ (( section (".ram_d2") )); // 64 x 51 floats
static float upstream_out[256] __attribute__ (( section (".ram_d2") ));
static float class_out_data[8] __attribute__ (( section (".ram_d2") ));
static volatile uint32_t do_send; // flag
static bool is_processing; // flag

typedef enum { MIC_OFF, MIC_ON } mic_power_state_t;
static void mic_power_ctrl(mic_power_state_t x) {
	switch(x) {
		case MIC_OFF: HAL_GPIO_WritePin(MIC_OFF_GPIO_Port, MIC_OFF_Pin, GPIO_PIN_SET);   break;
		case MIC_ON:  HAL_GPIO_WritePin(MIC_OFF_GPIO_Port, MIC_OFF_Pin, GPIO_PIN_RESET); break;
		default: __BKPT();
	}
}

static void decimate_32_to_8(float *dest, float *src, unsigned in_len) {
	if (dest == NULL || src == NULL || in_len % 4 != 0) { __BKPT(); return; }

	for(int i=0,j=0; i<in_len; i += 4, j++) {
		dest[j] = src[i];
	}
}

static uint64_t start_stopwatch(void) {
	return lptim_get_counter_us_fast();
}

static uint32_t stop_stopwatch_ms(uint64_t start) {
	uint64_t now = lptim_get_counter_us_fast();
	now = (now - start)/1000; // to ms
	return now&0xFFFFFFFF;
}

static uint64_t stop_stopwatch_us(uint64_t start) {
	uint64_t now = lptim_get_counter_us_fast();
	now = (now - start); // us
	return now;
}

#define ICS43434_AOP_DBSPL 120.0
static float compute_spl_db(int32_t *x, uint32_t len) {
	double rms=0.0;
	for(int i=0; i<len; i++) {
		// sample will be range [-1 to 1)
		float sample = (float)x[i]/(1<<23);
		rms += sample*sample;
	}
	rms = rms / len;
	rms = sqrt(rms);
	// convert to dB
	rms = 20.0*log10(rms) + ICS43434_AOP_DBSPL;
	return (float)rms;
}

static float compute_spl_db_float(float *x, uint32_t len) {
	double rms=0.0;
	for(int i=0; i<len; i++) {
		// sample will be range [-1 to 1)
		rms += x[i]*x[i];
	}
	rms = rms / len;
	rms = sqrt(rms);
	// convert to dB
	rms = 20.0*log10(rms) + ICS43434_AOP_DBSPL;
	return (float)rms;
}

static float * my_ai_process(float *data) {
	// Upstream
	int res;
	res = aiRun(data, upstream_out);
	if (res) { debug_printf("!!ERROR!!!\r\n"); __BKPT(); }

	// Downstream
	res = aiRun2(upstream_out, class_out_data);
	if (res) { debug_printf("!!ERROR!!!\r\n"); __BKPT(); }

	return class_out_data;
}

static float dBSPL;
static float dBSPL_thresh = -100;
float get_db_spl(void) { return dBSPL; }
void set_dBSPL_thresh(float x) { dBSPL_thresh = x; }

float * get_ml_upstream(void) {
	return upstream_out;
}

float * get_ml_downstream(void) {
	return class_out_data;
}

void start_microphone(void) {
	HAL_StatusTypeDef ret;
	mic_power_ctrl(MIC_ON); // on by default
	ret = HAL_I2S_Receive_DMA(&hi2s3, (uint16_t *)raw_data, RAW_AUD_LEN/2);
	if (ret != HAL_OK) __BKPT();
}

void stop_microphone(void) {
	HAL_I2S_DMAStop(&hi2s3);
	mic_power_ctrl(MIC_OFF);
}

void ManagedAICallback(UINT32 arg1, UINT32 arg2);

extern void MaxSystemClock_Config(void);
extern void MinSystemClock_Config(void);

#ifndef ML_RUN_MODULO_DEFAULT
#define ML_RUN_MODULO_DEFAULT 4
#endif

uint32_t ml_run_modulo = ML_RUN_MODULO_DEFAULT;
void set_ml_modulo(uint32_t x) { ml_run_modulo = x; }
uint32_t get_ml_modulo(void) { return ml_run_modulo; }

// At this point, 32 kHz mono data is in float filter_buf0[]
static void mic_data_callback(void) {
	static uint32_t ml_run_idx = 0;
#ifdef MKII_AUDIO_PROCESSING_DEBUG_STATS
	static uint64_t last;
	uint64_t now;
#endif

#ifdef MKII_AUDIO_PROCESSING_DEBUG_STATS
	now = start_stopwatch();
#endif

#ifdef DO_COMP_AND_A_WEIGHT_FILTERS // Do the filtering...
	sonyc_comp_filter_go(filter_buf0, filter_buf1, FILTER_BUF_SZ_SAMPS); // Mic compensated data is in filter_buf1
#if (IIR_FILTER_SKIP > 0)
	memset(filter_buf1, 0, IIR_FILTER_SKIP*sizeof(float)); // Clear the start of the filtered data which is garbage due to FIR delay
#endif

	// Only do this if we are going to do ML step
	if (ml_run_idx % ml_run_modulo == 0) {
		decimate_32_to_8(&mono_data[HALF_FFT], filter_buf1, FILTER_BUF_SZ_SAMPS); // mono-data now contains 8 kHz data for ML
	}

	//sonyc_iir_filter_reset(); // not sure if good idea... given that we have duty cycling (but also skipping...) ???
	sonyc_iir_filter_go(&filter_buf1[IIR_FILTER_SKIP], filter_buf0, FILTER_BUF_SZ_SAMPS-IIR_FILTER_SKIP*2); // Data for SPL computation is now in filter_buf0
#else // #ifdef DO_COMP_AND_A_WEIGHT_FILTERS
	if (ml_run_idx % ml_run_modulo == 0) {
		decimate_32_to_8(&mono_data[HALF_FFT], filter_buf0, FILTER_BUF_SZ_SAMPS); // Mono-data now contains 8 kHz data for ML
	}
#endif

	// Compute db SPL with SPL_CALC_SAMPLE_SKIP just because I'm a little paranoid around the edges
	dBSPL = compute_spl_db_float(&filter_buf0[SPL_CALC_SAMPLE_SKIP], ARRAY_LEN(filter_buf0)-SPL_CALC_SAMPLE_SKIP*2);

	if (ml_run_idx++ % ml_run_modulo != 0) {
		ManagedAICallback(1,0); // Indicates only SPL update
		goto out;
	}

	if (dBSPL < dBSPL_thresh) {
#ifdef MKII_AUDIO_PROCESSING_DEBUG_STATS
		hal_printf("SPL of %d below processing threshold, skipping\r\n", (int)dBSPL);
#endif
		ManagedAICallback(1,0);
		goto out; // Below threshold, do not run ML
	}

	// Get Mels from all 51 hops
	// Note transpose step to match ML input
	for(int i=0; i<NUM_HOPS; i++) {
		float *energies = mfcc_test_float(&mono_data[i*HOP_SIZE]);
		for(int j=0; j<NUM_BINS; j++) {
			output_swapped[j][i] = energies[j];
		}
	}
	amp_to_db((float *)output_swapped, NUM_BINS*NUM_HOPS);
	my_ai_process((float *)output_swapped);
	ManagedAICallback(0,0);
#ifdef MKII_AUDIO_PROCESSING_DEBUG_STATS
	hal_printf("Audio Processing Took %lu ms dBSPL was %d dB\r\n", stop_stopwatch_ms(now), (int)(dBSPL));
	hal_printf("Time since last frame: %lu\r\n", stop_stopwatch_ms(last));
	last = now;
#endif
out:
	return;
}

static void stereo_to_mono_float(int32_t *s, float *m, uint32_t s_len_bytes) {
	unsigned mono_samples = s_len_bytes / 8; // 4 bytes each, every other sample

#ifdef _DEBUG // Sanity checks
	if (s == NULL || m == NULL || s_len_bytes == 0) { /*__BKPT();*/ return; }
	if (s_len_bytes % 4 != 0) { /*__BKPT();*/ return; }
#endif

	// Because DMA happens outside cache visibility
	SCB_InvalidateDCache_by_Addr(s, s_len_bytes);

	// Sign extend from
	// http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
	// This compiles to SBFX instruction which seems optimal
	for(int i=0; i < mono_samples; i++) {
		struct {int32_t x:24;} temp;
		int32_t x = s[i*2];
		temp.x = x;
		m[i] = (float)(temp.x)/(1<<23);
	}

}

// s = stereo source, m = mono dest, s_len stereo data length bytes
static void stereo_to_mono(int32_t *s, int32_t *m, uint32_t s_len_bytes) {
	unsigned mono_samples = s_len_bytes / 8; // 4 bytes each, every other sample

#ifdef _DEBUG // Sanity checks
	if (s == NULL || m == NULL || s_len_bytes == 0) { /*__BKPT();*/ return; }
	if (s_len_bytes % 4 != 0) { /*__BKPT();*/ return; }
#endif

	// Because DMA happens outside cache visibility
	//SCB_InvalidateDCache_by_Addr(s, s_len_bytes);

	// Sign extend from
	// http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
	// This compiles to SBFX instruction which seems optimal
	for(int i=0; i < mono_samples; i++) {
		struct {int32_t x:24;} temp;
		int32_t x = s[i*2];
		m[i] = temp.x = x;
	}
}

static void mic_err_do(void *p) {
	hal_printf("Mic dropped frame or error\r\n");
}

/*
	OK this sucks...
	At 32 kHz sampling we cannot transfer 1-second of data in a single DMA shot (half transfer) as we could at 8 kHz.
	So instead of 1-second half transfers we must use 0.5 second half transfers and stitch them together.
	This requires an extra side buffer, 'raw_data_dma_extra' which is needed so that we don't stomp on 'filter_buf0' while in use.
	Note that stereo_to_mono_float() is doing a lot of work: removes unused channel, sign extends, converts to float, and copys to processing buffer
*/
static void mic_cont_do(void *p) {

	// Copy 16k samples to side buffer (because processing buffer might be in use)
	if (do_send == 1) {
		stereo_to_mono_float(&raw_data[0], raw_data_dma_extra, RAW_HALF_DMA_BYTES/2);
		do_send = 0;
	}
	// Processing buffer *must* be free now so copy both the previous side buffer and the new data to procsesing buffer and ship it
	else if (do_send == 2) {
		// Burst to max clock speed a little earlier...
		GLOBAL_LOCK(irq);
		MaxSystemClock_Config();
		irq.Release();
		is_processing = true;
		memcpy(filter_buf0, raw_data_dma_extra, sizeof(raw_data_dma_extra));
		stereo_to_mono_float(&raw_data[RAW_AUD_LEN/4], &filter_buf0[RAW_AUD_LEN/8], RAW_HALF_DMA_BYTES/2);
		do_send = 0; // Can flag now because of side buffer in next round
		mic_data_callback();
		is_processing = false;
		irq.Acquire();
		MinSystemClock_Config();
	}
}

// IRQs
extern "C" {
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s){
	//mic_cont.Enqueue();
	__BKPT();
	mic_error_cont.Enqueue();
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	if (do_send != 0) {
		//__BKPT();
		mic_error_cont.Enqueue();
		return; // Abort. Behind schedule?
	}
	do_send = 1;
	mic_cont.Enqueue();
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
	if (do_send != 0 || is_processing) {
		//__BKPT();
		mic_error_cont.Enqueue();
		return; // Abort. Behind schedule?
	}
	do_send = 2;
	mic_cont.Enqueue();
}

void DMA1_Stream0_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hdma_spi3_rx);
}
} // extern "C" {

void HAL_I2S_MspInit(I2S_HandleTypeDef* i2sHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(i2sHandle->Instance==SPI3) {
		__HAL_RCC_SPI3_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**I2S3 GPIO Configuration
		PA4     ------> I2S3_WS
		PC10     ------> I2S3_CK
		PC11     ------> I2S3_SDI
		PC12     ------> I2S3_SDO
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/* I2S3 DMA Init */
		/* SPI3_RX Init */
		hdma_spi3_rx.Instance = DMA1_Stream0;
		hdma_spi3_rx.Init.Request = DMA_REQUEST_SPI3_RX;
		hdma_spi3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_spi3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_spi3_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_spi3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma_spi3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		hdma_spi3_rx.Init.Mode = DMA_CIRCULAR;
		hdma_spi3_rx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_spi3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		if (HAL_DMA_Init(&hdma_spi3_rx) != HAL_OK)
			__BKPT();
		__HAL_LINKDMA(i2sHandle,hdmarx,hdma_spi3_rx);
	}
}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef* i2sHandle)
{
	if(i2sHandle->Instance==SPI3) {
		__HAL_RCC_SPI3_CLK_DISABLE();
		/**I2S3 GPIO Configuration
		PA4     ------> I2S3_WS
		PC10     ------> I2S3_CK
		PC11     ------> I2S3_SDI
		PC12     ------> I2S3_SDO
		*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12);
		HAL_DMA_DeInit(i2sHandle->hdmarx);
	}
}

static void MX_DMA_Init(void) {
	__HAL_RCC_DMA1_CLK_ENABLE();
	HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}

BOOL I2S_Internal_Initialize() {
	MX_DMA_Init();
	CPU_GPIO_EnableOutputPin(MIC_POWER_CTRL_PIN, FALSE);
	hi2s3.Instance = SPI3;
	hi2s3.Init.Mode = I2S_MODE_MASTER_RX;
	hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
	hi2s3.Init.DataFormat = I2S_DATAFORMAT_24B;
	hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	hi2s3.Init.AudioFreq = SAMP_RATE_HZ;
	hi2s3.Init.CPOL = I2S_CPOL_LOW;
	hi2s3.Init.FirstBit = I2S_FIRSTBIT_MSB;
	hi2s3.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
	hi2s3.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
	hi2s3.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_DISABLE;
	if (HAL_I2S_Init(&hi2s3) != HAL_OK) { /*__BKPT();*/ return FALSE; }
	do_send = 0;
	is_processing = false;
	mic_power_ctrl(MIC_ON);
	MX_X_CUBE_AI_Init();
	mfcc_init();
	memset(mono_data, 0, sizeof(mono_data));
	mic_cont.InitializeCallback(mic_cont_do, NULL);
	mic_error_cont.InitializeCallback(mic_err_do, NULL);
	memset(filter_buf0, 0, sizeof(filter_buf0));
	memset(filter_buf1, 0, sizeof(filter_buf1));
	return TRUE;
}

// Not used
void I2S_Test() {
	MX_X_CUBE_AI_Init();
	mfcc_init();
}

BOOL I2S_Internal_Uninitialize() { stop_microphone(); }

#ifdef I2S_FUNC_CPP_DEBUG
#pragma GCC pop_options
#endif
