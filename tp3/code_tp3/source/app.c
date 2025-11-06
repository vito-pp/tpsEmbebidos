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

#include "drv/hal/timer.h"
#include "drv/hal/NCO.h"
#include "drv/mcal/board.h"
#include "drv/mcal/gpio.h"
#include "drv/mcal/SysTick.h"
#include "drv/mcal/UART_strings.h"
#include "drv/mcal/UART.h"
#include "drv/mcal/dma.h"
#include "drv/mcal/UART.h"
#include "drv/mcal/UART_strings.h"
#include "drv/mcal/pit.h"
#include "drv/mcal/DAC.h"
#include "bitstream.h"

#define PARITY UART_PARITY_ODD

void uint16_to_bin(uint16_t value, char *out, size_t out_len);

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

/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/

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


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);

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
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);

    // PIT init
    PIT_Init();

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
    
    // UART 
    UART_Init(PARITY);

    // DAC
    DAC_Init();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    UART_Poll();

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
        // uint16_to_bin(frame, bits, sizeof(bits));
        // UART_SendString("Dato Recibido: ");
        UART_SendString(rx_line);
        // UART_SendString("\r\n");
        // UART_SendString("Informacion enviada al NCO: ");
        // UART_SendString(bits);
        // UART_SendString("\r\n");
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
                reciving_bitstream[i] = 0;
            }
    	}
    }


    // char received_data[1]; // placeholder
    // received_data[0] = deformat_bitstream(reciving_bitstream);

    // if (received_data != 0){
    //     char bits[17];
    //     uint16_t frame = data_to_uart(received_data[0]);
    //     uint16_to_bin(frame, bits, sizeof(bits));
    //     UART_SendString("Dato Recibido del ADC: ");
    //     UART_SendString(received_data);
    //     UART_SendString("\r\n");
    //     UART_SendString("Informacion recibida en bits: ");
    //     UART_SendString(bits);
    //     UART_SendString("\r\n");
    // }

}

/*******************************************************************************
 *******************************************************************************
    LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

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

static void delayLoop(uint32_t veces)
{
    while (veces--);
}

static void __error_handler__(void)
{
    gpioWrite(PIN_LED_RED, LED_ACTIVE);
}
