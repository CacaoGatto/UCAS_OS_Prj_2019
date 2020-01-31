#include "irq.h"
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
int shell_init = 0;
queue_t ready_queue;
//queue_t block_queue;
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
            ((pcb_t*)check)->status = TASK_READY;
		}
		check = next_check;
	}
}

static void get_bonus()
{
    void* check_queue = sleep_queue.next;
    while (check_queue != NULL) {
        void* check = ((queue_t*)check_queue)->head;
        while (check != NULL) {
            ((pcb_t*)check)->bonus += 1;
            check = ((pcb_t*)check)->next;
        }
        check_queue = ((queue_t*)check_queue)->next;
    }
}

void scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.
	current_running->cursor_x = screen_cursor_x;
	current_running->cursor_y = screen_cursor_y;
	
	check_sleeping();
	if (shell_init && block_accur == FALSE) current_running->status = TASK_READY;
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
    current_running->status = TASK_RUNNING;
    shell_init = 1;
	
	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;
}

void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
	current_running->target_counter = sleep_time * 100 * 150000 / TIMER_INTERVAL;
	current_running->current_counter = 0;
    current_running->status = TASK_BLOCKED;
	do_block(&sleep_queue);
	do_scheduler();
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue
    current_running->status = TASK_BLOCKED;
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
        ((pcb_t*)temp)->status = TASK_READY;
		queue_push(&ready_queue,temp);
	}
}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
	void* temp;
	while (!queue_is_empty(queue)) {
		temp = queue_dequeue(queue);
        ((pcb_t*)temp)->status = TASK_READY;
		queue_push(&ready_queue,temp);
	}
}

void do_ps()
{
    // print all processes
    vt100_move_cursor(1, ++pcb[0].line);
    printk("[PROCESS TABLE]");
    int num, items = 0;
    for (num = 0; num < NUM_MAX_TASK; num++) {
        switch(pcb[num].status) {
            case TASK_BLOCKED:
                vt100_move_cursor(1, ++pcb[0].line);
                printk("[%d] PID : %d  Status : BLOCKED", items++, pcb[num].pid);
                break;
            case TASK_RUNNING:
                vt100_move_cursor(1, ++pcb[0].line);
                printk("[%d] PID : %d  Status : RUNNING", items++, pcb[num].pid);
                break;
            case TASK_READY:
                vt100_move_cursor(1, ++pcb[0].line);
                printk("[%d] PID : %d  Status : READY", items++, pcb[num].pid);
                break;
            default:
                break;
        }
    }
}

int init_one_pcb()
{
    int try, get = 0;
    for (try = 1; try < NUM_MAX_TASK; try++) {
        if (pcb[try].status == TASK_EXITED) {
            get = 1;
            int i;
            for (i = 0; i < 31; i++) {
                pcb[try].kernel_context.regs[i] = 0;
                pcb[try].user_context.regs[i] = 0;
            }
            
            pcb[try].kernel_stack_top = KERNEL_STACK_TOP - try * 0x4000;
            pcb[try].kernel_context.regs[29] = pcb[try].kernel_stack_top;
            pcb[try].kernel_context.regs[31] = (uint32_t)handle_int + 0x10;
            pcb[try].kernel_context.cp0_status = 0x1000fc02;
            pcb[try].kernel_context.hi = 0;
            pcb[try].kernel_context.lo = 0;
            pcb[try].kernel_context.cp0_badvaddr = 0;
            pcb[try].kernel_context.cp0_cause = 0;
            pcb[try].kernel_context.cp0_epc = 0;
            
            pcb[try].user_stack_top = USER_STACK_TOP - try * 0x4000;
            pcb[try].user_context.regs[29] = pcb[try].user_stack_top;
            pcb[try].user_context.regs[31] = 0;
            pcb[try].user_context.cp0_status = 0x1000fc03;
            pcb[try].user_context.hi = 0;
            pcb[try].user_context.lo = 0;
            pcb[try].user_context.cp0_badvaddr = 0;
            pcb[try].user_context.cp0_cause = 0;
            
            pcb[try].bonus = 0;
            pcb[try].status = TASK_READY;
            pcb[try].pcond = NULL;
            pcb[try].pbarrier = NULL;
            for (i = 0; i < MAX_LOCKS; i++) {
                pcb[0].locks[i] = NULL;
                pcb[0].sems[i] = NULL;
            }
            
            break;
        }
    }
    return (get * try);
}

void do_spawn(task_info_t *task)
{
    int num = init_one_pcb();
    //vt100_move_cursor(1, ++pcb[0].line);
    if (!num) return;
    //printk("Exec process[%d] %s.", ++process_id, task->name);
    pcb[num].user_context.cp0_epc = task->entry_point;
	pcb[num].type = task->type;
	pcb[num].priority = task->priority;
    pcb[num].pid = ++process_id;
	queue_push(&ready_queue,&pcb[num]);
}

void do_kill(pid_t pid)
{
    void* check_queue = all_queue.head;
    void* check;
    int found = 0;
    while (check_queue != NULL) {
        check = ((queue_t*)check_queue)->head;
        while (check != NULL) {
            if (((pcb_t*)check)->pid == pid) {
                found = 1;
                break;
            }
            check = ((pcb_t*)check)->next;
        }
        if (found) break;
        check_queue = ((queue_t*)check_queue)->next;
    }
    if (!found) return;
    if ((pcb_t*)check == current_running) do_exit();
    ((pcb_t*)check)->status = TASK_EXITED;
    do_unblock_all(&((pcb_t*)check)->wait);
    int i;
    for (i = 0; i < MAX_LOCKS; i++) {
        if (((pcb_t*)check)->locks[i] != NULL) {
            do_unblock_all(&((pcb_t*)check)->locks[i]->queue);
            ((pcb_t*)check)->locks[i]->status = UNLOCKED;
        }
        if (((pcb_t*)check)->sems[i] != NULL) {
            do_unblock_all(&((pcb_t*)check)->sems[i]->queue);
            ((pcb_t*)check)->sems[i]->limit++;
        }
    }
    if (((pcb_t*)check)->pcond != NULL) {
        do_unblock_all(&((pcb_t*)check)->pcond->queue);
    }
    if (((pcb_t*)check)->pbarrier != NULL) {
        ((pcb_t*)check)->pbarrier->limit++;
    }
    queue_remove(check_queue,((pcb_t*)check));
	block_accur = TRUE;
}

void do_exit()
{
    current_running->status = TASK_EXITED;
    do_unblock_all(&current_running->wait);
    int i;
    for (i = 0; i < MAX_LOCKS; i++) {
        if (current_running->locks[i] != NULL) {
            do_unblock_all(&current_running->locks[i]->queue);
            current_running->locks[i]->status = UNLOCKED;
        }
        if (current_running->sems[i] != NULL) {
            do_unblock_all(&current_running->sems[i]->queue);
            current_running->sems[i]->limit++;
        }
    }
    if (current_running->pcond != NULL) {
        do_unblock_all(&current_running->pcond->queue);
    }
    if (current_running->pbarrier != NULL) {
        current_running->pbarrier->limit++;
    }
    next_running = (pcb_t*)queue_remove(&ready_queue,current_running);
	block_accur = TRUE;
    do_scheduler();
}

void do_wait(pid_t pid)
{
    void* check_queue = all_queue.head;
    void* check;
    int found = 0;
    while (check_queue != NULL) {
        check = ((queue_t*)check_queue)->head;
        while (check != NULL) {
            if (((pcb_t*)check)->pid == pid) {
                do_block(&((pcb_t*)check)->wait);
                do_scheduler();
                found = 1;
                break;
            }
            check = ((pcb_t*)check)->next;
        }
        if (found) break;
        check_queue = ((queue_t*)check_queue)->next;
    }
}

pid_t do_getpid()
{
    return current_running->pid;
}
