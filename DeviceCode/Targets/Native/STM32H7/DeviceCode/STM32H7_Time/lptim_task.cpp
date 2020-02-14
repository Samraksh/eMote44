#include <stdint.h>
#include <string.h>
#include "lptim.h"

#include <tinyhal.h>

static lptim_task_t * volatile task_HEAD = NULL;
static lptim_task_t * volatile task_RUNNING = NULL;
static volatile unsigned task_list_size = 0;

// TODO: CONSIDER LOCKING ETC


bool task_is_linked(lptim_task_t *x) {
	lptim_task_t *HEAD;
	for(HEAD = task_HEAD; HEAD != NULL; HEAD = HEAD->next) {
		if (x == HEAD) return true;
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


static void unlink_lptim_task(lptim_task_t *x) {
	lptim_task_t *t = task_HEAD;
	lptim_task_t *p = NULL;

	if (x == NULL || t == NULL)  { __BKPT(); goto out_fail; }

	// Find element
	while (t != x && t != NULL) {
		p = t;
		t = t->next;
	}

	if (t == NULL) { __BKPT(); goto out_fail; } // not found in list

	// Snip
	if (p == NULL) task_HEAD = t->next; // New HEAD node
	else p->next = t->next;

	x->next = NULL; // Explicit clear to be safe

	if (x == task_RUNNING) task_RUNNING = NULL;
	task_list_size--;
out_fail:
	return;
}

// TODO: ONLY DOES delay_ms FOR NOW
// BIG TODO: DOES NOT ACCOUNT FOR TIME ELAPSED
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
	ret = lptim_set_delay_ms(next->delay_ms, LPTIM_DEBUG);
	if (ret) __BKPT();
	task_RUNNING = next;
}

// ISR
void lptim_task_cb() {
	lptim_task_t *task = task_RUNNING;
	if (task_RUNNING == NULL) return; // Shouldn't happen
	unlink_lptim_task(task_RUNNING);  // Do this early in case cb re-queues

	// run the ISR task
	if (task->isr_cb != NULL) {
		lptim_cmp_cb_fn cb;
		cb = *(task->isr_cb);
		cb(task->data);
	}

	// Queue continuation
	if (task->contin != NULL) {
		HAL_CONTINUATION *contin = (HAL_CONTINUATION *)task->contin;
		contin->Enqueue();
	}
}
