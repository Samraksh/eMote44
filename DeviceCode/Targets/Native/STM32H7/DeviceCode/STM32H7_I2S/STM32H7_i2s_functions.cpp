#include <tinyhal.h>
//#include <limits.h>

#include <Samraksh\serial_frame_pal.h>
#include <Samraksh\SONYC_ML\sonyc_ml.h>

#include <STM32H7_Time\lptim.h>

#define debug_printf hal_printf

//#define SEND_AUDIO_DATA_ONLY

static I2S_HandleTypeDef hi2s3;
static DMA_HandleTypeDef hdma_spi3_rx;

static void stereo_to_mono(uint32_t *s, int32_t *m, uint32_t s_len_bytes) __attribute__ ((unused));

// DMA audio parameters
#define SAMPLE_DMA_SEC 2 // seconds to be buffered per FULL dma transfer
#define SAMP_RATE_HZ 8000UL

#define MONO_DATA_BYTES (SAMP_RATE_HZ/2 * SAMPLE_DMA_SEC * 3) // HALF transfer size, post-fixup
#define RAW_AUD_LEN (SAMP_RATE_HZ * SAMPLE_DMA_SEC * 2) // FULL DMA transfer size, in SAMPLES, pre-fixup
#define RAW_HALF_DMA_BYTES (RAW_AUD_LEN * 4 / 2) // 4 bytes per sample, HALF transfer

#if ( (RAW_AUD_LEN * 4) % 32 != 0)
	#error "raw_data[] size must be % 32 for DMA"
#endif

#define MIC_OFF_Pin GPIO_PIN_0
#define MIC_OFF_GPIO_Port GPIOC
#define MIC_POWER_CTRL _P(C,0)

typedef enum { MIC_OFF, MIC_ON } mic_power_state_t;
static void mic_power_ctrl(mic_power_state_t x) __attribute__ ((unused));
static void mic_power_ctrl(mic_power_state_t x) {
	switch(x) {
		case MIC_OFF: HAL_GPIO_WritePin(MIC_OFF_GPIO_Port, MIC_OFF_Pin, GPIO_PIN_SET);   break;
		case MIC_ON:  HAL_GPIO_WritePin(MIC_OFF_GPIO_Port, MIC_OFF_Pin, GPIO_PIN_RESET); break;
		default: __BKPT();
	}
}

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
#ifdef SEND_AUDIO_DATA_ONLY
static uint8_t mono_data[MONO_DATA_BYTES] __attribute__ (( section (".ram_d1"), aligned(32) ));
#define mono_data_padded NULL
#else
static int32_t mono_data[FRAME_SIZE+FFT_SIZE] __attribute__ (( section (".ram_d1"), aligned(32) ));
#endif

// Stereo-channel 32-bis per frame (note mic data is 24-bit)
static int32_t raw_data[RAW_AUD_LEN]     __attribute__ (( section (".ram_d2"), aligned(32) ));

static volatile uint32_t do_send; // flag
static bool isInit;

static uint64_t start_time;

static inline uint64_t start_stopwatch(void) {
	return lptim_get_counter_us_fast();
}

static inline uint32_t stop_stopwatch_ms(uint64_t start) {
	uint64_t now = lptim_get_counter_us_fast();
	now = (now - start)/1000; // to ms
	return now&0xFFFFFFFF;
}

static inline uint64_t stop_stopwatch_us(uint64_t start) {
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

#ifndef SEND_AUDIO_DATA_ONLY
static float upstream_out[256] __attribute__ (( section (".ram_d1") ));
static float class_out_data[8] __attribute__ (( section (".ram_d1") ));
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
#endif

#ifndef SEND_AUDIO_DATA_ONLY
static myTypeA output_swapped __attribute__ (( section (".ram_d1") )); // 64 x 51 floats
static const char * class_to_string(int x) {
	switch(x) {
		case 0: return "engine";
		case 1: return "machinery-impact";
		case 2: return "non-machinery-impact";
		case 3: return "powered-saw";
		case 4: return "alert-signal";
		case 5: return "music";
		case 6: return "human-voice";
		case 7: return "dog";
		default: return "BAD CLASS";
	}
}
#endif
// Padded version is for MFCC + ML processing
static void mic_data_callback(void *buf, unsigned len) {
	static bool doneOne = false;
	if (!doneOne) { doneOne = true; return; } // Skip the initial Mic output as it contains garbage
#ifdef SEND_AUDIO_DATA_ONLY
	//HAL_I2S_DMAStop(&hi2s3);
	send_framed_serial_data((uint8_t *)buf, len, FRAME_TYPE_BIN_AUDIO);
#else
	int ret;
	float *my_classes;
	float dBSPL;
	int32_t dBSPL_print;
	int32_t *my_raw_data = (int32_t *) buf;
	uint64_t end_time, proc_end;
	uint32_t diff_time_ms, proc_end_ms;

	// Compute db SPL
	dBSPL = compute_spl_db(my_raw_data, len/sizeof(int32_t));
	dBSPL_print = (int32_t) dBSPL;

	// Get Mels from all 51 hops
	// Note transpose step to match ML input
	for(int i=0; i<NUM_HOPS; i++) {
		float *energies = mfcc_test(&my_raw_data[i*HOP_SIZE]);
		for(int j=0; j<NUM_BINS; j++) {
			output_swapped[j][i] = energies[j];
		}
	}
	amp_to_db((float *)output_swapped, NUM_BINS*NUM_HOPS);
	proc_end_ms = stop_stopwatch_ms(start_time);
	my_classes = my_ai_process((float *)output_swapped);
	ret = maxarg_float((const float *)my_classes, 8);
	diff_time_ms = stop_stopwatch_ms(start_time);
	switch(ret) {
		case 0:  hal_printf("Classified: %s (%d)\t\t\tdBSPL: %ld  took: %lums  non-ML: %lums\r\n", 	class_to_string(ret), ret, dBSPL_print, diff_time_ms, proc_end_ms); break;
		case 1:  hal_printf("Classified: %s (%d)\tdBSPL: %ld  took: %lums  non-ML: %lums\r\n", 		class_to_string(ret), ret, dBSPL_print, diff_time_ms, proc_end_ms); break;
		default: hal_printf("Classified: %s (%d)\t\tdBSPL: %ld  took: %lums  non-ML: %lums\r\n", 	class_to_string(ret), ret, dBSPL_print, diff_time_ms, proc_end_ms);
	}
#endif
}

// s = stereo source, m = mono dest, s_len stereo data length bytes
static void stereo_to_mono(int32_t *s, int32_t *m, uint32_t s_len_bytes) {
	unsigned mono_samples = s_len_bytes / 8; // 4 bytes each, every other sample

#ifdef _DEBUG
	// Sanity checks
	if (s == NULL || m == NULL || s_len_bytes == 0) { __BKPT(); return; }
	if (s_len_bytes % 4 != 0) { __BKPT(); return; }
#endif

	// Because DMA happens outside cache visibility
	SCB_InvalidateDCache_by_Addr(s, s_len_bytes);

	// Sign extend from
	// http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
	// This compiles to SBFX instruction which seems optimal
	for(int i=0; i < mono_samples; i++) {
		struct {int32_t x:24;} temp;
		int32_t x = s[i*2];
		m[i] = temp.x = x;
	}
}


static void wait_one(void) {
	while( HAL_CONTINUATION::Dequeue_And_Execute() == TRUE ) ;
	__disable_irq();
	if (do_send == 0) __WFI();
	__enable_irq();
}

void mic_test(void) __attribute__ ((noreturn));
#ifndef SEND_AUDIO_DATA_ONLY
void mic_test(void) {
	HAL_StatusTypeDef ret;
	do_send = 0;

	if (isInit == false) __BKPT();
	memset(mono_data, 0, sizeof(mono_data));

	mic_power_ctrl(MIC_ON);
	debug_printf("Enabled mic power\r\n");

	ret = HAL_I2S_Receive_DMA(&hi2s3, (uint16_t *)raw_data, RAW_AUD_LEN);
	if (ret != HAL_OK) { __BKPT(); goto out_mic_test_bad; }
	debug_printf("I2S sampling started\r\n");

	while (1) {
		if (do_send == 0) wait_one();
		else if (do_send == 1) {
			start_time = start_stopwatch();
			stereo_to_mono(&raw_data[0], &mono_data[HALF_FFT], RAW_HALF_DMA_BYTES);
			do_send = 3; // flag send needed
		}
		else if (do_send == 2) {
			start_time = start_stopwatch();
			stereo_to_mono(&raw_data[RAW_AUD_LEN/2], &mono_data[HALF_FFT], RAW_HALF_DMA_BYTES);
			do_send = 3; // flag send needed
		}
		// Send on prepared data
		else if (do_send == 3) {
			mic_data_callback(mono_data, MONO_DATA_BYTES);
			do_send = 0; // flag data processed
		}
	}

out_mic_test_bad:
	debug_printf("Fatal Error... aborting\r\n");
	while(1) { wait_one(); }
}
#else
void mic_test(void) {
	HAL_StatusTypeDef ret;
	do_send = 0;

	if (isInit == false) __BKPT();

	mic_power_ctrl(MIC_ON);
	debug_printf("Enabled mic power\r\n");

	ret = HAL_I2S_Receive_DMA(&hi2s3, (uint16_t *)raw_data, RAW_AUD_LEN);
	if (ret != HAL_OK) { __BKPT(); goto out_mic_test_bad; }
	debug_printf("I2S sampling started\r\n");
	debug_printf("Output: Signed 24-bit PCM Mono @ %lu Hz\r\n", SAMP_RATE_HZ);

	while (1) {
		if (do_send == 0) wait_one();
		else if (do_send == 1) {
			SCB_InvalidateDCache_by_Addr(raw_data, RAW_HALF_DMA_BYTES);
			for (int i=0,j=0; j<MONO_DATA_BYTES; j+=12, i+=8) {
				memcpy(&mono_data[j+0], (uint8_t *) &raw_data[i+0], 3);
				memcpy(&mono_data[j+3], (uint8_t *) &raw_data[i+2], 3);
				memcpy(&mono_data[j+6], (uint8_t *) &raw_data[i+4], 3);
				memcpy(&mono_data[j+9], (uint8_t *) &raw_data[i+6], 3);
			}
			do_send = 3; // flag send needed
		}
		else if (do_send == 2) {
			SCB_InvalidateDCache_by_Addr(&raw_data[RAW_AUD_LEN/2], RAW_HALF_DMA_BYTES);
			for (int i=RAW_AUD_LEN/2,j=0; j<MONO_DATA_BYTES; j+=12, i+=8) {
				memcpy(&mono_data[j+0], (uint8_t *) &raw_data[i+0], 3);
				memcpy(&mono_data[j+3], (uint8_t *) &raw_data[i+2], 3);
				memcpy(&mono_data[j+6], (uint8_t *) &raw_data[i+4], 3);
				memcpy(&mono_data[j+9], (uint8_t *) &raw_data[i+6], 3);
			}
			do_send = 3; // flag send needed
		}

		// Send on prepared data
		else if (do_send == 3) {
			mic_data_callback(mono_data, MONO_DATA_BYTES);
			do_send = 0; // flag data processed
		}
	}

out_mic_test_bad:
	debug_printf("Fatal Error... aborting\r\n");
	while(1) { __WFI(); }
}
#endif // #ifndef SEND_AUDIO_DATA_ONLY

// IRQs
extern "C" {
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s){
	__BKPT();
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	if (do_send != 0) __BKPT();
	do_send = 1;
	__DMB();
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
	if (do_send != 0) __BKPT();
	do_send = 2;
	__DMB();
}

void DMA1_Stream0_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hdma_spi3_rx);
}

// Not used
// void SPI3_IRQHandler(void) {
  // HAL_I2S_IRQHandler(&hi2s3);
// }
}

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
	CPU_GPIO_EnableOutputPin(MIC_POWER_CTRL, FALSE);
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
	if (HAL_I2S_Init(&hi2s3) != HAL_OK)
		__BKPT();
	isInit = true;
}

void I2S_Test() {
#ifndef SEND_AUDIO_DATA_ONLY
	MX_X_CUBE_AI_Init();
	mfcc_init();
#endif
	mic_test();
}

BOOL I2S_Internal_Uninitialize() {

}