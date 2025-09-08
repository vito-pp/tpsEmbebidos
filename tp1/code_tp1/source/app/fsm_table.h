#ifndef FSM_TABLE_H_
#define FSM_TABLE_H_

#define FSM_END 0xFF

typedef enum 
{
    EV_ENTER,   // user pressed ENTER (from Idle -> start input)
    EV_VALID,   // ID+PIN were correct 
    EV_INVALID, // ID+PIN were incorrect
    EV_RESET,   // user holds button, so resets
    EV_TIMEOUT, // generic timeout
} FSM_Event_t;

#endif // FSM_TABLE_H_