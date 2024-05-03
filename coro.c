/* Implementing coroutines with setjmp/longjmp */

#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include "ttt.c"
//#include "tt.c"

#define CTRL_KEY(k) ((k) & 0x1f)
static char table[N_GRIDS];
static int AI1_w = 0;
static int AI2_w = 0;
static int c = 0;
//static int move_record[N_GRIDS];
//static int move_count = 0;
struct termios orig_termios;


void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(IXON);
  raw.c_lflag &= ~(ECHO | ICANON);
  orig_termios.c_lflag |= (ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


static void wfi()
{
    enableRawMode();
    struct timeval timeout;

    // 設置超時值為 1 秒
    timeout.tv_sec = 1;  // 秒
    timeout.tv_usec = 0; // 微秒

    // 使用 select 函數監視標準輸入
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

    if (result > 0) {
        if (read(STDIN_FILENO, &c, 1) == 1) {
        }
    } else if (result == 0) {
        c = '\0';
    } else {
        perror("select");
    }
}

static void print_times()
{
    // 設置要顯示時間的城市時區
    setenv("TZ", "Asia/Taipei", 1);  // 設置為台北時區（示例）

    // 獲取當前的系統時間
    time_t raw_time;
    time(&raw_time);

    // 將時間轉換為當地時間
    struct tm *local_time = localtime(&raw_time);

    // 格式化當地時間
    char formatted_time[100];
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", local_time);

    // 顯示當地時間
    printf("當前時間（台北時間）：%s\n", formatted_time);
}

struct task {
    jmp_buf env;
    struct list_head list;
    char task_name[10];
    int n;
    int i;
};

struct arg {
    int n;
    int i;
    char *task_name;
};

static LIST_HEAD(tasklist);
static void (**tasks)(void *);
static struct arg *args;
static int ntasks;
static jmp_buf sched, stop;
static struct task *cur_task;

void stopf()
{
    while (1)
    {
        wfi();
        if (c == CTRL_KEY('p')){
            c = '\0';
            longjmp(stop, 1);
        }
            
    }
    
}

static void task_add(struct task *task)
{
    list_add_tail(&task->list, &tasklist);
}

static void task_switch()
{

    if (!list_empty(&tasklist)) {
        struct task *t = list_first_entry(&tasklist, struct task, list);
        list_del(&t->list);
        cur_task = t;
        longjmp(t->env, 1);
    }
}

void schedule(void)
{
    int i = 0;

    setjmp(sched);

    while (ntasks-- > 0) {
        struct arg arg = args[i];
        tasks[i++](&arg);
        printf("Never reached\n");
    }

    task_switch();
}

/* A task yields control n times */

void task0(void *arg)
{
    struct task *task = malloc(sizeof(struct task));
    strcpy(task->task_name, ((struct arg *) arg)->task_name);
    task->n = ((struct arg *) arg)->n;
    task->i = ((struct arg *) arg)->i;
    INIT_LIST_HEAD(&task->list);


    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }

    task = cur_task;

    while (c!=CTRL_KEY('q')) {
        if (setjmp(task->env) == 0) {
            AI1(table);
            task_add(task);
            task_switch();
        }
        task = cur_task;
    }
    longjmp(sched, 1);
}

void task1(void *arg)
{
    struct task *task = malloc(sizeof(struct task));
    strcpy(task->task_name, ((struct arg *) arg)->task_name);
    task->n = ((struct arg *) arg)->n;
    task->i = ((struct arg *) arg)->i;
    INIT_LIST_HEAD(&task->list);



    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }

    task = cur_task;

    while (c!=CTRL_KEY('q')) {
        if (setjmp(task->env) == 0) {
            if(system("clear") == -1){
                //The system method failed
            }
            char win = check_win(table);
            if (win == 'D') {
                draw_board(table);
                printf("It is a draw!\n");
                printf("AI_1_win: %d   AI_2_win: %d\n",AI1_w,AI2_w);
                
                print_times();
                sleep(1);
                memset(table, ' ', N_GRIDS);
                negamax_init();
                
            } else if (win != ' ') {
                draw_board(table);
                printf("%c won!\n", win);
                if (win == 'X'){
                    AI1_w++;
                }
                else {
                    AI2_w++;
                }
                printf("AI_1_win: %d   AI_2_win: %d\n",AI1_w,AI2_w);
                
                print_times();
                sleep(1);
                memset(table, ' ', N_GRIDS);
                negamax_init();
                
            }
            else {
                draw_board(table);
                printf("\n");
                printf("AI_1_win: %d   AI_2_win: %d\n",AI1_w,AI2_w);
                
                print_times();
                
            }
            //enableRawMode();
            
            wfi();
            setjmp(stop);
            if (c ==CTRL_KEY('p')){
                stopf();
            }
            // if (read(STDIN_FILENO, &c, 1) == -1 );

            task_add(task);
            task_switch();
        }
        task = cur_task;
    }
    
    longjmp(sched, 1);
}

void task2(void *arg)
{
    struct task *task = malloc(sizeof(struct task));
    strcpy(task->task_name, ((struct arg *) arg)->task_name);
    task->n = ((struct arg *) arg)->n;
    task->i = ((struct arg *) arg)->i;
    INIT_LIST_HEAD(&task->list);


    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }

    task = cur_task;

    while (c!=CTRL_KEY('q')) {
        if (setjmp(task->env) == 0) {
            AI2(table);
            
            task_add(task);
            task_switch();

        }
        task = cur_task;
    }
    print_moves();
    longjmp(sched, 1);
}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
int coro(void)
{
    void (*registered_task[])(void *) = {task0, task1, task2, task1};
    struct arg arg0 = {.n = 70, .i = 0, .task_name = "Task 0"};
    struct arg arg1 = {.n = 70, .i = 1, .task_name = "Task 1"};
    struct arg arg2 = {.n = 70, .i = 0, .task_name = "Task 2"};
    struct arg arg3 = {.n = 70, .i = 0, .task_name = "Task 3"};
    struct arg registered_arg[] = {arg0, arg1, arg2, arg3};
    tasks = registered_task;
    args = registered_arg;
    ntasks = ARRAY_SIZE(registered_task);
    srand(time(NULL));
    memset(table, ' ', N_GRIDS);
    negamax_init();
    schedule();

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    move_count = 0;
    c = '\0';
    return 0;
}