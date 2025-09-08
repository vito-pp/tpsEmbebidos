/**
 * @file fms.h
 * @brief Declaration of the FSM structure. 
 */

#ifndef FSM_H_
#define FSM_H_

#include <stdint.h>

typedef struct FSM_State_t
{
    uint8_t event;
    const struct FSM_State_t *next_state_table;
    void (*action)(void);
} FSM_State_t;

FSM_State_t *fsmStep(const FSM_State_t *state_table, uint8_t ev);

uint8_t getEvent(void);

#endif // FSM_H_