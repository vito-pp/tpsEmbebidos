/*
 * uart0_k64.h
 *
 * Driver UART0 mínimo para MK64F12 (FRDM-K64F) acorde a la consigna del TP:
 *  - Simplicidad: 8-N-1, baudrate por SBR/BRFA, TX/RX por polling.
 *  - Pines fijos de OpenSDA: PTB16 (RX) y PTB17 (TX) en ALT3.
 *  - Sin colas ni DMA. ISR de RX **opcional** (un byte + flag).
 *  - Integración directa con MCUXpresso SDK (usa fsl_clock.h para srcClock).
 *
 * Autor: ChatGPT (UART Driver)
 */

#ifndef UART0_K64_H_
#define UART0_K64_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "MK64F12.h"
#include "fsl_clock.h"   /* Para CLOCK_GetFreq(...) */

/* =============================
 *  Configuración de pines
 * =============================
 */
#define UART0_TX_PORTB_PIN   (17u)   /* PTB17 -> UART0_TX */
#define UART0_RX_PORTB_PIN   (16u)   /* PTB16 -> UART0_RX */

#define ENABLE_PORT_CLOCKB()   (SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK)

/* =============================
 *  API pública (UART0 solamente)
 * =============================
 */

/**
 * @brief Inicializa UART0 en 8-N-1 con el baudrate indicado.
 *
 * - Habilita clock de UART0 y PORTB.
 * - Configura PB16/PB17 en ALT3.
 * - Lee SIM->SOPT2[UART0SRC] para determinar la fuente de clock
 *   y obtiene la frecuencia con CLOCK_GetFreq(...).
 * - Programa SBR/BRFA y habilita TX/RX.
 *
 * @param baud   Baudrate deseado (ej.: 9600, 115200).
 */
void uart0_init(uint32_t baud);

/**
 * @brief Cambia el baudrate de UART0 (mantiene 8-N-1).
 */
void uart0_set_baud(uint32_t baud);

/**
 * @brief Habilita/Deshabilita TX y/o RX.
 */
void uart0_enable(bool txEnable, bool rxEnable);

/**
 * @brief Envía un byte (bloqueante).
 */
void uart0_write_byte(uint8_t data);

/**
 * @brief Recibe un byte (bloqueante).
 */
uint8_t uart0_read_byte(void);

/**
 * @brief Envía múltiples bytes (bloqueante).
 */
void uart0_write(const uint8_t *data, size_t len);

/**
 * @brief Recibe @p maxlen bytes (bloqueante).
 * @return la cantidad leída (=maxlen).
 */
size_t uart0_read(uint8_t *buf, size_t maxlen);

/**
 * @brief Habilita/Deshabilita interrupción de RX (RIE) de UART0.
 */
void uart0_enable_rx_interrupt(bool enable);

/* =============================
 *  ISR mínima (opcional)
 * =============================
 * Definir UART0K64_USE_ISR para compilar la ISR simple:
 *   - Al llegar un byte, lee S1 y luego D (limpia RDRF) y setea un flag + dato.
 */
extern volatile bool     g_uart0_rx_flag;
extern volatile uint8_t  g_uart0_rx_data;

#endif /* UART0_K64_H_ */
