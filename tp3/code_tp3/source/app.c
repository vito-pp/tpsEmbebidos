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

static const uint32_t led_mask_toggle  = (1 << 21);

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
    // for dma to write
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioMode(PIN_LED_GREEN, OUTPUT);
    gpioMode(PIN_LED_BLUE, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
    gpioWrite(PIN_LED_GREEN, !LED_ACTIVE);
    gpioWrite(PIN_LED_BLUE, !LED_ACTIVE);

    // DMA init
    DMA_Init();
    dma_cfg_t dma_cfg =
    {
        .ch = 0,
        .request_src = DMA_REQ_ALWAYS63,
        .trig_mode = true,
        .saddr = (void *)&led_mask_toggle,
        .daddr = (void *)&(PTB->PTOR),
        .nbytes = 4,
        .soff = 0, .doff = 0,
        .major_count = 1,
        .slast = 0,
        .dlast = 0,
        .int_major = false,
        .on_major = NULL,
        .user = NULL
    };
    DMA_Config(&dma_cfg);
    DMA_Start(0);

    // PIT init
    PIT_Init();
    pit_cfg_t pit_cfg =
    {
        .ch = PIT_CH0,
        .load_val = PIT_TICKS_FROM_MS(1000),
        .periodic = true,
        .int_en = false,
        .dma_req = true,
        .callback = NULL,
        .user = NULL
    };
    PIT_Config(&pit_cfg);
    PIT_Start(PIT_CH0);

    // FPU enable
    SCB->CPACR |= (0xF << 20);
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
