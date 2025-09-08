#include "fsm.h"
#include "fsm_table.h"

FSM_State_t *fsmStep(const FSM_State_t *state_table, uint8_t ev)
{
    // scan this state's possible transitions for a match or fallback
    while (state_table->event != ev && state_table->event != FSM_END)
        ++state_table;

    // execute action and transition
    state_table->action();
    return state_table->next_state_table;
}

uint8_t getEvent(void)
{
    
}