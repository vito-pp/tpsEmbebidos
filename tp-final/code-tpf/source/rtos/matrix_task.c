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
#define MATRIX_INDICATOR_DURATION_TICKS  (2u * OS_CFG_TICK_RATE_HZ)

typedef enum
{
    MATRIX_MSG_REFRESH,
    MATRIX_MSG_INDICATOR
} MatrixMessageType_t;

typedef struct
{
    MatrixMessageType_t type;
    MatrixIndicator_t indicator;
} MatrixMessage_t;

static OS_TCB  MatrixTaskTCB;
static CPU_STK MatrixTaskStk[MATRIX_TASK_STK_SIZE];
static OS_Q    MatrixQueue;
static OS_SEM  MatrixDmaReady;

static MatrixMessage_t refresh_message =
    {MATRIX_MSG_REFRESH, MATRIX_INDICATOR_NONE};
static MatrixMessage_t valid_indicator_message =
    {MATRIX_MSG_INDICATOR, MATRIX_INDICATOR_VALID};
static MatrixMessage_t invalid_indicator_message =
    {MATRIX_MSG_INDICATOR, MATRIX_INDICATOR_INVALID};
static MatrixMessage_t timeout_indicator_message =
    {MATRIX_MSG_INDICATOR, MATRIX_INDICATOR_TIMEOUT};

static void MatrixTask(void *p_arg);
static void MatrixTask_UpdateDisplay(MatrixIndicator_t indicator);

void MatrixTask_Create(void)
{
    OS_ERR err;

    OSSemCreate(&MatrixDmaReady,
                "Matrix DMA Ready",
                1u,
                &err);

    if (err != OS_ERR_NONE) {
        while (1) {
        }
    }

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

void MatrixTask_FrameDoneFromISR(void)
{
    OS_ERR err;

    OSSemPost(&MatrixDmaReady, OS_OPT_POST_1, &err);
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

void MatrixTask_RequestIndicator(MatrixIndicator_t indicator)
{
    OS_ERR err;
    MatrixMessage_t *message;

    if (indicator == MATRIX_INDICATOR_VALID) {
        message = &valid_indicator_message;
    } else if (indicator == MATRIX_INDICATOR_INVALID) {
        message = &invalid_indicator_message;
    } else if (indicator == MATRIX_INDICATOR_TIMEOUT) {
        message = &timeout_indicator_message;
    } else {
        return;
    }

    OSQPost(&MatrixQueue,
            message,
            sizeof(*message),
            OS_OPT_POST_FIFO,
            &err);
}

static void MatrixTask(void *p_arg)
{
    OS_ERR err;
    OS_MSG_SIZE message_size;
    OS_TICK wait_timeout = 0u;
    MatrixMessage_t *message;
    MatrixIndicator_t indicator = MATRIX_INDICATOR_NONE;

    (void)p_arg;

    MatrixTask_UpdateDisplay(indicator);

    while (1) {
        message = OSQPend(&MatrixQueue,
                          wait_timeout,
                          OS_OPT_PEND_BLOCKING,
                          &message_size,
                          0,
                          &err);

        if ((err == OS_ERR_TIMEOUT) &&
            (indicator != MATRIX_INDICATOR_NONE)) {
            indicator = MATRIX_INDICATOR_NONE;
            wait_timeout = 0u;
            MatrixTask_UpdateDisplay(indicator);
        } else if ((err == OS_ERR_NONE) &&
            (message != 0) &&
            (message_size == sizeof(*message))) {
            if (message->type == MATRIX_MSG_INDICATOR) {
                indicator = message->indicator;
                wait_timeout = MATRIX_INDICATOR_DURATION_TICKS;
            }
            MatrixTask_UpdateDisplay(indicator);
        }
    }
}

static void MatrixTask_UpdateDisplay(MatrixIndicator_t indicator)
{
    OS_ERR err;
    uint8_t floor;
    uint8_t indicator_index;

    OSSemPend(&MatrixDmaReady,
              0u,
              OS_OPT_PEND_BLOCKING,
              0,
              &err);

    if (err != OS_ERR_NONE) {
        return;
    }

    for (floor = 1u; floor <= MATRIX_FLOOR_COUNT; floor++) {
        setOcupation(floor, getFloorOccupancy(floor));
    }

    for (indicator_index = 1u;
         indicator_index <= MATRIX_INDICATOR_COUNT;
         indicator_index++) {
        clearErrorX(indicator_index);
    }

    if (indicator == MATRIX_INDICATOR_VALID) {
        setErrorX(1u);
    } else if (indicator == MATRIX_INDICATOR_INVALID) {
        setErrorX(2u);
    } else if (indicator == MATRIX_INDICATOR_TIMEOUT) {
        setErrorX(3u);
    }

    gpioWrite(PORTNUM2PIN(PB, 3), 1);
    loadMap();
}
