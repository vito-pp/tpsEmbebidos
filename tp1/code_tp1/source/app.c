/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>

#include "drv/board.h"
#include "drv/rotary_encoder.h"
#include "drv/gpio.h"
#include "misc/timer.h"
#include "drv/SysTick.h"
#include "drv/mag_strip.h"
#include "drv/shift_registers.h"
#include "ui/display.h"
#include "ui/fsm.h"
#include "ui/auth_ui.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

int pan2Id(uint64_t pan);

static FSM_State_t *current;
static FSM_event_t event;

enc_input_t last_button_state = ENC_NONE;

tim_id_t inactivity_id;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    encoderInit();

	magStrip_Init();

	display_init();

    current = getInitState();

    timerInit();
    tim_id_t id = timerGetId();
    if (id != TIMER_INVALID_ID)
    {
        timerStart(id, 1, TIM_MODE_PERIODIC, encoder_callback);
    }
    inactivity_id = timerGetId();
    if (inactivity_id != TIMER_INVALID_ID)
    {
        timerStart(id, 5000, TIM_MODE_PERIODIC, triggerTimeout);
    }
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    event = getEvent();
    current = fsmStep(current, event);

    if (event != EV_NONE)
    {
        timerStop(inactivity_id);
        timerStart(inactivity_id, 5000, TIM_MODE_PERIODIC, triggerTimeout);
    }

    timerUpdate();
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
