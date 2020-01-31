#include "test2.h"
#include "lock.h"
#include "stdio.h"
#include "syscall.h"

int lock1_is_init = FALSE;
int lock2_is_init = FALSE;
static char blank[] = {"                                                       "};

/* if you want to use spin lock, you need define SPIN_LOCK */
//  #define SPIN_LOCK
spin_lock_t spin_lock;

/* if you want to use mutex lock, you need define MUTEX_LOCK */
#define MUTEX_LOCK
mutex_lock_t mutex_lock_1;
mutex_lock_t mutex_lock_2;

void lock_task1(void)
{
        int print_location = 3;
        while (1)
        {
                int i;
                if (!lock1_is_init)
                {

#ifdef SPIN_LOCK
                        spin_lock_init(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                        mutex_lock_init(&mutex_lock_1);
#endif
                        lock1_is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock_1.\n");

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_1);
#endif

                for (i = 0; i < 25; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock_1 and running.(%d)\n", i);
                }

				sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock_1 and exited.\n");

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_release(&mutex_lock_1);
#endif
        }
}

void lock_task2(void)
{
        int print_location = 4;
        while (1)
        {
                int i;
                if (!lock1_is_init)
                {

#ifdef SPIN_LOCK
                        spin_lock_init(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                        mutex_lock_init(&mutex_lock_1);
#endif
                        lock1_is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock_1.\n");

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_1);
#endif

                for (i = 0; i < 25; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock_1 and running.(%d)\n", i);
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock_1 and exited.\n");

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_release(&mutex_lock_1);
#endif
        }
}

void lock_task3(void)
{
        int print_location = 5;
        while (1)
        {
                int i;
                if (!lock2_is_init)
                {

#ifdef SPIN_LOCK
                        spin_lock_init(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                        mutex_lock_init(&mutex_lock_2);
#endif
                        lock2_is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock_2.\n");

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_2);
#endif

                for (i = 0; i < 20; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock_2 and running.(%d)\n", i);
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock_2 and exited.\n");

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_release(&mutex_lock_2);
#endif
        }
}

void lock_task4(void)
{
        int print_location = 6;
        while (1)
        {
                int i;
                if (!lock2_is_init)
                {

#ifdef SPIN_LOCK
                        spin_lock_init(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                        mutex_lock_init(&mutex_lock_2);
#endif
                        lock2_is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock_2.\n");

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_2);
#endif

                for (i = 0; i < 20; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock_2 and running.(%d)\n", i);
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock_2 and exited.\n");

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                mutex_lock_release(&mutex_lock_2);
#endif
        }
}

void lock_task5(void)
{
        int print_location = 7;
        while (1)
        {
                int i;
				if (!lock1_is_init)
                {
#ifdef MUTEX_LOCK
                        mutex_lock_init(&mutex_lock_1);
#endif
                        lock1_is_init = TRUE;
                }
                if (!lock2_is_init)
                {
#ifdef MUTEX_LOCK
                        mutex_lock_init(&mutex_lock_2);
#endif
                        lock2_is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock_1 & lock_2.\n");

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_1);
#endif

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock_1. Applying for lock_2.\n");

#ifdef MUTEX_LOCK
                mutex_lock_acquire(&mutex_lock_2);
#endif

                for (i = 0; i < 15; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock_1 & lock_2 and running.(%d)\n", i);
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock_1 & lock_2 and exited.\n");

#ifdef MUTEX_LOCK
				mutex_lock_release(&mutex_lock_1);
                mutex_lock_release(&mutex_lock_2);
#endif
        }
}