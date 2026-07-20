#include <stdbool.h>

#include <os.h>

#include "rtos/app_event.h"

#define APP_EVENT_QUEUE_SIZE  12u

typedef struct
{
    FSM_event_t event;
} AppEventMessage_t;

static OS_Q AppEventQueue;

static AppEventMessage_t event_messages[] =
{
    {EV_ENTER},
    {EV_DOUBLE_ENTER},
    {EV_FORWARD},
    {EV_BACKWARD},
    {EV_RESET},
    {EV_MAG_DATA},
    {EV_VALID},
    {EV_INVALID},
    {EV_TIMEOUT}
};

static AppEventMessage_t *AppEvent_GetMessage(FSM_event_t event);
static bool AppEvent_PostMessage(FSM_event_t event);

void AppEvent_Create(void)
{
    OS_ERR err;

    OSQCreate(&AppEventQueue,
              "Application Event Queue",
              APP_EVENT_QUEUE_SIZE,
              &err);

    if (err != OS_ERR_NONE) {
        while (1) {
        }
    }
}

bool AppEvent_Post(FSM_event_t event)
{
    return AppEvent_PostMessage(event);
}

bool AppEvent_PostFromISR(FSM_event_t event)
{
    return AppEvent_PostMessage(event);
}

AppEventPendResult_t AppEvent_Pend(FSM_event_t *event, OS_TICK timeout)
{
    OS_ERR err;
    OS_MSG_SIZE message_size;
    AppEventMessage_t *message;

    if (event == 0) {
        return APP_EVENT_PEND_ERROR;
    }

    message = OSQPend(&AppEventQueue,
                      timeout,
                      OS_OPT_PEND_BLOCKING,
                      &message_size,
                      0,
                      &err);

    if (err == OS_ERR_TIMEOUT) {
        return APP_EVENT_PEND_TIMEOUT;
    }

    if ((err != OS_ERR_NONE) ||
        (message == 0) ||
        (message_size != sizeof(*message))) {
        return APP_EVENT_PEND_ERROR;
    }

    *event = message->event;
    return APP_EVENT_PEND_RECEIVED;
}

void AppEvent_Flush(void)
{
    OS_ERR err;

    OSQFlush(&AppEventQueue, &err);
}

static AppEventMessage_t *AppEvent_GetMessage(FSM_event_t event)
{
    if (event > EV_TIMEOUT) {
        return 0;
    }

    return &event_messages[event];
}

static bool AppEvent_PostMessage(FSM_event_t event)
{
    OS_ERR err;
    AppEventMessage_t *message = AppEvent_GetMessage(event);

    if (message == 0) {
        return false;
    }

    OSQPost(&AppEventQueue,
            message,
            sizeof(*message),
            OS_OPT_POST_FIFO,
            &err);

    return (err == OS_ERR_NONE);
}
