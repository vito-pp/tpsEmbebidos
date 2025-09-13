/**
 * @file fms.h
 * @brief Declaration of the FSM structure. 
 */

#ifndef FSM_H_
#define FSM_H_

#include <stdint.h>

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum 
{
    EV_ENTER,
    EV_DOUBLE_ENTER,  
    EV_FORWARD,   
    EV_BACKWARD, 
    EV_RESET,
    EV_MAG_DATA,
    EV_VALID,
    EV_INVALID,
    EV_TIMEOUT,
    EV_NONE = 0xFF
} FSM_event_t;

typedef struct FSM_State_t
{
    FSM_event_t event;
    const struct FSM_State_t *next_state_table;
    void (*action)(void);
} FSM_State_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

const FSM_State_t *fsmStep(const FSM_State_t *state_table, FSM_event_t ev);

FSM_event_t getEvent(void);

const FSM_State_t *getInitState(void);

#endif // FSM_H_