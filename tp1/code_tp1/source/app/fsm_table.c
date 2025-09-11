#include "fsm_table.h"
#include "fsm.h"

/*******************************************************************************
 * DECLARIATION OF STATES
 ******************************************************************************/

static const FSM_State_t idle[];

static const FSM_State_t insert_id0[];
static const FSM_State_t insert_id1[];
static const FSM_State_t insert_id2[];
static const FSM_State_t insert_id3[];
static const FSM_State_t insert_id4[];
static const FSM_State_t insert_id5[];
static const FSM_State_t insert_id6[];
static const FSM_State_t insert_id7[];

static const FSM_State_t insert_pin0[];
static const FSM_State_t insert_pin1[];
static const FSM_State_t insert_pin2[];
static const FSM_State_t insert_pin3[];
static const FSM_State_t insert_pin4[];

static const FSM_State_t validate[];

static const FSM_State_t unlock[];

/*******************************************************************************
 * DEFINITION OF STATES
 ******************************************************************************/

static const FSM_State_t idle[] = 
{
    {EV_ENTER, insert_id0, printInsertId},
    {EV_NONE, idle, printMenu}
};

static const FSM_State_t insert_id0[] =
{
    {EV_ENTER, insert_id1, storeDigit},
    //{EV_DOUBLE_ENTER, idle, reset},
    {EV_FORWARD, insert_id1, increaseDigit},
    {EV_BACKWARD, insert_id1, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id0, NULL}
};

static const FSM_State_t insert_id1[] =
{
    {EV_ENTER, insert_id2, storeDigit},
    {EV_DOUBLE_ENTER, insert_id0, eraseDigit},
    {EV_FORWARD, insert_id1, increaseDigit},
    {EV_BACKWARD, insert_id1, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id1, NULL}
};

static const FSM_State_t insert_id2[] =
{
    {EV_ENTER, insert_id3, storeDigit},
    {EV_DOUBLE_ENTER, insert_id1, eraseDigit},
    {EV_FORWARD, insert_id2, increaseDigit},
    {EV_BACKWARD, insert_id2, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id2, NULL}
};

static const FSM_State_t insert_id3[] =
{
    {EV_ENTER, insert_id4, storeDigit},
    {EV_DOUBLE_ENTER, insert_id2, eraseDigit},
    {EV_FORWARD, insert_id3, increaseDigit},
    {EV_BACKWARD, insert_id3, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id3, NULL}
};

static const FSM_State_t insert_id4[] =
{
    {EV_ENTER, insert_id5, storeDigit},
    {EV_DOUBLE_ENTER, insert_id3, eraseDigit},
    {EV_FORWARD, insert_id4, increaseDigit},
    {EV_BACKWARD, insert_id4, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id4, NULL}
};

static const FSM_State_t insert_id5[] =
{
    {EV_ENTER, insert_id6, storeDigit},
    {EV_DOUBLE_ENTER, insert_id4, eraseDigit},
    {EV_FORWARD, insert_id5, increaseDigit},
    {EV_BACKWARD, insert_id5, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id5, NULL}
};

static const FSM_State_t insert_id6[] =
{
    {EV_ENTER, insert_id7, storeDigit},
    {EV_DOUBLE_ENTER, insert_id5, eraseDigit},
    {EV_FORWARD, insert_id6, increaseDigit},
    {EV_BACKWARD, insert_id6, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id6, NULL}
};

static const FSM_State_t insert_id7[] =
{
    {EV_ENTER, insert_pin0, validateID},
    {EV_DOUBLE_ENTER, insert_id6, eraseDigit},
    {EV_FORWARD, insert_id7, increaseDigit},
    {EV_BACKWARD, insert_id7, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id7, NULL}
};

static const FSM_State_t insert_pin0[] =
{
    {EV_ENTER, insert_pin1, storeDigit},
    //{EV_DOUBLE_ENTER, insert_, eraseDigit},
    {EV_FORWARD, insert_pin0, increaseDigit},
    {EV_BACKWARD, insert_pin0, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin0, NULL}
};

static const FSM_State_t insert_pin1[] =
{
    {EV_ENTER, insert_pin2, storeDigit},
    {EV_DOUBLE_ENTER, insert_pin0, eraseDigit},
    {EV_FORWARD, insert_pin1, increaseDigit},
    {EV_BACKWARD, insert_pin1, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin1, NULL}
};

static const FSM_State_t insert_pin2[] =
{
    {EV_ENTER, insert_pin3, storeDigit},
    {EV_DOUBLE_ENTER, insert_pin1, eraseDigit},
    {EV_FORWARD, insert_pin2, increaseDigit},
    {EV_BACKWARD, insert_pin2, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin2, NULL}
};

static const FSM_State_t insert_pin3[] =
{
    {EV_ENTER, insert_pin4, storeDigit},
    {EV_DOUBLE_ENTER, insert_pin2, eraseDigit},
    {EV_FORWARD, insert_pin3, increaseDigit},
    {EV_BACKWARD, insert_pin3, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin3, NULL}
};

static const FSM_State_t insert_pin4[] =
{
    {EV_ENTER, validate, validatePIN},
    {EV_DOUBLE_ENTER, insert_pin3, eraseDigit},
    {EV_FORWARD, insert_pin4, increaseDigit},
    {EV_BACKWARD, insert_pin4, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin4, NULL}
};

static const FSM_State_t validate[] =
{
    {EV_VALID, unlock, lightLEDs},
    {EV_INVALID, idle, printWrong},
    {EV_NONE, validate, isValid}
};

static const FSM_State_t unlock[] =
{
    {EV_NONE, idle, delayLoop}
};

/*******************************************************************************
 * GLOBAL SCOPE FUNCTION DEFINITIONS
 ******************************************************************************/

FSM_State_t *getInitState(void)
{
    return idle;
}

