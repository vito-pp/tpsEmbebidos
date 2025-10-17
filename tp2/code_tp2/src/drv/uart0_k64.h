/*
 * uart0_k64.h
 *
 * Driver UART0 mínimo para MK64F12 (FRDM-K64F) usando la función de baudrate
 * de las diapositivas: __CORE_CLOCK__, SBR y BRFA, sin fsl_clock.h.
 *
 * - 8-N-1, TX/RX por polling.
 * - Pines fijos OpenSDA: PTB16 (RX) y PTB17 (TX) en ALT3.
 * - ISR de RX opcional (un byte + flag) si se define UART0K64_USE_ISR.
 */

#ifndef UART0_K64_H_
#define UART0_K64_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "MK64F12.h"

/* Baudrate por defecto usado por la función de las diapositivas si pasan 0
   o un valor inválido (>0x1FFF). */
#ifndef UART_HAL_DEFAULT_BAUDRATE
#define UART_HAL_DEFAULT_BAUDRATE   (9600u)
#endif

/* Pines de OpenSDA para UART0 */
#define UART0_TX_PORTB_PIN   (17u)   /* PTB17 -> UART0_TX (ALT3) */
#define UART0_RX_PORTB_PIN   (16u)   /* PTB16 -> UART0_RX (ALT3) */
#define ENABLE_PORT_CLOCKB() (SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK)

/* API pública (UART0) */
void     uart0_init(uint32_t baud);
void     uart0_set_baud(uint32_t baud);
void     uart0_enable(bool txEnable, bool rxEnable);
void     uart0_write_byte(uint8_t data);
uint8_t  uart0_read_byte(void);
void     uart0_write(const uint8_t *data, size_t len);
size_t   uart0_read(uint8_t *buf, size_t maxlen);
void     uart0_enable_rx_interrupt(bool enable);

/* ISR mínima opcional */
extern volatile bool     g_uart0_rx_flag;
extern volatile uint8_t  g_uart0_rx_data;

#endif /* UART0_K64_H_ */
