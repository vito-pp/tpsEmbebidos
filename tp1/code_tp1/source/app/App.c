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

#include "../drv/board.h"
#include "../drv/rotary_encoder.h"
#include "../drv/gpio.h"
#include "../timer.h"
#include "../drv/SysTick.h"
#include "../drv/mag_strip.h"
#include "../drv/shift_registers.h"
#include "../display.h"
#include "fsm.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
static FSM_State_t *current;
uint8_t last_button_state = ENC_NONE;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    encoderInit();
    // current = getInitState();
	int i = magStrip_Init();
	int j = serialData_init();
	//int j = serialData_init();
    timerInit();
    tim_id_t id = timerGetId();
    if (id != TIMER_INVALID_ID)
        timerStart(id, 1, TIM_MODE_PERIODIC, encoder_callback);


    /*
    id = timerGetId();
    if (id != TIMER_INVALID_ID)
            timerStart(id, 500, TIM_MODE_PERIODIC, goo);    
    */
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    timerUpdate();
    uint8_t button_state = encoder_update();
    if (button_state != ENC_NONE && button_state != last_button_state)
    {
        gpioToggle(PIN_LED_BLUE);
    }
    last_button_state = button_state;

}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void delayLoop(uint32_t veces)
{
	while(veces--);
}
