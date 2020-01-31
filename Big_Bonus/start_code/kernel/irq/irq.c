#include "irq.h"
#include "mac.h"
#include "time.h"
#include "sched.h"
#include "string.h"
#include "screen.h"

static void irq_timer()
{
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
	screen_reflush();
	time_elapsed += 10000 * 150000 / TIMER_INTERVAL;
	void* check = sleep_queue.head;
	while (check != NULL) {
		((pcb_t*)check)->current_counter += 1;
		check = ((pcb_t*)check)->next;
	}
	current_running->bonus = 0;
    if (mac_cnt >= 0) {
        int own = rdes[mac_cnt].des0 >> 31;
        if (!own) mac_irq_handle();
    }
	do_scheduler();
}

void interrupt_helper(uint32_t status, uint32_t cause)
{
    // TODO interrupt handler.
    // Leve3 exception Handler.
    // read CP0 register to analyze the type of interrupt.
	int im7 = status & 0x00008000;
	int ie = status & 0x1;
	int ip = cause & 0x0000ff00;
	int ec = cause & 0x7c;
	if (ip == 0x00008000) irq_timer();
    //else if (ip == 0x00000800 && reg_read_32(0xbfd0105c) == 0x8) mac_irq_handle();
}

void other_exception_handler()
{
    // TODO other exception handler
}