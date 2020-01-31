#include "lock.h"
#include "sched.h"
#include "syscall.h"

void spin_lock_init(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void spin_lock_acquire(spin_lock_t *lock)
{
    while (LOCKED == lock->status)
    {
    };
    lock->status = LOCKED;
}

void spin_lock_release(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
	lock->status = UNLOCKED;
    queue_init(&lock->queue);
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
	while (LOCKED == lock->status) {
		do_block(&lock->queue);
		do_scheduler();
	}
    int i, avail = 0;
    for (i = 0; i < MAX_LOCKS; i++) {
        if (current_running->locks[i] == NULL) {
            avail = 1;
            break;
        }
    }
    if (!avail) do_exit();
    current_running->locks[i] = lock;
	lock->status = LOCKED;
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
	do_unblock_all(&lock->queue);
    int i;
    for (i = 0; i < MAX_LOCKS; i++) {
        if (current_running->locks[i] == lock) {
            current_running->locks[i] = NULL;
            break;
        }
    }
	lock->status = UNLOCKED;
}
