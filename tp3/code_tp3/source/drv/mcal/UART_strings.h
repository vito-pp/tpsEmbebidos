#ifndef UART_ABSTRACT_H_
#define UART_ABSTRACT_H_

#include "UART.h"
#include <stddef.h>
#include <stdbool.h>
#include "MK64F12.h"

/* Ajustá los tamaños según tu caso de uso */
#ifndef UART_TX_BUF_SIZE
#define UART_TX_BUF_SIZE 2048
#endif

#ifndef UART_RX_BUF_SIZE
#define UART_RX_BUF_SIZE 2048
#endif

/**
 * @brief Realiza polling no bloqueante del hardware UART.
 *
 * Debe llamarse con frecuencia (por ejemplo en el lazo principal).
 * - Transmite bytes encolados (mientras TDRE esté en 1).
 * - Lee bytes disponibles (mientras RDRF esté en 1) y los encola en RX.
 */
void UART_Poll(void);

/**
 * @brief Encola una cadena terminada en null para transmisión no bloqueante.
 *
 * No espera al hardware. Encola hasta donde quepa en el buffer TX.
 * Llamar a UART_Poll() para ir sacando datos por el puerto.
 *
 * @param str Cadena null-terminated a enviar.
 * @return size_t Cantidad de bytes encolados (no incluye el '\0').
 */
size_t UART_SendString(const char *str);

/**
 * @brief Copia de manera no bloqueante una "línea" desde el buffer RX.
 *
 * Copia al buffer del usuario hasta encontrar '\n' o '\r' (no incluido),
 * o hasta completar max_len-1, o hasta que no haya más datos en RX.
 * Siempre agrega terminador '\0'.
 *
 * @param buffer Destino del string recibido.
 * @param max_len Tamaño del buffer destino (incluye '\0').
 * @return int Cantidad de bytes copiados (sin contar el '\0').
 */
int UART_ReceiveString(char *buffer, size_t max_len);

/**
 * @brief Devuelve bytes pendientes de transmitir en el buffer TX.
 */
size_t UART_TxPending(void);

/**
 * @brief Devuelve bytes disponibles para leer en el buffer RX.
 */
size_t UART_RxAvailable(void);

#endif /* UART_ABSTRACT_H_ */
