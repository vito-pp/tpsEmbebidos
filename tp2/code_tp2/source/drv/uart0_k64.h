/*
 * uart0_k64.h (No-Blocking Polling)
 *
 * UART0 para MK64F12 (FRDM-K64F) con API no bloqueante por polling.
 * Mantiene wrappers BLOQUEANTES para compatibilidad (write_byte/read_byte/write/read).
 *
 * Pines: PTB16 (RX) / PTB17 (TX) en ALT3 (OpenSDA).
 * Formato: 8-N-1. Sin FSL clock manager.
 */
#ifndef UART0_K64_H_
#define UART0_K64_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "MK64F12.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UART_HAL_DEFAULT_BAUDRATE
#define UART_HAL_DEFAULT_BAUDRATE   (9600u)
#endif

/* Pines de OpenSDA para UART0 */
#define UART0_TX_PORTB_PIN   (17u)   /* PTB17 -> UART0_TX (ALT3) */
#define UART0_RX_PORTB_PIN   (16u)   /* PTB16 -> UART0_RX (ALT3) */
#define ENABLE_PORT_CLOCKB() (SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK)

/* ===== API ===== */
void     uart0_init(uint32_t baud);
void     uart0_set_baud(uint32_t baud);
void     uart0_enable(bool txEnable, bool rxEnable);

/* ---- NO BLOQUEANTES (polling) ---- */
bool     uart0_tx_ready(void);                   /* TDRE=1? */
bool     uart0_rx_ready(void);                   /* RDRF=1? */
bool     uart0_try_write_byte(uint8_t b);        /* escribe si TDRE=1 */
bool     uart0_try_read_byte(uint8_t *out);      /* lee si RDRF=1 */
size_t   uart0_write_partial(const uint8_t *buf, size_t len);   /* escribe hasta que TDRE=0 */
size_t   uart0_read_partial(uint8_t *buf, size_t maxlen);       /* lee mientras haya RDRF */

/* ---- WRAPPERS BLOQUEANTES (compatibilidad) ---- */
void     uart0_write_byte(uint8_t data);         /* espera TDRE */
uint8_t  uart0_read_byte(void);                  /* espera RDRF */
void     uart0_write(const uint8_t *data, size_t len);
size_t   uart0_read(uint8_t *buf, size_t maxlen);

/* ---- Interrupciones RX (opcional; no usadas en modo polling) ---- */
void     uart0_enable_rx_interrupt(bool enable);
extern volatile bool     g_uart0_rx_flag;
extern volatile uint8_t  g_uart0_rx_data;

/* ---- Errores de RX (diagnóstico opcional) ---- */
typedef struct {
    volatile uint32_t overrun;   /* OR */
    volatile uint32_t noise;     /* NF */
    volatile uint32_t framing;   /* FE */
    volatile uint32_t parity;    /* PF */
} uart0_error_counters_t;

const uart0_error_counters_t* uart0_get_error_counters(void);
void     uart0_clear_error_counters(void);

#ifdef __cplusplus
}
#endif
#endif /* UART0_K64_H_ */
