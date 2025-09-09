/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "../drv/board.h"
#include "../drv/rotary_encoder.h"
#include "../drv/gpio.h"
#include "../timer.h"
#include <stdio.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
uint8_t last_button_state = ENC_NONE;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    timerInit();
    tim_id_t id = timerGetId();
    if (id != TIMER_INVALID_ID)
        timerStart(id, 1, TIM_MODE_PERIODIC, encoder_update);

    /*
    id = timerGetId();
    if (id != TIMER_INVALID_ID)
            timerStart(id, 500, TIM_MODE_PERIODIC, goo);    
    */
    gpioMode(PORTNUM2PIN(PB, 2), INPUT);
    gpioMode(PORTNUM2PIN(PB, 3), INPUT);
    gpioMode(PORTNUM2PIN(PB, 10), INPUT);
    gpioMode(PIN_LED_BLUE, OUTPUT);


}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    timerUpdate();
    uint8_t button_state = encoder_update();
    if (button_state != ENC_NONE && button_state != last_button_state){
        gpioToggle(PIN_LED_BLUE);
    }
    last_button_state = button_state;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
