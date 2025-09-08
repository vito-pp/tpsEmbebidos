#include "fsm_table.h"
#include "fsm.h"

static const FSM_State_t idle[];
static const FSM_State_t insert[];
static const FSM_State_t unlock[];

static const FSM_State_t idle[] = 
{
    {EV_ENTER, insert, printIDnPIN},
    {FSM_END, idle, printMenu}
};

static const FSM_State_t insert[]
{
    {EV_VALID, unlock, toggleLED},
    {EV_INVALID, idle, NULL},
    {EV_TIMEOUT, idle, NULL},
    {FSM_END, insert, NULL}
};

static const FSM_State_t unlock[]
{
    {EV_TIMEOUT, idle, NULL},
    {FSM_END, unlock, NULL}
};