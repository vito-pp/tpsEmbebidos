/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "../drv/board.h"
#include "../drv/gpio.h"
#include "../timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
static void foo(void);
static void goo(void);

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
        timerStart(id, 1000, TIM_MODE_PERIODIC, foo);

    id = timerGetId();
    if (id != TIMER_INVALID_ID)
            timerStart(id, 500, TIM_MODE_PERIODIC, goo);
        
    gpioMode(PIN_LED_BLUE, OUTPUT);
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioMode(PORTNUM2PIN(PB, 2), OUTPUT);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    gpioWrite(PORTNUM2PIN(PB, 2), LOW);
    timerUpdate();
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

static void foo(void)
{
    gpioToggle(PIN_LED_BLUE);
    gpioWrite(PORTNUM2PIN(PB, 2), HIGH);
}

static void goo(void)
{
    gpioToggle(PIN_LED_RED);
    gpioWrite(PORTNUM2PIN(PB, 2), HIGH);
}

/*******************************************************************************
 ******************************************************************************/
