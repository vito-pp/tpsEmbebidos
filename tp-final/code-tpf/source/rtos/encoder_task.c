#include <stdbool.h>
#include <stdint.h>

#include <os.h>

#include "drv/rotary_encoder.h"
#include "rtos/app_event.h"
#include "rtos/encoder_task.h"

#define ENCODER_TASK_PRIO       4u
#define ENCODER_TASK_STK_SIZE   512u
#define ENCODER_QUEUE_SIZE      12u

#define MIN_PRESS_TICKS \
    ((MIN_PRESS_CYCLES + ENCODER_BUFFER_SIZE - 1u) / ENCODER_BUFFER_SIZE)
#define LONG_PRESS_TICKS \
    (LONG_PRESS_CYCLES / ENCODER_BUFFER_SIZE)
#define DOUBLE_PRESS_TICKS \
    (MAX_CYCLES_BETWEEN_DOUBLE_PRESS / ENCODER_BUFFER_SIZE)

typedef struct
{
    EncoderRawEvent_t event;
} EncoderMessage_t;

static OS_TCB  EncoderTaskTCB;
static CPU_STK EncoderTaskStk[ENCODER_TASK_STK_SIZE];
static OS_Q    EncoderQueue;

static EncoderMessage_t encoder_messages[] =
{
    {ENCODER_RAW_CW},
    {ENCODER_RAW_CCW},
    {ENCODER_RAW_BUTTON_DOWN},
    {ENCODER_RAW_BUTTON_UP},
    {ENCODER_RAW_RESET}
};

static void EncoderTask(void *p_arg);
static bool EncoderTask_DeadlineReached(OS_TICK now, OS_TICK deadline);

void EncoderTask_Create(void)
{
    OS_ERR err;

    OSQCreate(&EncoderQueue,
              "Encoder Queue",
              ENCODER_QUEUE_SIZE,
              &err);

    if (err != OS_ERR_NONE) {
        while (1) {
        }
    }

    OSTaskCreate(&EncoderTaskTCB,
                 "Encoder Task",
                 EncoderTask,
                 0,
                 ENCODER_TASK_PRIO,
                 &EncoderTaskStk[0],
                 ENCODER_TASK_STK_SIZE / 10u,
                 ENCODER_TASK_STK_SIZE,
                 0,
                 0,
                 0,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);

    if (err != OS_ERR_NONE) {
        while (1) {
        }
    }

    encoderInit();
}

void EncoderTask_PostFromISR(EncoderRawEvent_t event)
{
    OS_ERR err;

    if (event > ENCODER_RAW_RESET) {
        return;
    }

    OSQPost(&EncoderQueue,
            &encoder_messages[event],
            sizeof(encoder_messages[event]),
            OS_OPT_POST_FIFO,
            &err);
}

void EncoderTask_Reset(void)
{
    OS_ERR err;

    OSQFlush(&EncoderQueue, &err);
    OSQPost(&EncoderQueue,
            &encoder_messages[ENCODER_RAW_RESET],
            sizeof(encoder_messages[ENCODER_RAW_RESET]),
            OS_OPT_POST_FIFO,
            &err);
}

static void EncoderTask(void *p_arg)
{
    OS_ERR err;
    OS_MSG_SIZE message_size;
    OS_TICK now;
    OS_TICK timeout;
    OS_TICK press_started = 0u;
    OS_TICK long_deadline = 0u;
    OS_TICK single_deadline = 0u;
    EncoderMessage_t *message;
    bool button_down = false;
    bool long_reported = false;
    bool single_pending = false;

    (void)p_arg;

    while (1) {
        now = OSTimeGet(&err);

        if (button_down && !long_reported &&
            EncoderTask_DeadlineReached(now, long_deadline)) {
            AppEvent_Post(EV_RESET);
            long_reported = true;
            single_pending = false;
            continue;
        }

        if (single_pending &&
            EncoderTask_DeadlineReached(now, single_deadline)) {
            AppEvent_Post(EV_ENTER);
            single_pending = false;
            continue;
        }

        timeout = 0u;
        if (button_down && !long_reported) {
            timeout = long_deadline - now;
        }
        if (single_pending &&
            ((timeout == 0u) || ((single_deadline - now) < timeout))) {
            timeout = single_deadline - now;
        }

        message = OSQPend(&EncoderQueue,
                          timeout,
                          OS_OPT_PEND_BLOCKING,
                          &message_size,
                          0,
                          &err);

        if (err == OS_ERR_TIMEOUT) {
            continue;
        }

        if ((err != OS_ERR_NONE) ||
            (message == 0) ||
            (message_size != sizeof(*message))) {
            continue;
        }

        now = OSTimeGet(&err);

        switch (message->event) {
        case ENCODER_RAW_CW:
            if (!single_pending) {
                AppEvent_Post(EV_FORWARD);
            }
            break;

        case ENCODER_RAW_CCW:
            if (!single_pending) {
                AppEvent_Post(EV_BACKWARD);
            }
            break;

        case ENCODER_RAW_BUTTON_DOWN:
            if (!button_down) {
                button_down = true;
                long_reported = false;
                press_started = now;
                long_deadline = now + LONG_PRESS_TICKS;
            }
            break;

        case ENCODER_RAW_BUTTON_UP:
            if (!button_down) {
                break;
            }

            button_down = false;
            if (long_reported) {
                break;
            }

            if ((now - press_started) < MIN_PRESS_TICKS) {
                break;
            }

            if (single_pending &&
                !EncoderTask_DeadlineReached(now, single_deadline)) {
                AppEvent_Post(EV_DOUBLE_ENTER);
                single_pending = false;
            } else {
                single_pending = true;
                single_deadline = now + DOUBLE_PRESS_TICKS;
            }
            break;

        case ENCODER_RAW_RESET:
            button_down = false;
            long_reported = false;
            single_pending = false;
            break;

        default:
            break;
        }
    }
}

static bool EncoderTask_DeadlineReached(OS_TICK now, OS_TICK deadline)
{
    return ((int32_t)(now - deadline) >= 0);
}
