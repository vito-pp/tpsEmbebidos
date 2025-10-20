/***************************************************************************//**
  @file     App.c
  @brief    Test simple: enviar el mismo mensaje continuamente por UART0
*******************************************************************************/

#include <stdbool.h>
#include "uart0_k64.h"       // Tu driver UART0 (no bloqueante)
#include "uart_nb_layer.h"   // Capa de abstracción por polling no bloqueante

#ifndef UART_TEST_BAUD
#define UART_TEST_BAUD   (115200u)
#endif

/* Mensaje a enviar continuamente (terminado con \n) */
static const char *kMsg = "HELLO UART (FRDM-K64F)\n";

/* Se llama 1 vez al inicio */
void App_Init (void)
{
    uart0_init(UART_TEST_BAUD);   // UART0 en PTB16/17 (VCOM OpenSDA), 115200 8N1
    uartnb_init();                // Inicializa la capa no bloqueante
}

/* Se llama en un loop infinito */
void App_Run (void)
{
    // Bombea UART: drena RX y envía 1 byte si TDRE=1. No bloquea.
    uartnb_poll();

    // Cuando TX queda libre, encolamos el mensaje otra vez
    if (uartnb_tx_idle()) {
        (void)uartnb_send_str(kMsg);  // Encola "HELLO UART...\n"
    }

    // Opcional: acá podés agregar otras tareas no bloqueantes
}
