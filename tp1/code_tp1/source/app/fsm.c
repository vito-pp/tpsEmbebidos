#include "fsm.h"
#include "fsm_table.h"

fsm_step(const State_t *state_table, FSM_Event_t ev)
{
    // scan this state's rows for a match or fallback
    while (state_table->event != ev && state_table->event != FSM_END)
        ++state_table;

    // execute action and transition
    state_table->action();
    return state_table->next_state_table;
}