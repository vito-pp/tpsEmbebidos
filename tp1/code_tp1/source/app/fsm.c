#include "fsm.h"
#include "fsm_table.h"
#include "../drv/rotary_encoder.h"

FSM_State_t *fsmStep(const FSM_State_t *state_table, FSM_event_t ev)
{
    // scan this state's possible transitions for a match or fallback
    while (state_table->event != ev && state_table->event != EV_NONE)
        ++state_table;

    // execute action and transition
    state_table->action();
    return state_table->next_state_table;
}

uint8_t getEvent(void)
{
    if (magStripNewData())  return EV_MAG_DATA;

    switch (encoderInput())
    {
    case ENC_CLICK:         return EV_ENTER;
    case ENC_DOUBLE_CLICK:  return EV_DOUBLE_ENTER;
    case ENC_FORWARD:       return EV_FORWARD;
    case ENC_BACKWARD:      return EV_BACKWARD;
    case ENC_HOLD:          return EV_RESET;
    default:                return EV_NONE;
    }
}
