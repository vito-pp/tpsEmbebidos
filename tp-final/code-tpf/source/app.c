#include <stdbool.h>
#include <stdint.h>

#include <os.h>

#include "drv/board.h"
#include "drv/gpio.h"
#include "drv/SysTick.h"
#include "drv/mag_strip.h"
#include "drv/matStream.h"
#include "rtos/app_event.h"
#include "rtos/encoder_task.h"
#include "rtos/matrix_task.h"
#include "ui/auth_ui.h"
#include "ui/display.h"
#include "ui/fsm.h"

#define SYSTEM_CORE_CLOCK_HZ          100000000u
#define APP_INITIAL_TIMEOUT_TICKS     5000u
#define APP_INACTIVITY_TIMEOUT_TICKS  20000u
#define APP_FEEDBACK_TIMEOUT_TICKS    10000u
#define STATUS_LED_TASK_PRIO          8u
#define STATUS_LED_TASK_STK_SIZE      256u
#define STATUS_LED_QUEUE_SIZE         4u
#define STATUS_LED_BLINK_COUNT        4u
#define STATUS_LED_HALF_PERIOD_TICKS  (OS_CFG_TICK_RATE_HZ / 4u)

typedef enum
{
    STATUS_LED_VALID,
    STATUS_LED_TIMEOUT
} StatusLedEvent_t;

typedef struct
{
    StatusLedEvent_t event;
} StatusLedMessage_t;

static FSM_State_t *current;
static OS_TICK inactivity_timeout;
static OS_TCB StatusLedTaskTCB;
static CPU_STK StatusLedTaskStk[STATUS_LED_TASK_STK_SIZE];
static OS_Q StatusLedQueue;

static StatusLedMessage_t status_led_messages[] =
{
    {STATUS_LED_VALID},
    {STATUS_LED_TIMEOUT}
};

static void App_ProcessEvent(FSM_event_t event);
static void App_WaitForFeedback(void);
static void StatusLed_Init(void);
static void StatusLed_Request(StatusLedEvent_t event);
static void StatusLed_Task(void *p_arg);

void App_Init(void)
{
    bool systick_ok;

    magStrip_Init();
    display_init();
    dispBus_init();

    systick_ok = SysTick_Init(0,
                              SYSTEM_CORE_CLOCK_HZ /
                              OS_CFG_TICK_RATE_HZ);
    if (!systick_ok) {
        while (1) {
        }
    }

    StatusLed_Init();

    current = getInitState();
    current = fsmStep(current, EV_NONE);
    inactivity_timeout = APP_INITIAL_TIMEOUT_TICKS;
}

void App_Run(void)
{
    FSM_event_t event;
    AppEventPendResult_t pend_result;

    pend_result = AppEvent_Pend(&event, inactivity_timeout);

    if (pend_result == APP_EVENT_PEND_TIMEOUT) {
        event = EV_TIMEOUT;
    } else if (pend_result != APP_EVENT_PEND_RECEIVED) {
        return;
    }

    if ((event == EV_MAG_DATA) && (validateData() == 0u)) {
        MatrixTask_RequestIndicator(MATRIX_INDICATOR_INVALID);
        resetMagData();
        return;
    }

    App_ProcessEvent(event);
    inactivity_timeout = APP_INACTIVITY_TIMEOUT_TICKS;
}

static void App_ProcessEvent(FSM_event_t event)
{
    FSM_event_t validation_result;

    if (event == EV_TIMEOUT) {
        StatusLed_Request(STATUS_LED_TIMEOUT);
        MatrixTask_RequestIndicator(MATRIX_INDICATOR_TIMEOUT);
    }

    current = fsmStep(current, event);
    current = fsmStep(current, EV_NONE);

    if (!isDataReady()) {
        return;
    }

    validation_result = isValid() ? EV_VALID : EV_INVALID;
    current = fsmStep(current, validation_result);

    if (validation_result == EV_VALID) {
        StatusLed_Request(STATUS_LED_VALID);
        MatrixTask_RequestIndicator(MATRIX_INDICATOR_VALID);
        current = fsmStep(current, EV_NONE);
    } else {
        MatrixTask_RequestIndicator(MATRIX_INDICATOR_INVALID);
    }

    App_WaitForFeedback();
    reset();
    EncoderTask_Reset();
    AppEvent_Flush();
    current = fsmStep(current, EV_NONE);
}

static void App_WaitForFeedback(void)
{
    OS_ERR err;
    OS_TICK elapsed;
    OS_TICK started;
    OS_TICK remaining = APP_FEEDBACK_TIMEOUT_TICKS;
    FSM_event_t ignored_event;
    AppEventPendResult_t pend_result;

    while (remaining > 0u) {
        started = OSTimeGet(&err);
        pend_result = AppEvent_Pend(&ignored_event, remaining);

        if ((pend_result == APP_EVENT_PEND_TIMEOUT) ||
            (pend_result == APP_EVENT_PEND_ERROR)) {
            break;
        }

        elapsed = OSTimeGet(&err) - started;
        if (elapsed >= remaining) {
            break;
        }
        remaining -= elapsed;
    }
}

static void StatusLed_Init(void)
{
    OS_ERR err;

    gpioMode(PIN_LED_GREEN, OUTPUT);
    gpioMode(PIN_LED_BLUE, OUTPUT);
    gpioWrite(PIN_LED_GREEN, !LED_ACTIVE);
    gpioWrite(PIN_LED_BLUE, !LED_ACTIVE);

    OSQCreate(&StatusLedQueue,
              "Status LED Queue",
              STATUS_LED_QUEUE_SIZE,
              &err);

    if (err != OS_ERR_NONE) {
        while (1) {
        }
    }

    OSTaskCreate(&StatusLedTaskTCB,
                 "Status LED Task",
                 StatusLed_Task,
                 0,
                 STATUS_LED_TASK_PRIO,
                 &StatusLedTaskStk[0],
                 STATUS_LED_TASK_STK_SIZE / 10u,
                 STATUS_LED_TASK_STK_SIZE,
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

static void StatusLed_Request(StatusLedEvent_t event)
{
    OS_ERR err;

    if (event > STATUS_LED_TIMEOUT) {
        return;
    }

    OSQPost(&StatusLedQueue,
            &status_led_messages[event],
            sizeof(status_led_messages[event]),
            OS_OPT_POST_FIFO,
            &err);
}

static void StatusLed_Task(void *p_arg)
{
    OS_ERR err;
    OS_MSG_SIZE message_size;
    StatusLedMessage_t *message;
    pin_t pin;
    uint8_t blink;

    (void)p_arg;

    while (1) {
        message = OSQPend(&StatusLedQueue,
                          0u,
                          OS_OPT_PEND_BLOCKING,
                          &message_size,
                          0,
                          &err);

        if ((err != OS_ERR_NONE) ||
            (message == 0) ||
            (message_size != sizeof(*message))) {
            continue;
        }

        pin = (message->event == STATUS_LED_VALID)
                  ? PIN_LED_GREEN
                  : PIN_LED_BLUE;

        gpioWrite(PIN_LED_GREEN, !LED_ACTIVE);
        gpioWrite(PIN_LED_BLUE, !LED_ACTIVE);

        for (blink = 0u; blink < STATUS_LED_BLINK_COUNT; blink++) {
            gpioWrite(pin, LED_ACTIVE);
            OSTimeDly(STATUS_LED_HALF_PERIOD_TICKS,
                      OS_OPT_TIME_DLY,
                      &err);
            gpioWrite(pin, !LED_ACTIVE);
            OSTimeDly(STATUS_LED_HALF_PERIOD_TICKS,
                      OS_OPT_TIME_DLY,
                      &err);
        }
    }
}
