/***************************************************************************//**
  @file     App.c
  @brief    Test simple: enviar el mismo mensaje continuamente por UART0
*******************************************************************************/

#include <stdbool.h>
#include "drv/UART_strings.h"       // Tu driver UART0 (no bloqueante)
#include "drv/UART.h"   // Capa de abstracción por polling no bloqueante


/* Se llama 1 vez al inicio */
void App_Init (void)
{
	UART_Init();

}

char rx_line[128];

/* Se llama en un loop infinito */
void App_Run (void)
{
        UART_Poll();

		/* TX no bloqueante */
		if (UART_TxPending() == 0)
		{
			/* Intentar encolar (puede no entrar todo a la vez) */
			UART_SendString("A,0,R,-170\n");
			UART_SendString("A,0,C,-120\n");
			UART_SendString("A,0,O,-90\n");
			UART_SendString("A,0,R,-45\n");
			UART_SendString("A,0,C,0\n");
			UART_SendString("A,0,O,45\n");
			UART_SendString("A,0,R,60\n");
			UART_SendString("A,0,C,90\n");
			UART_SendString("A,0,O,150\n");
			UART_SendString("A,1,R,-170\n");
			UART_SendString("A,1,C,-120\n");
			UART_SendString("A,1,O,-90\n");


		}


		/* RX no bloqueante: copiar disponible hasta fin de línea o hasta llenar */
		int n = UART_ReceiveString(rx_line, sizeof(rx_line));
		if (n > 0)
		{
			UART_SendString("Recibido: ");
			UART_SendString(rx_line);
			UART_SendString("\r\n");
		}
}
