#include <os.h>

#include "rtos/display_task.h"
#include "ui/display.h"

#define DISPLAY_TASK_PRIO       7u
#define DISPLAY_TASK_STK_SIZE   256u

static OS_TCB  DisplayTaskTCB;
static CPU_STK DisplayTaskStk[DISPLAY_TASK_STK_SIZE];

static void DisplayTask(void *p_arg);

void DisplayTask_Create(void)
{
    OS_ERR err;

    OSTaskCreate(&DisplayTaskTCB,
                 "Display Task",
                 DisplayTask,
                 0,
                 DISPLAY_TASK_PRIO,
                 &DisplayTaskStk[0],
                 DISPLAY_TASK_STK_SIZE / 10u,
                 DISPLAY_TASK_STK_SIZE,
                 0,
                 0,
                 0,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);

    if (err != OS_ERR_NONE) {
        while (1) {
        }
    }
}

static void DisplayTask(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;

    while (1) {
        displayRefresh();
        OSTimeDly(1u, OS_OPT_TIME_DLY, &err);
    }
}
