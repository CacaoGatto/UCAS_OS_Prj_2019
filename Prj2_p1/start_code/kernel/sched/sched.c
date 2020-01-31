#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;
pcb_t *next_running;

/* global process id */
pid_t process_id = 0;

int block_accur = FALSE;
queue_t ready_queue;
queue_t block_queue;

static void check_sleeping()
{
}

void scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.
	if (block_accur == TRUE) {
		current_running = next_running;
		block_accur = FALSE;
	}
	else current_running = (pcb_t*)current_running->next;
	if (current_running == NULL) current_running = &pcb[1];
	process_id = current_running->pid; 
}

void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
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
	void* temp = queue_dequeue(queue);
	queue_push(&ready_queue,temp);
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
