/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include "../drv/board.h"
#include "../drv/gpio.h"
#include "../timer.h"
#include "fsm.h"
#include "../drv/board.h"
#include "../drv/gpio.h"
#include "../drv/SysTick.h"
#include "../drv/mag_strip.h"
#include "../drv/shift_registers.h"
#include "../display.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
static FSM_State_t *current;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    timerInit();
    // tim_id_t id = timerGetId();
    // if (id != TIMER_INVALID_ID)
    //     timerStart(id, 1000, TIM_MODE_PERIODIC, NULL);

    // current = getInitState();
	int i = magStrip_Init();
	int j = serialData_init();
	//int j = serialData_init();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    // FSM_event_t event = getEvent();
    // if (event != EV_NONE)
        // current = fsmStep(current, event);

    display(1, 0, 0);
    display(2, 1, 0);
    display(3, 2, 0);
    display(4, 3, 0);

    int i;
    //timerUpdate();
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void delayLoop(uint32_t veces)
{
    while (veces--);
}

/*******************************************************************************
 ******************************************************************************/
