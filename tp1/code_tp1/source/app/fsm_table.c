#include "fsm.h"

typedef enum 
{
    EV_ENTER,   // user pressed ENTER (from Idle -> start input)
    EV_VALID,   // ID+PIN were correct 
    EV_INVALID, // ID+PIN were incorrect
    EV_RESET,   // user holds button, so resets
    EV_TIMEOUT, // generic timeout
} FSM_Event_t;

static const State_t idle[];
static const State_t insert[];
static const State_t unlock[];

static const State_t idle[] = 
{
    {EV_ENTER, insert, printIDnPIN},
    {FSM_END, idle, printMenu}
};

static const State_t insert[]
{
    {EV_VALID, unlock, toggleLED},
    {EV_INVALID, idle, NULL},
    {EV_TIMEOUT, idle, NULL},
    {FSM_END, insert, NULL}
};

static const State_t unlock[]
{
    {EV_TIMEOUT, idle, NULL},
    {FSM_END, unlock, NULL}
};