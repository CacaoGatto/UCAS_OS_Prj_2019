/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode. 
 *       The main function is to make system calls through the user's output.
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

#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "syscall.h"

static void disable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

static void enable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}

static char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

    while ((*stat_port & 0x01))
    {
        ch = *read_port;
    }
    return ch;
}

struct task_info task0 = {"shell", (uint32_t)&test_shell, USER_PROCESS, 7};

struct task_info task1 = {"mac_init_task", (uint32_t)&mac_init_task, USER_PROCESS, 1};
struct task_info task2 = {"mac_send_task", (uint32_t)&mac_send_task, USER_PROCESS, 1};
struct task_info task3 = {"mac_recv_task", (uint32_t)&mac_recv_task, USER_PROCESS, 1};

/*
struct task_info task1 = {"task1", (uint32_t)&ready_to_exit_task, USER_PROCESS, 1};
struct task_info task2 = {"task2", (uint32_t)&wait_lock_task, USER_PROCESS, 1};
struct task_info task3 = {"task3", (uint32_t)&wait_exit_task, USER_PROCESS, 1};

struct task_info task4 = {"task4", (uint32_t)&semaphore_add_task1, USER_PROCESS, 1};
struct task_info task5 = {"task5", (uint32_t)&semaphore_add_task2, USER_PROCESS, 1};
struct task_info task6 = {"task6", (uint32_t)&semaphore_add_task3, USER_PROCESS, 1};

struct task_info task7 = {"task7", (uint32_t)&producer_task, USER_PROCESS, 1};
struct task_info task8 = {"task8", (uint32_t)&consumer_task1, USER_PROCESS, 1};
struct task_info task9 = {"task9", (uint32_t)&consumer_task2, USER_PROCESS, 1};

struct task_info task10 = {"task10", (uint32_t)&barrier_task1, USER_PROCESS, 1};
struct task_info task11 = {"task11", (uint32_t)&barrier_task2, USER_PROCESS, 1};
struct task_info task12 = {"task12", (uint32_t)&barrier_task3, USER_PROCESS, 1};

struct task_info task13 = {"SunQuan",(uint32_t)&SunQuan, USER_PROCESS, 1};
struct task_info task14 = {"LiuBei", (uint32_t)&LiuBei, USER_PROCESS, 1};
struct task_info task15 = {"CaoCao", (uint32_t)&CaoCao, USER_PROCESS, 1};
*/
struct task_info *test_tasks[16] = {&task1, &task2, &task3};
int num_test_tasks = 15;

void test_shell()
{
    pcb[0].line = SHELL_BOARD - 1;
    char inst[20];
    int idx = 0;
    sys_move_cursor(1, pcb[0].line);
    printf("----------------- C O M M A N D -----------------");
    sys_move_cursor(1, ++pcb[0].line);
    printf("> root@UCAS_OS: ");
    
    while (1)
    {
        // read command from UART port
        disable_interrupt();
        char ch = read_uart_ch();
        enable_interrupt();
        if (ch == 0) continue;
        // TODO solve command
        if (ch != '\r') {
            if (ch == 8) {
                idx--;
                if (idx < 0) idx = 0;
                else printf("%c", ch);
            }
            else if (idx < 100) {
                inst[idx++] = ch;
                printf("%c", ch);
            }
        }
        else {
            inst[idx] = '\0';
            if (inst[0]=='p' && inst[1]=='s' && inst[2]=='\0') {
                sys_move_cursor(1, ++pcb[0].line);
                sys_ps();
            }
            else if (inst[0]=='c' && inst[1]=='l' && inst[2]=='e' && inst[3]=='a' && inst[4]=='r' && inst[5]=='\0') {
                sys_clear(SHELL_BOARD, SCREEN_HEIGHT);
                pcb[0].line = SHELL_BOARD;
                sys_move_cursor(1, pcb[0].line);
            }
            else if (inst[0]=='e' && inst[1]=='x' && inst[2]=='e' && inst[3]=='c' && inst[4]==' ') {
                sys_move_cursor(1, ++pcb[0].line);
                int task_num = 0, i = 5;
                while(i < idx) task_num = task_num*10+inst[i++]-'0';
                if (task_num >= 0 && task_num < num_test_tasks) {
                    sys_spawn(test_tasks[task_num]);
                    printf("Exec process[%d] %s.", process_id, test_tasks[task_num]->name);
                    sys_move_cursor(1, ++pcb[0].line);
                }
                else {
                    printf("[ERROR] Invalid Task!");
                    pcb[0].line++;
                }
            }
            else if (inst[0]=='k' && inst[1]=='i' && inst[2]=='l' && inst[3]=='l' && inst[4]==' ') {
                sys_move_cursor(1, ++pcb[0].line);
                int target_pid = 0, i = 5;
                while(i < idx) target_pid = target_pid*10+inst[i++]-'0';
                sys_kill(target_pid);
                printf("process[%d] has been killed.", target_pid);
                sys_move_cursor(1, ++pcb[0].line);
            }
            else sys_move_cursor(1, ++pcb[0].line);;
            
            idx = 0;
            sys_move_cursor(1, pcb[0].line);
            printf("> root@UCAS_OS: ");
        }
    }
}