#include <stdint.h>
#include <string.h>
#include "lptim.h"

#include <tinyhal.h>

static lptim_task_t * volatile task_HEAD = NULL;
static lptim_task_t * volatile task_RUNNING = NULL;
static volatile unsigned task_list_size = 0;

// TODO: CONSIDER LOCKING ETC


bool task_is_linked(lptim_task_t *x) {
	lptim_task_t *HEAD = task_HEAD;
	for(unsigned i=0; i<task_list_size; i++) {
		if (HEAD == NULL) __BKPT(); // Shouldn't happen
		if (x == HEAD) return true;
		HEAD = HEAD->next;
	}
	return false;
}

// Fast append to front
static void add_lptim_task_front(lptim_task_t *x) {
	lptim_task_t *prev = task_HEAD;
	if (task_is_linked(x)) return; // Do nothing if node is already present
	task_HEAD = x;
	x->next = prev;
	task_list_size++;
}

// TODO: SIMPLE CASE ONLY
static void delete_lptim_task(lptim_task_t *x) {
	// Root check
	task_list_size--;
	if( x == task_HEAD && x->next == NULL ) {
		task_HEAD = NULL; return;
	}
}

// TODO: ONLY DOES delay_ms FOR NOW
static lptim_task_t * get_next_task() {
	lptim_task_t *min = task_HEAD;
	lptim_task_t *t;

	if (min->next == NULL) return min; // trivial case, one item

	for(t = min->next; t != NULL; t = t->next) {
		if((t->delay_ms) < (min->delay_ms))
			min = t;
	}
	return min;
}

// Simple zero'ing
void lptim_task_init(lptim_task_t *x) {
	memset(x, 0, sizeof(lptim_task_t));
}

// Public facing function
int lptim_add_oneshot(lptim_task_t *x) {
	lptim_task_t *next;
	int ret;

	if (x == NULL) return -1;
	add_lptim_task_front(x);

	next = get_next_task();
	ret = set_lptim_set_delay_ms(next->delay_ms);
	if (ret) __BKPT();
	task_RUNNING = next;
}

// ISR
void lptim_task_cb() {
	if (task_RUNNING == NULL) return; // Shouldn't happen
	delete_lptim_task(task_RUNNING);  // Do this early in case cb re-queues

	// run the ISR task
	if (task_RUNNING->isr_cb != NULL) {
		lptim_cmp_cb_fn task;
		void *data;

		task = *(task_RUNNING->isr_cb);
		data = task_RUNNING->data;
		task(data);
	}

	// Queue continuation
	if (task_RUNNING->contin != NULL) {
		HAL_CONTINUATION *contin = (HAL_CONTINUATION *)task_RUNNING->contin;
		contin->Enqueue();
	}

	task_RUNNING = NULL;
}
