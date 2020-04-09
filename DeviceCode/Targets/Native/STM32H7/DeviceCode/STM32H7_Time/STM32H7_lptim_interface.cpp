#include <tinyhal.h>
#include "STM32H7_time_functions.h"

#define GPIO_0 _P(B,12)
#define GPIO_1 _P(B,13)

#ifdef __cplusplus
extern "C" {
#endif

#include "lptim.h"

void LptimInit(){
	// Starts all LPTIM
	MX_LPTIM_Init();
}

void lptimIRQHandler(void){
	queueLptimCallback();
}

int callLptimSetCompareMicroseconds(uint32_t us){
	int retCode = 0;
	
		retCode = lptim_set_delay_us(us, LPTIM_VT);
		if (retCode == lptim_err_long) { // Desired intervall too long. Default to max interval.
			retCode = lptim_set_vt_fire_on_roll();
			return retCode;
		}
		else
		if (retCode == lptim_err_short){
			retCode = lptim_set_delay_us(MIN_LPTIM_COUNT, LPTIM_VT);
			return retCode;
		}
		int timeout = 10;
		while ( (retCode != lptim_err_none) && (timeout-- >0) ){
			retCode = lptim_set_delay_us(us, LPTIM_VT);
		}
		if (timeout == 0) {
			lptimIRQHandler();
		   	return -1;
		}

	return 0;
}

int callLptimSetCompare(uint16_t ticks, bool is_next_epoch){
	return lptim_set_compare_ticks(ticks, is_next_epoch, LPTIM_VT);
}

uint64_t requestLptimCounter(void){
	return lptim_get_counter_us(LPTIM_VT);
}

#ifdef __cplusplus
}
#endif

