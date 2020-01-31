#include "sem.h"
#include "sched.h"
#include "stdio.h"

void do_semaphore_init(semaphore_t *s, int val)
{
    s->limit = val;
    queue_init(&s->queue);
}

void do_semaphore_up(semaphore_t *s)
{
    do_unblock_all(&s->queue);
    int i;
    for (i = 0; i < MAX_LOCKS; i++) {
        if (current_running->sems[i] == s) {
            current_running->sems[i] = NULL;
            break;
        }
    }
	s->limit++;
}

void do_semaphore_down(semaphore_t *s)
{
    while (s->limit <= 0) {
		do_block(&s->queue);
		do_scheduler();
	}
    int i, avail = 0;
    for (i = 0; i < MAX_LOCKS; i++) {
        if (current_running->sems[i] == NULL) {
            avail = 1;
            break;
        }
    }
    if (!avail) do_exit();
    current_running->sems[i] = s;
	s->limit--;
}