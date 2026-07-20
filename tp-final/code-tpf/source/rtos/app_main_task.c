#include <os.h>
#include <cpu_core.h>

#include "app.h"
#include "hardware.h"
#include "app_main_task.h"
#include "app_event.h"
#include "display_task.h"
#include "encoder_task.h"
#include "matrix_task.h"

#define APP_MAIN_TASK_PRIO       5u
#define APP_MAIN_TASK_STK_SIZE   1024u

static OS_TCB  AppMainTaskTCB;
static CPU_STK AppMainTaskStk[APP_MAIN_TASK_STK_SIZE];

static void AppMainTask(void *p_arg);

void AppMainTask_Create(void)
{
    OS_ERR err;

    OSTaskCreate(&AppMainTaskTCB,
                 "App Main Task",
                 AppMainTask,
                 0,
                 APP_MAIN_TASK_PRIO,
                 &AppMainTaskStk[0],
                 APP_MAIN_TASK_STK_SIZE / 10,
                 APP_MAIN_TASK_STK_SIZE,
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

static void AppMainTask(void *p_arg)
{
    (void)p_arg;

    CPU_Init();

    AppEvent_Create();
    EncoderTask_Create();

    App_Init();

    DisplayTask_Create();
    MatrixTask_Create();

    hw_EnableInterrupts();

    while (1) {
        App_Run();
    }
}
