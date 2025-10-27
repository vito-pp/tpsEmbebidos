/***************************************************************************//**
  @file     App.c
  @brief    Test simple: enviar el mismo mensaje continuamente por UART0
*******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "drv/board.h"
#include "drv/gpio.h"
#include "misc/timer.h"
#include "drv/SysTick.h"
#include "drv/UART_strings.h"
#include "drv/UART.h"
#include "drv/dma.h"

/****************
 * TESTING STUFF
 ****************/

#define N 10
static uint16_t src[N] = {0x1234,0x6789,0x1122,0x2233,0x5588,0x2345,0x3145,
                        0x8172,0x6183,0x3756};
static uint8_t dst[N];
static volatile bool g_done = false;

static void dma_major_cb(void *u);

/*******************************************************************************
 * FILE SCOPE VARIABLES
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
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
    gpioMode(PIN_LED_BLUE, OUTPUT);
    gpioWrite(PIN_LED_BLUE, !LED_ACTIVE);

    DMA_Init();
    dma_cfg_t cfg =
    {
        .ch          = 0,
        .request_src = DMA_REQ_ALWAYS63,
        .saddr       = src,
        .daddr       = dst,
        .elem_size   = 1,          
        .soff        = 2,          
        .doff        = 1,          
        .major_count = N,         
        .slast       = 0,          
        .dlast       = 0,          
        .int_major   = true,       
        .on_major    = dma_major_cb,
        .user        = NULL
    };
    DMA_Config(&cfg);
    DMA_Start(0);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	;
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

static void dma_major_cb(void *u) {
    (void)u;
    // One-shot: stop channel so it doesn’t keep re-running
    gpioWrite(PIN_LED_BLUE, LED_ACTIVE);
    g_done = true;
}
