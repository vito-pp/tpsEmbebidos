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

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    I2C_MasterInit(0, 9600);
    FXOS_Init(0);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    FXOS_ReadAccelerometer(&mg);
    FXOS_ReadMagnetometer(&uT);
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
