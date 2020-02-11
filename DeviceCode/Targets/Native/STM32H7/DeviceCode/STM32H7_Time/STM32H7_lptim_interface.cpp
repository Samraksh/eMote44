#include <tinyhal.h>
#include "STM32H7_time_functions.h"

#define GPIO_0 _P(B,12)
#define GPIO_1 _P(B,13)

#ifdef __cplusplus
extern "C" {
#endif

#include "lptim.h"


void LptimInit(){
	MX_LPTIM_Init();
}

void lptimIRQHandler(void){
	queueLptimCallback();
}

int callLptimSetCompareMicroseconds(uint32_t us){
	int retCode = 0;
	if (us < 400){
		lptimIRQHandler();
		return 0;
	} else {
		retCode = set_lptim_set_delay_us(us);
		if (retCode == lptim_err_short){
			lptimIRQHandler();
			return 0;
		}
		int timeout = 10;
		while ( (retCode != lptim_err_none) && (timeout-- >0) ){
			retCode = set_lptim_set_delay_us(us);
		}
		if (timeout == 0) {
			lptimIRQHandler();
		   	return -1;
		}
	}
	return 0;
}

int callLptimSetCompare(uint16_t ticks, bool is_next_epoch){
	return lptim_set_compare_ticks(ticks, is_next_epoch);
}

uint64_t requestLptimCounter(void){
	return my_get_counter_lptim_us();
}

#ifdef __cplusplus
}
#endif

