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
#include "drv/mcal/ADC.h"

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

#define TX 1
#define RX 0 
#define MODEM_MODE RX

/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/

 #if MODEM_MODE == RX
#define RX_BUF_LEN 32
static uint16_t rx_buffer[RX_BUF_LEN] __attribute__((aligned(4)));
static volatile bool rx_ready = false;
#endif

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
static void __error_handler__(void);

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR CALLBACKS
 ******************************************************************************/

 #if MODEM_MODE == RX
static void dma_rx_major_cb(void *user);
#endif 

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    // for the error handler
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
    
    // UART init
    UART_Init(UART_PARITY_ODD); 

#if MODEM_MODE == RX 
    // DMA init
    DMA_Init();
    dma_cfg_t dma_cfg =
    {
        .ch = 0,
        .request_src = DMA_REQ_ALWAYS63,
        .trig_mode = true,
        .saddr = (void *)&ADC0->R[0],
        .daddr = rx_buffer,
        .nbytes = 2, // 16-bit
        .soff = 0, .doff = 2,
        .major_count = RX_BUF_LEN,
        .slast = 0,
        .dlast = -(RX_BUF_LEN * 2),
        .int_major = true,
        .on_major = dma_rx_major_cb,
        .user = NULL
    };
    DMA_Config(&dma_cfg);
    DMA_Start(0);

    // PIT init
    PIT_Init();
    pit_cfg_t pit_cfg =
    {
        .ch = PIT_CH0,
        .load_val = PIT_TICKS_FROM_US(83), // 12kHz ADC sampling
        .periodic = true,
        .int_en = false,
        .dma_req = true, // PIT asserts DMA request
        .callback = NULL,
        .user = NULL
    };
    PIT_Config(&pit_cfg);

    ADC_Init(true); // true = dma_req enable

    // FPU enable
    SCB->CPACR |= (0xF << 20);

#else // TX MODE

#endif
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
#if MODEM_MODE == RX
    if (rx_ready)
    {
        rx_ready = false;
        for (int i = 0; i < RX_BUF_LEN; i++)
        {
            float d = demodFSK(rx_buffer[i]);
            bitstreamReconstruction(d);
            if (isDataReady())
            {
                bool *frame = retrieveBitstream();
                // send UART frame
            }
        }
    }
#else // TX MODE
    
#endif
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

static void dma_rx_major_cb(void *user)
{
    (void)user;
    rx_ready = true;
}
