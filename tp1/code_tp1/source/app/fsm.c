#include <stdio.h>
#include <unistd.h> // sleep()
#include <stdlib.h> // exit()
#include "fsm.h"
//#include "../drv/rotary_encoder.h"

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
 * GLOBAL SCOPE FUNCTION DEFINITIONS
 ******************************************************************************/

const FSM_State_t *getInitState(void)
{
    return idle;
}

const FSM_State_t *fsmStep(const FSM_State_t *state_table, FSM_event_t ev)
{
    // scan this state's possible transitions for a match or fallback
    while (state_table->event != ev && state_table->event != EV_NONE)
        ++state_table;

    // execute action and transition
    if(state_table->action != NULL)
    {
        state_table->action();
    }
    return state_table->next_state_table;
}

// FSM_event_t getEvent(void)
// {
//     if (magStripNewData())  return EV_MAG_DATA; // jump to insertPIN

//     if (isDataReady()) // when id&pin have been registered
//     {
//         return isValid() ? EV_VALID : EV_INVALID;
//     }
// 
//     // implement TIMEOUT   
// 
//     switch (encoderInput()) // input from the user
//     {
//     case ENC_CLICK:         return EV_ENTER;
//     case ENC_DOUBLE_CLICK:  return EV_DOUBLE_ENTER;
//     case ENC_FORWARD:       return EV_FORWARD;
//     case ENC_BACKWARD:      return EV_BACKWARD;
//     case ENC_HOLD:          return EV_RESET;
//     default:                return EV_NONE;
//     }
// }

/* ========= ACTION CALLBACKS FOR TESTING ========= */

// Idle
static void printMenu(void)         { puts("\r[Idle] Menu"); }
static void reset(void)             { puts("\r[Idle] Menu (reseted)"); }
static void printInsertId(void)     { puts("\r[Idle] -> Insert ID"); }

// Insert ID / PIN editing
static void increaseDigit(void)     { puts("\r[Insert] ++digit"); }
static void decreaseDigit(void)     { puts("\r[Insert] --digit"); }
static void storeDigit(void)        { puts("\r[Insert] stored digit"); }
static void eraseDigit(void)        { puts("\r[Insert] erased digit"); }
static void clearInput(void)        { puts("\r[Insert] clear input"); }
static void printInsertPin(void)    { puts("\r[Insert] -> Insert PIN"); }

// Validation & results
static void validateID(void)        { puts("\r[Validate] check ID"); }
static void validatePIN(void)       { puts("\r[Validate] check PIN"); }
static void lightLEDs(void)         { puts("\r[Unlock] LED ON"); }
static void printWrong(void)        { puts("\r[Validate] wrong creds"); }
static void validation(void)        { puts("\r[Validate] dispatch result"); }

// Unlock timing
static void unlockLEDOff(void)      { puts("\r[Unlock] LED OFF"); }
static void finalDelay(void)
{ 
    puts("\r[Unlock] wait done"); 
    sleep(1);
    exit(0);
}

/*******************************************************************************
 * DEFINITION OF STATES AND THEIR TRANSITIONS
 ******************************************************************************/

static const FSM_State_t idle[] = 
{
    {EV_ENTER, insert_id0, printInsertId},
    {EV_NONE, idle, NULL}
};

static const FSM_State_t insert_id0[] =
{
    {EV_ENTER, insert_id1, storeDigit},
    //{EV_DOUBLE_ENTER, idle, reset},
    {EV_FORWARD, insert_id1, increaseDigit},
    {EV_BACKWARD, insert_id1, decreaseDigit},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_MAG_DATA, insert_pin0, NULL},
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
    {EV_NONE, validate, validation}
};

static const FSM_State_t unlock[] =
{
    {EV_NONE, idle, finalDelay}
};