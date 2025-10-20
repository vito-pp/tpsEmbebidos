/*
 * uart_nb_layer.h
 *
 * Capa de abstracción NO BLOQUEANTE por polling encima de uart0_k64.*
 * Usa EXCLUSIVAMENTE uart0_try_write_byte() y uart0_try_read_byte().
 *
 * Provee:
 *  - Cola TX: encola mensajes/bytes y los emite "byte a byte" cuando TDRE=1.
 *  - Cola RX: acumula bytes recibidos; utilidades para leer "líneas" con delimitador.
 *  - Servicio: uartnb_poll() a invocar frecuentemente en el loop principal.
 *
 * No usa interrupciones ni las funciones *_partial.
 *
 * Licencia: MIT
 */
#ifndef UART_NB_LAYER_H
#define UART_NB_LAYER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==== Configuración por defecto (pueden ajustarse) ==== */
#ifndef UARTNB_TX_RING_SZ
#define UARTNB_TX_RING_SZ   512u     /* debe ser potencia de 2 para índices rápidos */
#endif

#ifndef UARTNB_RX_RING_SZ
#define UARTNB_RX_RING_SZ   512u     /* idem */
#endif

/* ==== API ==== */

/* Inicializa la capa (limpia colas). No toca la UART; llamá antes a uart0_init(). */
void uartnb_init(void);

/* --- Envío (no bloqueante) --- */

/* Encola bytes para transmisión (copia a la cola). Devuelve false si no entra todo. */
bool uartnb_send_bytes(const uint8_t *data, size_t len);

/* Encola un string ASCII (sin incluir el '\0'). */
bool uartnb_send_str(const char *str);

/* Encola y agrega delimitador (por ej. '\n'). */
bool uartnb_send_line(const char *str, uint8_t delim);

/* Indica si no queda nada pendiente de transmitir. */
bool uartnb_tx_idle(void);

/* --- Recepción (no bloqueante) --- */

/* Devuelve cuántos bytes hay disponibles en la cola RX. */
size_t uartnb_rx_available(void);

/* Extrae hasta 'maxlen' bytes de la cola RX hacia 'out'. Devuelve bytes copiados. */
size_t uartnb_rx_read(uint8_t *out, size_t maxlen);

/* Lee una "línea" hasta encontrar 'delim'. Copia a 'out' (sin incluir 'delim').
 * Si hay línea completa, devuelve true y guarda su longitud en *out_len.
 * Si no hay línea completa, devuelve false y no copia nada.
 */
bool uartnb_rx_readline(uint8_t *out, size_t maxlen, uint8_t delim, size_t *out_len);

/* --- Servicio --- */

/* Debe llamarse frecuentemente (p. ej., en cada iteración del loop).
 * - Emite 0..N bytes (típicamente 1..hasta que TDRE se ocupe) pero SIEMPRE utilizando
 *   uart0_try_write_byte() "byte a byte".
 * - Drena RX llamando repetidamente a uart0_try_read_byte() mientras haya datos.
 */
void uartnb_poll(void);

/* Estadísticas simples (opcionales) */
typedef struct {
    uint32_t tx_enqueued;
    uint32_t tx_sent_bytes;
    uint32_t tx_overflow;
    uint32_t rx_received_bytes;
    uint32_t rx_overflow;
} uartnb_stats_t;

const uartnb_stats_t* uartnb_get_stats(void);
void uartnb_clear_stats(void);

#ifdef __cplusplus
}
#endif
#endif /* UART_NB_LAYER_H */
