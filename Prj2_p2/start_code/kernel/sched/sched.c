#include "irq.h"
#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

#define HIGHEST_PRIORITY 6

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;
pcb_t *next_running;

/* global process id */
pid_t process_id = 0;

int block_accur = FALSE;
queue_t ready_queue;
queue_t block_queue;
queue_t sleep_queue;

int cpu_priority = HIGHEST_PRIORITY;

static void check_sleeping()
{
	void* check = sleep_queue.head;
	while (check != NULL) {
		void* next_check = ((pcb_t*)check)->next;
		if (((pcb_t*)check)->current_counter >= ((pcb_t*)check)->target_counter) {
			next_check = queue_remove(&sleep_queue,check);
			queue_push(&ready_queue,check);
		}
		check = next_check;
	}
}

static void get_bonus()
{
	void* check = block_queue.head;
	while (check != NULL) {
		((pcb_t*)check)->bonus += 1;
		check = ((pcb_t*)check)->next;
	}
}

void scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.
	current_running->cursor_x = screen_cursor_x;
	current_running->cursor_y = screen_cursor_y;
	
	check_sleeping();
	
	if (block_accur == TRUE) {
		current_running = next_running;
		block_accur = FALSE;
	}
	else current_running = (pcb_t*)current_running->next;
	if (current_running == NULL) {
		current_running = ready_queue.head;
		cpu_priority--;
		if (cpu_priority == 0) {
			cpu_priority = HIGHEST_PRIORITY;
			get_bonus();
		}
	}
	while (current_running->priority + current_running->bonus < cpu_priority) {
		current_running = (pcb_t*)current_running->next;
		if (current_running == NULL) {
			current_running = ready_queue.head;
			cpu_priority--;
			if (cpu_priority == 0) {
				cpu_priority = HIGHEST_PRIORITY;
				get_bonus();
			}
		}
	}
	process_id = current_running->pid;
	
	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;
}

void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
	current_running->target_counter = sleep_time * 100 * 150000 / TIMER_INTERVAL;
	current_running->current_counter = 0;
	do_block(&sleep_queue);
	do_scheduler();
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue
	next_running = (pcb_t*)queue_remove(&ready_queue,current_running);
	queue_push(queue,current_running);
	block_accur = TRUE;
}

void do_unblock_one(queue_t *queue)
{
    // unblock the head task from the queue
	void* temp;
	if (!queue_is_empty(queue)) {
		temp = queue_dequeue(queue);
		queue_push(&ready_queue,temp);
	}
}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
	void* temp;
	while (!queue_is_empty(queue)) {
		temp = queue_dequeue(queue);
		queue_push(&ready_queue,temp);
	}
}
