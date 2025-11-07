/***************************************************************************//**
  @file     app.c
  @brief    Main Loop definition for TP3 of Embedded Systems
*******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "drv/hal/timer.h"
#include "drv/hal/NCO.h"
#include "drv/mcal/board.h"
#include "drv/mcal/gpio.h"
#include "drv/mcal/SysTick.h"
#include "drv/mcal/UART_strings.h"
#include "drv/mcal/UART.h"
#include "drv/mcal/dma.h"
#include "drv/mcal/pit.h"
#include "drv/mcal/ADC.h"
#include "drv/mcal/DAC.h"
#include "dsp/bitstream.h"
#include "dsp/demod_fsk.h"

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

#define RX_BUF_LEN 32
#define NUM_BUFFERS 2

/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/

static volatile uint16_t rx_buffers[NUM_BUFFERS][RX_BUF_LEN] __attribute__((aligned(4)));
static volatile uint16_t *current_buffer;
static volatile uint16_t *processing_buffer;
static volatile bool rx_ready = false;
static volatile uint8_t active_buffer = 0;
static char rx_word;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR CALLBACKS
 ******************************************************************************/

static void rx_pit_cb(void *user);
static void dma_rx_major_cb(void *user);
static void NCO_ISRBit(void *user);
static void NCO_ISRLut(void *user);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    // For the error handling
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
    gpioMode(PIN_TP1, OUTPUT);
    gpioWrite(PIN_TP1, LOW);
    
    UART_Init(UART_PARITY_ODD); 
    ADC_Init(true); // true = dma_req enable
    NCO_InitFixed(&nco_handle, K_MARK, K_SPACE, true);
    DAC_Init();
    PIT_Init();
    DMA_Init();
    
    // Initialize buffer pointers
    current_buffer = rx_buffers[0];
    processing_buffer = rx_buffers[1];
    dma_cfg_t dma_cfg =
    {
        .ch = 0,
        .request_src = DMA_REQ_ALWAYS63,
        .trig_mode = true,
        .saddr = (void *)&ADC0->R[0],
        .daddr = (void*)current_buffer,
        .nbytes = 2, // 16-bit
        .soff = 0, .doff = 2,
        .major_count = RX_BUF_LEN,
        .slast = 0,
        .dlast = 0,  // Don't reset address, we'll handle buffer switching
        .int_major = true,
        .on_major = dma_rx_major_cb,
        .user = NULL
    };
    DMA_Config(&dma_cfg);
    DMA_Start(0);

    // PIT configs
    pit_cfg_t pit_adc_cfg =
    {
        .ch = 0, // didnt work with ch2 (todo: check later)
        .load_val = PIT_TICKS_FROM_US(83), // 12kHz ADC sampling
        .periodic = true,
        .int_en = true,
        .dma_req = true, // PIT asserts DMA request
        .callback = rx_pit_cb,
        .user = NULL
    };
    PIT_Config(&pit_adc_cfg);

    pit_cfg_t pit_cfg_lut =
    {
        .ch = 1,
        .load_val = PIT_TICKS_FROM_US(20),
        .periodic = true,
        .int_en = true,
        .dma_req = false,
        .callback = NCO_ISRLut,
        .user = NULL
    };
    PIT_Config(&pit_cfg_lut);

    pit_cfg_t pit_cfg_bit =
    {
        .ch = 2,
        .load_val = PIT_TICKS_FROM_US(833),
        .periodic = true,
        .int_en = true,
        .dma_req = false,
        .callback = NCO_ISRBit,
        .user = NULL
    };
    PIT_Config(&pit_cfg_bit);

    // Inicializo los bitstreams en idle
    for (int i = 0; i < 11; i++)
    {
        idle_sending_bitstream[i] = true;
        idle_reciving_bitstream[i] = 0;
    }

    // FPU enable
    SCB->CPACR |= (0xF << 20);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    if (rx_ready)
    {
        rx_ready = false;
        // Process the buffer that was just filled
        for (int i = 0; i < RX_BUF_LEN; i++)
        {
            float d = demodFSK(processing_buffer[i]);        
            bitstreamReconstruction(d);
            if (isDataReady())
            {
                bool *frame = retrieveBitstream();
                rx_word = deformat_bitstream(frame);
                char rx_word_str[1];
                rx_word_str[0] = rx_word;
                UART_SendString(rx_word_str);
            }
        }
    }
}

/*******************************************************************************
 *******************************************************************************
    LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void rx_pit_cb(void *user)
{
    (void)user; // cookie disposal
    ADC_Start(ADC0, 1, ADC_mA); // starts adc's conversion
}

static void dma_rx_major_cb(void *user)
{
    // Swap buffers
    volatile uint16_t *temp = current_buffer;
    current_buffer = processing_buffer;
    processing_buffer = temp;
    
    // Update DMA destination address for next transfer
    DMA0->TCD[0].DADDR = (uint32_t)current_buffer;
    
    rx_ready = true;
}

static void NCO_ISRBit(void* user)
{
    (void)user; // cookie disposal

    // Flag de reset de Contador de bits enviados (Redundancia para seguridad)
    if (initiate_send)
    {
        cnt = 0;
        initiate_send = false;
    }

    // Decidir si envio en idle o si envio datos
    if (sending_data)
    {
        NCO_FskBit(&nco_handle, sending_bitstream[cnt]);
    }
    else
    {
        NCO_FskBit(&nco_handle, idle_sending_bitstream[cnt]);
    }

    cnt++;
    // Se setearon cnt bits en el NCO.
    if (cnt == 11)
    {
        sending_data = false;
        gpioWrite(PIN_LED_RED, !LED_ACTIVE);
        gpioWrite(PIN_TP1, LOW);
        cnt = 0;
    }
}

static void NCO_ISRLut(void *user)
{

    lut_value = NCO_TickQ15(&nco_handle);
    DAC_SetData(DAC0, lut_value);
}
