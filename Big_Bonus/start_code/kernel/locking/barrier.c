#include "barrier.h"

void do_barrier_init(barrier_t *barrier, int goal)
{
    barrier->limit = goal;
    queue_init(&barrier->queue);
}

void do_barrier_wait(barrier_t *barrier)
{
    barrier->limit--;
    if (barrier->limit <= 0) do_unblock_all(&barrier->queue);
    else {
        do_block(&barrier->queue);
        do_scheduler();
    }
    barrier->limit++;
}