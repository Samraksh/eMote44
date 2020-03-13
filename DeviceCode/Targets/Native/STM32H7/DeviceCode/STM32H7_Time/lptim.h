#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LPTIM_TICKS 0xFFFF

typedef enum {
	lptim_err_none	= 0,
	lptim_err_busy	= -1,
	lptim_err_short	= -2,
	lptim_err_long	= -3,
	lptim_err_inval	= -4,
} lptim_err;

typedef void (*lptim_cmp_cb_fn)(void *);
typedef struct lptim_task lptim_task_t;

enum { LPTIM_VT=0, LPTIM_DEBUG=1 };

struct lptim_task {
	uint16_t delay_ms;
	uint32_t ticks;
	lptim_task_t *next;
	void *data;	  // pointer to data to be used as needed
	void *contin; // HAL_CONTINUATION, for which we need to bring in C++, so re-cast for now
	lptim_cmp_cb_fn isr_cb; // ISR context code
};

void MX_LPTIM_Init(void);
uint64_t lptim_get_counter_us(int lptim);

// task stuff, not VT
bool task_is_linked(lptim_task_t *x);
void lptim_task_init(lptim_task_t *x);
int  lptim_add_oneshot(lptim_task_t *x);
void lptim_task_cb(void);

// Generic stuff
int lptim_set_delay_ticks(uint16_t ticks, int lptim);
int lptim_set_delay_ms(uint32_t ms, int lptim);
int lptim_set_delay_us(uint32_t us, int lptim);
int lptim_set_compare_ticks(uint16_t ticks, bool is_next_epoch, int lptim);
int lptim_set_compare_dticks(uint16_t dticks, int lptim);

// The VT specific handler
void lptimIRQHandler(void);

#ifdef __cplusplus
}
#endif
