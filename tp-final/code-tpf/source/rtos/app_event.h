#ifndef APP_EVENT_H_
#define APP_EVENT_H_

#include <stdbool.h>

#include <os.h>

#include "ui/fsm.h"

typedef enum
{
    APP_EVENT_PEND_RECEIVED,
    APP_EVENT_PEND_TIMEOUT,
    APP_EVENT_PEND_ERROR
} AppEventPendResult_t;

void AppEvent_Create(void);
bool AppEvent_Post(FSM_event_t event);
bool AppEvent_PostFromISR(FSM_event_t event);
AppEventPendResult_t AppEvent_Pend(FSM_event_t *event, OS_TICK timeout);
void AppEvent_Flush(void);

#endif /* APP_EVENT_H_ */
