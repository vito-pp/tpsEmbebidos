/***************************************************************************//**
  @file     App.c
  @brief    Main Loop definition for TP3 of Embedded Systems
*******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "drv/hal/timer.h"
#include "drv/mcal/board.h"
#include "drv/mcal/gpio.h"
#include "drv/mcal/SysTick.h"
#include "drv/mcal/UART_strings.h"
#include "drv/mcal/UART.h"
#include "drv/mcal/dma.h"
#include "drv/mcal/pit.h"

/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
static void __error_handler__(void);

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR CALLBACKS
 ******************************************************************************/

static void pit_cb(void * cb_param);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    // error handler init
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);

    // PIT cb
    gpioMode(PIN_LED_GREEN, OUTPUT);
    gpioWrite(PIN_LED_GREEN, !LED_ACTIVE);

    // PIT init
    PIT_Init();
    pit_cfg_t cfg =
    {
        .ch = PIT_CH0,
        .load_val = PIT_TICKS_FROM_MS(1000),
        .periodic = true,
        .int_en = true,
        .dma_req = false,
        .callback = pit_cb,
        .user = NULL
    };
    PIT_Config(&cfg);
    PIT_Start(PIT_CH0);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    int i = 0;
    i++;
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

static void pit_cb(void *cb_param)
{
    (void *)cb_param;
    gpioToggle(PIN_LED_GREEN);
}
