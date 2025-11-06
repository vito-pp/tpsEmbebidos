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

#define TX 1
#define RX 0 
#define MODEM_MODE RX

/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/

#if MODEM_MODE == RX
#define RX_BUF_LEN 128
static uint16_t rx_buffer[RX_BUF_LEN] __attribute__((aligned(4)));
static uint16_t rx_idx;
static volatile bool rx_ready = false;
static char rx_word[2048];
#else
static char rx_line[2048];

static NCO_Handle nco_handle;

// Bitstream de datos enviados por el DAC
static bool sending_bitstream[11]; // 11?

// Bitstream de datos recibidos por el ADC
static bool reciving_bitstream[11]; // 11?

static uint16_t lut_value;
static size_t cnt;
static bool initiate_send = false;
static bool sending_data = false;
#endif

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
static void __error_handler__(void);

void uint16_to_bin(uint16_t value, char *out, size_t out_len);

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR CALLBACKS
 ******************************************************************************/

 #if MODEM_MODE == RX
static void dma_rx_major_cb(void *user);
#endif 

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR CALLBACKS
 ******************************************************************************/

static void NCO_ISRBit(void *user);
static void NCO_ISRLut(void* user);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    // For the error handler
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
    
    // Common inits
    UART_Init(UART_PARITY_ODD); 
    PIT_Init();
    DMA_Init();

#if MODEM_MODE == RX 
    ADC_Init(false); // true = dma_req enable
    // DMA init
    // dma_cfg_t dma_cfg =
    // {
    //     .ch = 0,
    //     .request_src = DMA_REQ_ALWAYS63,
    //     .trig_mode = true,
    //     .saddr = (void *)&ADC0->R[0],
    //     .daddr = rx_buffer,
    //     .nbytes = 2, // 16-bit
    //     .soff = 0, .doff = 2,
    //     .major_count = RX_BUF_LEN,
    //     .slast = 0,
    //     .dlast = -(RX_BUF_LEN * 2),
    //     .int_major = true,
    //     .on_major = dma_rx_major_cb,
    //     .user = NULL
    // };
    // DMA_Config(&dma_cfg);
    // DMA_Start(0);

    // PIT configs
    pit_cfg_t pit_cfg =
    {
        .ch = PIT_CH0,
        .load_val = PIT_TICKS_FROM_US(83), // 12kHz ADC sampling
        .periodic = true,
        .int_en = true,
        .dma_req = false, // PIT asserts DMA request
        .callback = dma_rx_major_cb,
        .user = NULL
    };
    PIT_Config(&pit_cfg);

    // FPU enable
    SCB->CPACR |= (0xF << 20);

#else // TX MODE

    // PIT configs
    pit_cfg_t pit_cfg_lut =
    {
        .ch = 0,
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
        .ch = 1,
        .load_val = PIT_TICKS_FROM_US(833),
        .periodic = true,
        .int_en = true,
        .dma_req = false,
        .callback = NCO_ISRBit,
        .user = NULL
    };
    PIT_Config(&pit_cfg_bit);

    NCO_InitFixed(&nco_handle, K_MARK, K_SPACE, true);
    DAC_Init();
#endif
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    //UART_Poll();
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
                rx_word[0] = deformat_bitstream(frame);
                UART_SendString(rx_word);
            }
        }
    }
#else // TX MODE
    /* TX no bloqueante */
    if (UART_TxPending() == 0)
    {
        /* Intentar encolar (puede no entrar todo a la vez) */
        // UART_SendString("Hola mundo!\r\n");
    }

    /* RX no bloqueante: copiar disponible hasta fin de línea o hasta llenar */
    int n = UART_ReceiveString(rx_line, sizeof(rx_line));

    if (n > 0)
    {
        char bits[17];
        uint16_t frame = data_to_uart(rx_line[0]);
        uint16_to_bin(frame, bits, sizeof(bits));
        UART_SendString("Dato Recibido: ");
        UART_SendString(rx_line);
        UART_SendString("\r\n");
        UART_SendString("Informacion enviada al NCO: ");
        UART_SendString(bits);
        UART_SendString("\r\n");
        cnt = 0;
        format_bitstream(rx_line[0], sending_bitstream);
        initiate_send = true;
        sending_data = true;
    }

    else // no data recieved
    {
    	if (!sending_data){
            for (int i = 0; i < 11; i++)
            {
                sending_bitstream[i] = true;
            }
    	}
    }

    uint8_t received_data = 0; // placeholder
    recived_data = deformat_bitstream(reciving_bitstream);

    if (received_data != 0){
        char bits[17];
        uint16_t frame = data_to_uart(received_data);
        uint16_to_bin(frame, bits, sizeof(bits));
        UART_SendString("Dato Recibido del ADC: ");
        UART_SendString(received_data);
        UART_SendString("\r\n");
        UART_SendString("Informacion recibida en bits: ");
        UART_SendString(bits);
        UART_SendString("\r\n");
    }
#endif
}

/*******************************************************************************
 *******************************************************************************
    LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
#if MODEM_MODE == TX
static void NCO_ISRBit(void *user)
{
    if (initiate_send){
    	cnt = 0;
		initiate_send = false;
	}

    NCO_FskBit(&nco_handle, sending_bitstream[cnt]);
    cnt++;
    if(cnt == 11)
    {
    	sending_data = false;
        cnt = 0;
    }
}

static void NCO_ISRLut(void* user)
{

    lut_value = NCO_TickQ15(&nco_handle);
    DAC_SetData(DAC0, lut_value);
}

void uint16_to_bin(uint16_t value, char *out, size_t out_len){
    if (!out || out_len == 0)
        return;

    const size_t total_bits = 16;
    size_t need = total_bits + 1; // +1 para '\0'

    if (out_len < need) {
        size_t bits = out_len - 1; // cuantos bits podemos escribir
        for (size_t i = 0; i < bits; ++i) {
            size_t shift = bits - 1 - i;
            out[i] = ((value >> shift) & 1U) ? '1' : '0';
        }
        out[bits] = '\0';
        return;
    }

    for (int i = 0; i < (int)total_bits; ++i) {
        int shift = (int)total_bits - 1 - i; // MSB first
        out[i] = ((value >> shift) & 1U) ? '1' : '0';
    }
    out[total_bits] = '\0';
}
#else
static void dma_rx_major_cb(void *user)
{
    (void)user;
    static int cnt = 1;

    ADC_Start(ADC0, 1, ADC_mA);
    while (!ADC_IsReady(ADC0));
	rx_buffer[rx_idx] = ADC_getData(ADC0);
	rx_idx = (rx_idx + 1) % RX_BUF_LEN;

    if (cnt == RX_BUF_LEN)
    {
        rx_ready = true;
        cnt = 0;
    }
    cnt++;
}
#endif

static void delayLoop(uint32_t veces)
{
    while (veces--);
}

static void __error_handler__(void)
{
    gpioWrite(PIN_LED_RED, LED_ACTIVE);
}
