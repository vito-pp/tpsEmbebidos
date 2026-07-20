#include <stdint.h>

#include <os.h>

#include "drv/board.h"
#include "drv/gpio.h"
#include "rtos/matrix_task.h"
#include "ui/auth_ui.h"
#include "ui/map.h"

#define MATRIX_TASK_PRIO        6u
#define MATRIX_TASK_STK_SIZE    512u
#define MATRIX_QUEUE_SIZE       4u
#define MATRIX_FLOOR_COUNT      3u
#define MATRIX_INDICATOR_COUNT  4u

typedef enum
{
    MATRIX_MSG_REFRESH
} MatrixMessageType_t;

typedef struct
{
    MatrixMessageType_t type;
} MatrixMessage_t;

extern uint8_t sendingDMA;

static OS_TCB  MatrixTaskTCB;
static CPU_STK MatrixTaskStk[MATRIX_TASK_STK_SIZE];
static OS_Q    MatrixQueue;

static MatrixMessage_t refresh_message = {MATRIX_MSG_REFRESH};

static void MatrixTask(void *p_arg);
static void MatrixTask_UpdateDisplay(void);

void MatrixTask_Create(void)
{
    OS_ERR err;

    OSQCreate(&MatrixQueue,
              "Matrix Queue",
              MATRIX_QUEUE_SIZE,
              &err);

    if (err != OS_ERR_NONE) {
        while (1) {
        }
    }

    OSTaskCreate(&MatrixTaskTCB,
                 "Matrix Task",
                 MatrixTask,
                 0,
                 MATRIX_TASK_PRIO,
                 &MatrixTaskStk[0],
                 MATRIX_TASK_STK_SIZE / 10u,
                 MATRIX_TASK_STK_SIZE,
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

void MatrixTask_RequestRefresh(void)
{
    OS_ERR err;

    OSQPost(&MatrixQueue,
            &refresh_message,
            sizeof(refresh_message),
            OS_OPT_POST_FIFO,
            &err);
}

static void MatrixTask(void *p_arg)
{
    OS_ERR err;
    OS_MSG_SIZE message_size;
    MatrixMessage_t *message;

    (void)p_arg;

    MatrixTask_UpdateDisplay();

    while (1) {
        message = OSQPend(&MatrixQueue,
                          0,
                          OS_OPT_PEND_BLOCKING,
                          &message_size,
                          0,
                          &err);

        if ((err == OS_ERR_NONE) &&
            (message != 0) &&
            (message_size == sizeof(*message)) &&
            (message->type == MATRIX_MSG_REFRESH)) {
            MatrixTask_UpdateDisplay();
        }
    }
}

static void MatrixTask_UpdateDisplay(void)
{
    OS_ERR err;
    uint8_t floor;
    uint8_t indicator;

    for (floor = 1u; floor <= MATRIX_FLOOR_COUNT; floor++) {
        setOcupation(floor, getFloorOccupancy(floor));
    }

    for (indicator = 1u;
         indicator <= MATRIX_INDICATOR_COUNT;
         indicator++) {
        clearErrorX(indicator);
    }

    while (sendingDMA != 0u) {
        OSTimeDly(1u, OS_OPT_TIME_DLY, &err);
    }

    gpioWrite(PORTNUM2PIN(PB, 3), 1);
    loadMap();
}
