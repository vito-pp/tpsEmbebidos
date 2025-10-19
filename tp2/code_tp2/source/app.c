/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   todes
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>

#include "drv/board.h"
#include "drv/gpio.h"
#include "misc/timer.h"
#include "drv/SysTick.h"
#include "drv/i2c.h"
#include "drv/FXOS8700CQ.h"

static Vec3_t uT, mg;

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
static void __error_handler__(void);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
    if (!I2C_MasterInit(0, 9600))
    {
        __error_handler__();
    }
    if (!FXOS_Init(0))
    {
        __error_handler__();
    }
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    FXOS_ReadAccelerometer(&mg);
    //FXOS_ReadMagnetometer(&uT);
    I2C_ServicePoll(0);
    delayLoop(1000);
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

static void __error_handler__(void)
{
    gpioWrite(PIN_LED_RED, LED_ACTIVE);
}
