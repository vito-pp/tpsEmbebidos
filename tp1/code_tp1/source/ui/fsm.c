#include <stdio.h>
//#include <unistd.h> // sleep()
//#include <stdlib.h> // exit()

#include "fsm.h"
#include "display.h"
#include "auth_ui.h"
#include "../drv/rotary_encoder.h"
#include "../drv/mag_strip.h"

/*******************************************************************************
 * DECLARIATION OF STATES
 ******************************************************************************/

static FSM_State_t idle[];

static FSM_State_t insert_id0[];
static FSM_State_t insert_id1[];
static FSM_State_t insert_id2[];
static FSM_State_t insert_id3[];
static FSM_State_t insert_id4[];
static FSM_State_t insert_id5[];
static FSM_State_t insert_id6[];
static FSM_State_t insert_id7[];

static FSM_State_t insert_pin0[];
static FSM_State_t insert_pin1[];
static FSM_State_t insert_pin2[];
static FSM_State_t insert_pin3[];
static FSM_State_t insert_pin4[];

static FSM_State_t validate[];

static FSM_State_t unlock[];

/*******************************************************************************
 * GLOBAL SCOPE FUNCTION DEFINITIONS
 ******************************************************************************/

FSM_State_t *getInitState(void)
{
    return idle;
}

FSM_State_t *fsmStep(FSM_State_t *state_table, FSM_event_t ev)
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

FSM_event_t getEvent(void)
{
    if (getIsDataReady())  return EV_MAG_DATA; // jump to insertPIN

    // if (isDataReady()) // when id&pin have been registered
    // {
    //     return isValid() ? EV_VALID : EV_INVALID;
    // }

    // implement TIMEOUT   

    switch (encoder_update()) // input from the user
    {
    case ENC_BUTTON_PRESS:      return EV_ENTER;
    case ENC_DOUBLE_PRESS:      return EV_DOUBLE_ENTER;
    case ENC_CW:                return EV_FORWARD;
    case ENC_CCW:               return EV_BACKWARD;
    case ENC_BUTTON_LONG_PRESS: return EV_RESET;
    case ENC_NONE:              return EV_NONE;
    default:                    return EV_NONE;
    }
}

/* ========= ACTION CALLBACKS FOR TESTING ========= */

// // Idle
// static void printMenu(void)         { puts("\r[Idle] Menu"); }
// static void reset(void)             { puts("\r[Idle] Menu (reseted)"); }
// static void printInsertId(void)     { puts("\r[Idle] -> Insert ID"); }

// // Insert ID / PIN editing
// static void increaseDigit(void)     { puts("\r[Insert] ++digit"); }
// static void decreaseDigit(void)     { puts("\r[Insert] --digit"); }
// static void storeDigit(void)        { puts("\r[Insert] stored digit"); }
// static void eraseDigit(void)        { puts("\r[Insert] erased digit"); }
// static void clearInput(void)        { puts("\r[Insert] clear input"); }
// static void printInsertPin(void)    { puts("\r[Insert] -> Insert PIN"); }

// // Validation & results
// static void validateID(void)        { puts("\r[Validate] check ID"); }
// static void validatePIN(void)       { puts("\r[Validate] check PIN"); }
// static void lightLEDs(void)         { puts("\r[Unlock] LED ON"); }
// static void printWrong(void)        { puts("\r[Validate] wrong creds"); }
// static void validation(void)        { puts("\r[Validate] dispatch result"); }

// // Unlock timing
// static void unlockLEDOff(void)      { puts("\r[Unlock] LED OFF"); }
// static void finalDelay(void)
// { 
//     puts("\r[Unlock] wait done"); 
//     sleep(1);
//     exit(0);
// }

/*******************************************************************************
 * DEFINITION OF STATES AND THEIR TRANSITIONS
 ******************************************************************************/

static FSM_State_t idle[] = 
{
    {EV_ENTER, insert_id0, NULL},
    {EV_FORWARD, idle, increaseBrightness},
    {EV_BACKWARD, idle, decreaseBrightness},
    {EV_NONE, idle, printMenu}
};

static FSM_State_t insert_id0[] =
{
    {EV_ENTER, insert_id1, storeDigitID},
    {EV_DOUBLE_ENTER, idle, reset},
    {EV_FORWARD, insert_id0, increaseDigitID},
    {EV_BACKWARD, insert_id0, decreaseDigitID},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_MAG_DATA, insert_pin0, NULL},
    {EV_NONE, insert_id0, printID}
};

static FSM_State_t insert_id1[] =
{
    {EV_ENTER, insert_id2, storeDigitID},
    {EV_DOUBLE_ENTER, insert_id0, eraseDigitID},
    {EV_FORWARD, insert_id1, increaseDigitID},
    {EV_BACKWARD, insert_id1, decreaseDigitID},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id1, printID}
};

static FSM_State_t insert_id2[] =
{
    {EV_ENTER, insert_id3, storeDigitID},
    {EV_DOUBLE_ENTER, insert_id1, eraseDigitID},
    {EV_FORWARD, insert_id2, increaseDigitID},
    {EV_BACKWARD, insert_id2, decreaseDigitID},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id2, printID}
};

static FSM_State_t insert_id3[] =
{
    {EV_ENTER, insert_id4, storeDigitID},
    {EV_DOUBLE_ENTER, insert_id2, eraseDigitID},
    {EV_FORWARD, insert_id3, increaseDigitID},
    {EV_BACKWARD, insert_id3, decreaseDigitID},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id3, printID}
};

static FSM_State_t insert_id4[] =
{
    {EV_ENTER, insert_id5, storeDigitID},
    {EV_DOUBLE_ENTER, insert_id3, eraseDigitID},
    {EV_FORWARD, insert_id4, increaseDigitID},
    {EV_BACKWARD, insert_id4, decreaseDigitID},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id4, printID}
};

static FSM_State_t insert_id5[] =
{
    {EV_ENTER, insert_id6, storeDigitID},
    {EV_DOUBLE_ENTER, insert_id4, eraseDigitID},
    {EV_FORWARD, insert_id5, increaseDigitID},
    {EV_BACKWARD, insert_id5, decreaseDigitID},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id5, printID}
};

static FSM_State_t insert_id6[] =
{
    {EV_ENTER, insert_id7, storeDigitID},
    {EV_DOUBLE_ENTER, insert_id5, eraseDigitID},
    {EV_FORWARD, insert_id6, increaseDigitID},
    {EV_BACKWARD, insert_id6, decreaseDigitID},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id6, printID}
};

static FSM_State_t insert_id7[] =
{
    {EV_ENTER, insert_pin0, storeDigitID},
    {EV_DOUBLE_ENTER, insert_id6, eraseDigitID},
    {EV_FORWARD, insert_id7, increaseDigitID},
    {EV_BACKWARD, insert_id7, decreaseDigitID},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_id7, printID}
};

static FSM_State_t insert_pin0[] =
{
    {EV_ENTER, insert_pin1, storeDigitPIN},
    //{EV_DOUBLE_ENTER, insert_, eraseDigitPIN},
    {EV_FORWARD, insert_pin0, increaseDigitPIN},
    {EV_BACKWARD, insert_pin0, decreaseDigitPIN},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin0, printPIN}
};

static FSM_State_t insert_pin1[] =
{
    {EV_ENTER, insert_pin2, storeDigitPIN},
    {EV_DOUBLE_ENTER, insert_pin0, eraseDigitPIN},
    {EV_FORWARD, insert_pin1, increaseDigitPIN},
    {EV_BACKWARD, insert_pin1, decreaseDigitPIN},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin1, printPIN}
};

static FSM_State_t insert_pin2[] =
{
    {EV_ENTER, insert_pin3, storeDigitPIN},
    {EV_DOUBLE_ENTER, insert_pin1, eraseDigitPIN},
    {EV_FORWARD, insert_pin2, increaseDigitPIN},
    {EV_BACKWARD, insert_pin2, decreaseDigitPIN},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin2, printPIN}
};

static FSM_State_t insert_pin3[] =
{
    {EV_ENTER, insert_pin4, storeDigitPIN},
    {EV_DOUBLE_ENTER, insert_pin2, eraseDigitPIN},
    {EV_FORWARD, insert_pin3, increaseDigitPIN},
    {EV_BACKWARD, insert_pin3, decreaseDigitPIN},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin3, printPIN}
};

static FSM_State_t insert_pin4[] =
{
    {EV_ENTER, validate, storeDigitPIN},
    {EV_DOUBLE_ENTER, insert_pin3, eraseDigitPIN},
    {EV_FORWARD, insert_pin4, increaseDigitPIN},
    {EV_BACKWARD, insert_pin4, decreaseDigitPIN},
    {EV_RESET, idle, reset},
    {EV_TIMEOUT, idle, reset},
    {EV_NONE, insert_pin4, printPIN}
};

static FSM_State_t validate[] =
{
    {EV_VALID, unlock, NULL},
    {EV_INVALID, idle, NULL},
    {EV_NONE, validate, NULL}
};

static FSM_State_t unlock[] =
{
    {EV_NONE, idle, NULL}
};
