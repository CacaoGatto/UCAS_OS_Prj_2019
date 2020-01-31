/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "irq.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"

uint32_t exception_handler[32];

static void init_pcb()
{
	//for TASK1
	queue_init(&ready_queue);
	queue_init(&block_queue);
	queue_init(&sleep_queue);
	pcb[0].kernel_stack_top = 0xa0f00000;
	pcb[0].user_stack_top = 0xa0e80000;
	pcb[0].next = &pcb[1];
	pcb[0].prev = NULL;
	pcb[0].pid  = 0;
	current_running = &pcb[0];
	int i;
	for (i = 1; i <= 15; i++) {
		memset(pcb[i],0,sizeof(pcb[i]));
		
		pcb[i].kernel_stack_top = pcb[0].kernel_stack_top - i * 0x1000;
		pcb[i].kernel_context.regs[29] = pcb[i].kernel_stack_top;
		pcb[i].kernel_context.regs[31] = (uint32_t)handle_int + 0x10;
		pcb[i].kernel_context.cp0_status = 0x10008002;
		pcb[i].kernel_context.hi = 0;
		pcb[i].kernel_context.lo = 0;
		pcb[i].kernel_context.cp0_badvaddr = 0;
		pcb[i].kernel_context.cp0_cause = 0;
		pcb[i].kernel_context.cp0_epc = 0;
		
		pcb[i].user_stack_top = pcb[0].user_stack_top - i * 0x1000;
		pcb[i].user_context.regs[29] = pcb[i].user_stack_top;
		pcb[i].user_context.regs[31] = 0;
		pcb[i].user_context.cp0_status = 0x10008003;
		pcb[i].user_context.hi = 0;
		pcb[i].user_context.lo = 0;
		pcb[i].user_context.cp0_badvaddr = 0;
		pcb[i].user_context.cp0_cause = 0;
		
		pcb[i].bonus = 0;
		pcb[i].pid = i;
	}
	
	for (i = 1; i <= 3; i++) {
		pcb[i].user_context.cp0_epc = sched2_tasks[i-1]->entry_point;
		pcb[i].type = sched2_tasks[i-1]->type;
		pcb[i].priority = i * 2;
		queue_push(&ready_queue,&pcb[i]);
//		printk("\ntask%d:0x%x\npcb%d:0x%x\n\n",i,sched2_tasks[i-1]->entry_point,i,&pcb[i]);
	}
	for (i = 4; i <= 5; i++) {
		pcb[i].user_context.cp0_epc = timer_tasks[i-4]->entry_point;
		pcb[i].type = timer_tasks[i-4]->type;
		pcb[i].priority = 3;
		queue_push(&ready_queue,&pcb[i]);
//		printk("\ntask%d:0x%x\npcb%d:0x%x\n\n",i,timer_tasks[i-4]->entry_point,i,&pcb[i]);
	}
	for (i = 6; i <= 10; i++) {
		pcb[i].user_context.cp0_epc = lock_tasks[i-6]->entry_point;
		pcb[i].type = lock_tasks[i-6]->type;
		pcb[i].priority = i / 2;
		pcb[i].priority = 11 - pcb[i].priority * 2;
		queue_push(&ready_queue,&pcb[i]);
//		printk("\ntask%d:0x%x\npcb%d:0x%x\n\n",i,timer_tasks[i-6]->entry_point,i,&pcb[i]);
	}
}

static void init_exception_handler()
{
	int i;
	for (i = 0; i < 32; i++) exception_handler[i] = (uint32_t)handle_other;
	exception_handler[0] = (uint32_t)handle_int;
	exception_handler[8] = (uint32_t)handle_syscall;
}

static void init_exception()
{
	// 1. Get CP0_STATUS
	// 2. Disable all interrupt
	// 3. Copy the level 2 exception handling code to 0x80000180
	// 4. reset CP0_COMPARE & CP0_COUNT register
	
	init_exception_handler();
	
	int eh_size = exception_handler_end - exception_handler_begin + 1;
	void* eh_entry = (void*)0x80000180;
	memcpy(eh_entry, exception_handler_begin,  eh_size);
}

static void init_syscall(void)
{
	// init system call table.
	syscall[SYSCALL_SLEEP] = (int(*)())&do_sleep;
	
	syscall[SYSCALL_WRITE] = (int(*)())&screen_write;
	syscall[SYSCALL_CURSOR] = (int(*)())&screen_move_cursor;
	syscall[SYSCALL_REFLUSH] = (int(*)())&screen_reflush;
	
	syscall[SYSCALL_MUTEX_LOCK_INIT] = (int(*)())&do_mutex_lock_init;
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = (int(*)())&do_mutex_lock_acquire;
	syscall[SYSCALL_MUTEX_LOCK_RELEASE] = (int(*)())&do_mutex_lock_release;
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
void __attribute__((section(".entry_function"))) _start(void)
{
	// Close the cache, no longer refresh the cache 
	// when making the exception vector entry copy
	asm_start();

	// init interrupt (^_^)
	init_exception();
	printk("> [INIT] Interrupt processing initialization succeeded.\n");
	
	// init system call table (0_0)
	init_syscall();
	printk("> [INIT] System call initialized successfully.\n");

	// init Process Control Block (-_-!)
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	// init screen (QAQ)
	init_screen();
	printk("> [INIT] SCREEN initialization succeeded.\n");

	// TODO Enable interrupt
	init_clk();
	init_int();
	
	while (1)
	{
		// (QAQQQQQQQQQQQ)
		// If you do non-preemptive scheduling, you need to use it to surrender control
		// do_scheduler();
	};
	return;
}
