/***************************************************************************/ /**
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

void uint16_to_bin(uint16_t value, char *out, size_t out_len)
{
    if (!out || out_len == 0)
        return;

    const size_t total_bits = 16;
    size_t need = total_bits + 1; // +1 para '\0'

    if (out_len < need)
    {
        size_t bits = out_len - 1; // cuantos bits podemos escribir
        for (size_t i = 0; i < bits; ++i)
        {
            size_t shift = bits - 1 - i;
            out[i] = ((value >> shift) & 1U) ? '1' : '0';
        }
        out[bits] = '\0';
        return;
    }

    for (int i = 0; i < (int)total_bits; ++i)
    {
        int shift = (int)total_bits - 1 - i; // MSB first
        out[i] = ((value >> shift) & 1U) ? '1' : '0';
    }
    out[total_bits] = '\0';
}

/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/

static char rx_line[2048];

// Buffer circular para almacenar caracteres a transmitir
#define TX_BUFFER_SIZE 2048
static char tx_buffer[TX_BUFFER_SIZE];
static volatile size_t tx_head = 0; // Índice donde escribir próximo carácter
static volatile size_t tx_tail = 0; // Índice del próximo carácter a enviar

static NCO_Handle nco_handle;

// Bitstream de datos enviados por el DAC
static bool sending_bitstream[11];
static bool idle_sending_bitstream[11];
static bool idle_reciving_bitstream[11];

// Bitstream de datos recibidos por el ADC
static bool reciving_bitstream[11]; // 11?

static uint16_t lut_value;
static size_t cnt;
static volatile bool initiate_send = false;
static volatile bool sending_data = false;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR CALLBACKS
 ******************************************************************************/

static void NCO_ISRBit(void *user);
static void NCO_ISRLut(void *user);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
    gpioMode(PIN_TP1, OUTPUT);
    gpioWrite(PIN_TP1, LOW);

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
            .user = NULL};

    PIT_Config(&pit_cfg_lut);

    pit_cfg_t pit_cfg_bit =
        {
            .ch = 1,
            .load_val = PIT_TICKS_FROM_US(833),
            .periodic = true,
            .int_en = true,
            .dma_req = false,
            .callback = NCO_ISRBit,
            .user = NULL};

    PIT_Config(&pit_cfg_bit);

    NCO_InitFixed(&nco_handle, K_MARK, K_SPACE, true);

    // UART
    UART_Init(PARITY);

    // DAC
    DAC_Init();

    // Inicializo los bitstreams en idle
    for (int i = 0; i < 11; i++)
    {
        idle_sending_bitstream[i] = true;
        idle_reciving_bitstream[i] = 0;
    }
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{
    UART_Poll();

    /* TX no bloqueante */
    if (UART_TxPending() == 0)
    {
        /* Intentar encolar (puede no entrar todo a la vez) */
        // UART_SendString("Hola mundo!\r\n");
    }

    /* RX no bloqueante: copiar disponible hasta fin de línea o hasta llenar */
    int r = UART_ReceiveString(rx_line, sizeof(rx_line));

    if (r > 0)
    {
        // Encolar todos los caracteres recibidos en el buffer circular
        for (int i = 0; i < r; i++)
        {
            size_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;

            // En un buffer circular verdadero, solo nos preocupamos si el siguiente
            // espacio está ocupado por el tail
            if (next_head != tx_tail)
            {
                tx_buffer[tx_head] = rx_line[i];
                tx_head = next_head;
            }
            // Si el buffer está lleno, simplemente descartamos el carácter
        }

        // UART_SendString(rx_line); // Echo de lo recibido y guardado en el buffer circular
    }

    // Si no estamos enviando y hay datos en el buffer, iniciar nueva transmisión
    if (!sending_data && tx_head != tx_tail)
    {
        format_bitstream(tx_buffer[tx_tail], sending_bitstream);

        // Hacer un echo del caracter realmente enviado
        UART_SendString((char[]){tx_buffer[tx_tail], '\0'});

        tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE; // Avanzar al siguiente carácter
        initiate_send = true;
        sending_data = true;
        gpioWrite(PIN_LED_RED, LED_ACTIVE);
        gpioWrite(PIN_TP1, HIGH);
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

static void delayLoop(uint32_t veces)
{
    while (veces--)
        ;
}

static void __error_handler__(void)
{
    gpioWrite(PIN_LED_RED, LED_ACTIVE);
}
