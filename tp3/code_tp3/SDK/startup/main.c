/*
 * main.c
 *
 * Simple UART test program
 *
 */

#include "hardware.h"
#include "../source/drv/mcal/UART.h"
#include "../source/drv/mcal/UART_strings.h"
#include "../source/bitstream.h"

#define PARITY UART_PARITY_ODD
#define __FOREVER__ for (;;)

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

int main(void)
{

	hw_Init();
	UART_Init(PARITY);

	char rx_line[64];

	uint8_t data = 0b01010101;

	// Enable interrupts
	hw_EnableInterrupts();

	__FOREVER__
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
			uint16_to_bin(frame, bits, sizeof(bits));
			UART_SendString("Dato Recibido: ");
			UART_SendString(rx_line);
			UART_SendString("\r\n");
			UART_SendString("Informacion enviada al NCO: ");
			UART_SendString(bits);
			UART_SendString("\r\n");
		}
	}
}
