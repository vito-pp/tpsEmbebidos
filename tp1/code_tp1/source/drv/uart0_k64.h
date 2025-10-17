/*
 * uart0_k64.h
 *
 * Driver UART0 mínimo para MK64F12 (FRDM-K64F), sin dependencias del SDK.
 * - 8-N-1, baudrate por SBR/BRFA, TX/RX por polling.
 * - Pines fijos: PTB16 (RX) y PTB17 (TX) en ALT3 (OpenSDA).
 * - ISR de RX opcional (un byte + flag).
 *
 * IMPORTANTE: Definí UART0_SRC_CLOCK_HZ con la frecuencia real del clock
 * que alimenta a UART0 (según SIM->SOPT2[UART0SRC]).
 */

#ifndef UART0_K64_H_
#define UART0_K64_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "MK64F12.h"

/* ===== Clock fuente de UART0 =====
 * Valor típico en FRDM-K64F: PLL=120MHz -> PLL/2=60MHz
 * Cambiá este valor si tu proyecto usa otra fuente o frecuencia.
 */
#ifndef UART0_SRC_CLOCK_HZ
#define UART0_SRC_CLOCK_HZ   (60000000u)
#endif

/* ===== Pines de OpenSDA ===== */
#define UART0_TX_PORTB_PIN   (17u)   /* PTB17 -> UART0_TX */
#define UART0_RX_PORTB_PIN   (16u)   /* PTB16 -> UART0_RX */
#define ENABLE_PORT_CLOCKB() (SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK)

/* ===== API pública (UART0) ===== */
void     uart0_init(uint32_t baud);
void     uart0_set_baud(uint32_t baud);
void     uart0_enable(bool txEnable, bool rxEnable);
void     uart0_write_byte(uint8_t data);
uint8_t  uart0_read_byte(void);
void     uart0_write(const uint8_t *data, size_t len);
size_t   uart0_read(uint8_t *buf, size_t maxlen);
void     uart0_enable_rx_interrupt(bool enable);

/* ISR mínima (opcional: definir UART0K64_USE_ISR) */
extern volatile bool     g_uart0_rx_flag;
extern volatile uint8_t  g_uart0_rx_data;

#endif /* UART0_K64_H_ */
