/***************************************************************************//**
  @file     app.c
  @brief    Main Loop definition for TP3 of Embedded Systems v1
  @authors  Grupo 1
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
#include "drv/mcal/SysTick.h"
#include "dsp/bitstream.h"
#include "dsp/demod_fsk.h"

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

#define RX_BUFFER_SIZE 32
#define TX_BUFFER_SIZE 2048

/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/
// Variables de recepcion de datos. Strings
static uint16_t rx_buffer[RX_BUFFER_SIZE] __attribute__((aligned(4)));
static char rx_word[2048];
static char rx_line[2048];

// Circular buffer for transmission data
static char tx_buffer[TX_BUFFER_SIZE];
static volatile size_t tx_head = 0; 
static volatile size_t tx_tail = 0; 

static NCO_Handle nco_handle;

// Bitstream sent by the DAC
static bool sending_bitstream[11];
static bool idle_sending_bitstream[11];
static bool idle_reciving_bitstream[11];
static volatile uint16_t lut_value;
static size_t bit_cnt;


// Miscellaneous Variables - Manejo de Estados
static char idle_counter;
static bool last_byte_idle;
static volatile bool rx_ready = false;
static volatile bool initiate_send = false;
static volatile bool sending_data = false;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR CALLBACKS
 ******************************************************************************/

static void rx_pit_cb(void *user);
static void dma_rx_major_cb(void *user);
static void NCO_ISRBit(void);
static void NCO_ISRLut(void *user);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

 
void App_Init(void)
{
    // For the debugging and meassuring times
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
    gpioMode(PIN_LED_BLUE, OUTPUT);
    gpioWrite(PIN_LED_BLUE, !LED_ACTIVE);
    gpioMode(PIN_TP1, OUTPUT);
    gpioWrite(PIN_TP1, LOW);
    gpioMode(PIN_TP2, OUTPUT);
    gpioWrite(PIN_TP2, LOW);
    gpioMode(PIN_TP3, OUTPUT);
    gpioWrite(PIN_TP3, LOW);
    gpioMode(PIN_TP4, OUTPUT);
    gpioWrite(PIN_TP4, LOW);
    
    UART_Init(UART_PARITY_ODD); 
    ADC_Init(true); // true = dma_req enable
    NCO_InitFixed(&nco_handle, K_MARK, K_SPACE, true);
    DAC_Init();
    PIT_Init();
    DMA_Init();

    dma_cfg_t dma_dac_cfg =
    {
        .ch = 0,
        .request_src = DMA_REQ_ALWAYS63,
        .trig_mode = true, // PIT ch0 will trigger this dma req
        .saddr = (void*)&lut_value,
        .daddr = (void*)&DAC0->DAT[0].DATL,
        .nbytes = 2, // 16-bit
        .soff = 0, .doff = 0,
        .major_count = 1,
        .slast = 0,
        .dlast = 0,
        .int_major = false,
        .on_major = NULL,
        .user = NULL
    };
    DMA_Config(&dma_dac_cfg);
    DMA_Start(0);

    dma_cfg_t dma_adc_cfg = // ToDo: implement ping pong buffer
    {
        .ch = 1,
        .request_src = DMA_REQ_ADC0,
        .trig_mode = false,
        .saddr = (void *)&ADC0->R[0],
        .daddr = rx_buffer,
        .nbytes = 2, // 16-bit
        .soff = 0, .doff = 2,
        .major_count = RX_BUFFER_SIZE,
        .slast = 0,
        .dlast = -(RX_BUFFER_SIZE * 2),
        .int_major = true,
        .on_major = dma_rx_major_cb,
        .user = NULL
    };
    DMA_Config(&dma_adc_cfg);
    DMA_Start(1);

    // use systick for bit ISR
	SysTick_Init(NCO_ISRBit, 83333); // systick isr at 1.2kHz

    // PIT configs
    pit_cfg_t pit_cfg_lut =
    {
        .ch = 0, // has to be the same channel as DMA channel to be triggered
        .load_val = PIT_TICKS_FROM_US(20), // DAC's output refresh rate
        .periodic = true,
        .int_en = true,
        .dma_req = true,
        .callback = NCO_ISRLut,
        .user = NULL
    };
    PIT_Config(&pit_cfg_lut);

    pit_cfg_t pit_adc_cfg =
    {
        .ch = 1,
        .load_val = PIT_TICKS_FROM_US(83), // 12kHz ADC sampling
        .periodic = true,
        .int_en = true,
        .dma_req = false,
        .callback = rx_pit_cb, // PIT starts ADC's start of conversion
        .user = NULL
    };
    PIT_Config(&pit_adc_cfg);

    // Inicializo los bitstreams en idle
    for (int i = 0; i < 11; i++)
    {
        idle_sending_bitstream[i] = true;
        idle_reciving_bitstream[i] = 0;
    }

    // FPU enable
    SCB->CPACR |= (0xF << 20);
}

void App_Run(void)
{
    // TX main loop
    UART_Poll();
    int r = UART_ReceiveString(rx_line, sizeof(rx_line));

    if (r > 0)
    {
        // Queue all the recieved characters on the ring buffer
        for (int i = 0; i < r; i++)
        {
            size_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;

            if (next_head != tx_tail)
            {
                tx_buffer[tx_head] = rx_line[i];
                tx_head = next_head;
            }
        }

        // UART_SendString(rx_line); // Echo from the recieve data
    }

    // // If we aren't recieving data, send whats in the buffer
    if (!sending_data && tx_head != tx_tail)
    {
<<<<<<< HEAD
        if (idle_counter == 20){
=======
        if (idle_counter == 4){
>>>>>>> 7bb88c13bd4e796cc1c6cd59c516845ae8321edb
            last_byte_idle = true;
            idle_counter = 0;
        }

        if (last_byte_idle)
        {
            format_bitstream(tx_buffer[tx_tail], sending_bitstream);
            tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;

            initiate_send = true;
            sending_data = true;

        // Hacer un echo del caracter realmente enviado
        //UART_SendString((char[]){tx_buffer[tx_tail], '\0'});
            gpioWrite(PIN_LED_RED, LED_ACTIVE);
            gpioWrite(PIN_TP1, HIGH);
        }
    }
    UART_Poll();

    // RX main loop
    if (rx_ready)
    {
        rx_ready = false;
        for (int i = 0; i < RX_BUFFER_SIZE; i++)
        {
            float d = demodFSK(rx_buffer[i]);
            bitstreamReconstruction(d);
            if (isDataReady())
            {
                bool *frame = retrieveBitstream();
                rx_word[0] = deformat_bitstream(frame);
                UART_SendString(rx_word);
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
    //gpioWrite(PIN_TP3, HIGH);
    ADC_Start(ADC0, 1, ADC_mA); // starts ADC's conversion
    //gpioWrite(PIN_TP3, LOW);
}

static void dma_rx_major_cb(void *user)
{
    rx_ready = true;
}

static void NCO_ISRBit(void)
{
    //gpioWrite(PIN_TP4, HIGH);
    // Flag de reset de Contador de bits enviados (Redundancia para seguridad)
    if (initiate_send)
    {
        bit_cnt = 0;
        initiate_send = false;
    }

    // Decidir si envio en idle o si envio datos
    if (sending_data)
    {
        NCO_FskBit(&nco_handle, sending_bitstream[bit_cnt]);
    }
    else
    {

        NCO_FskBit(&nco_handle, idle_sending_bitstream[bit_cnt]);
        if (!last_byte_idle)
        {
            idle_counter++;
        }
    }

    bit_cnt++;
    // Se setearon bit_cnt bits en el NCO.
    if (bit_cnt == 11)
    {
        if (sending_data)
        {
            last_byte_idle = false;
        }
        sending_data = false;
        gpioWrite(PIN_LED_RED, !LED_ACTIVE);
        gpioWrite(PIN_TP1, LOW);
        bit_cnt = 0;
    }
    //gpioWrite(PIN_TP4, LOW);
}

static void NCO_ISRLut(void *user)
{
	//gpioWrite(PIN_TP2, HIGH);
    lut_value = NCO_TickQ15(&nco_handle);
    //gpioWrite(PIN_TP2, LOW);
}
