#include <stdbool.h>
#include <stdint.h>

#include <os.h>

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

static FSM_State_t *current;
static OS_TICK inactivity_timeout;

static void App_ProcessEvent(FSM_event_t event);
static void App_WaitForFeedback(void);

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
        resetMagData();
        return;
    }

    App_ProcessEvent(event);
    inactivity_timeout = APP_INACTIVITY_TIMEOUT_TICKS;
}

static void App_ProcessEvent(FSM_event_t event)
{
    FSM_event_t validation_result;

    current = fsmStep(current, event);
    current = fsmStep(current, EV_NONE);

    if (!isDataReady()) {
        return;
    }

    validation_result = isValid() ? EV_VALID : EV_INVALID;
    current = fsmStep(current, validation_result);

    if (validation_result == EV_VALID) {
        MatrixTask_RequestRefresh();
        current = fsmStep(current, EV_NONE);
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
