/*
 * uart0_k64.c
 *
 * Implementación UART0 mínima para FRDM-K64F, sin fsl_clock.h:
 * - Habilita SCGC4_UART0 y PORTB
 * - PB16/PB17 -> ALT3
 * - Cálculo SBR/BRFA con sobremuestreo 16x
 * - TX/RX por polling (TDRE/RDRF)
 * - ISR mínima opcional (un byte + flag)
 */

#include "uart0_k64.h"

/* ===== Helpers locales ===== */

static inline uint32_t uart0_get_src_clock_hz(void)
{
    /* Sin SDK: el usuario define UART0_SRC_CLOCK_HZ según su configuración. */
    return (uint32_t)UART0_SRC_CLOCK_HZ;
}

/* baud = srcClock / (16 * (SBR + BRFA/32)) */
static void uart0_program_baud(uint32_t srcClockHz, uint32_t baud)
{
    uint16_t sbr = (uint16_t)(srcClockHz / (16u * baud));
    if (sbr == 0u)     sbr = 1u;
    if (sbr > 0x1FFFu) sbr = 0x1FFFu; /* 13 bits */

    uint32_t brfa = ((2u * srcClockHz) / baud) - (32u * (uint32_t)sbr);
    if (brfa > 31u) brfa = 31u;

    /* Guardar C2 y deshabilitar TX/RX antes de tocar divisores */
    uint8_t c2 = UART0->C2;
    UART0->C2 = 0u;

    UART0->BDH = (uint8_t)((UART0->BDH & ~UART_BDH_SBR_MASK) | UART_BDH_SBR((sbr >> 8) & 0x1Fu));
    UART0->BDL = (uint8_t)(sbr & 0xFFu);
    UART0->C4  = (uint8_t)((UART0->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa & 0x1Fu));

    UART0->C2 = c2; /* restaurar */
}

static void uart0_configure_pins(void)
{
    ENABLE_PORT_CLOCKB();
    PORTB->PCR[UART0_TX_PORTB_PIN] = PORT_PCR_MUX(3u); /* ALT3 */
    PORTB->PCR[UART0_RX_PORTB_PIN] = PORT_PCR_MUX(3u); /* ALT3 */
}

/* ===== API pública ===== */

volatile bool    g_uart0_rx_flag = false;
volatile uint8_t g_uart0_rx_data = 0u;

void uart0_init(uint32_t baud)
{
    /* 1) Clock del módulo y pines */
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
    uart0_configure_pins();

    /* 2) Deshabilitar TX/RX y 8-N-1 */
    UART0->C2 = 0u;
    UART0->C1 = 0u; /* M=0 (8 bits), PE=0 */
    UART0->S2 = 0u;
    UART0->C3 = 0u;

    /* 3) (Opcional) FIFOs */
    UART0->PFIFO = (uint8_t)(UART_PFIFO_TXFE_MASK | UART_PFIFO_RXFE_MASK);

    /* 4) Baudrate */
    uint32_t srcClockHz = uart0_get_src_clock_hz();
    uart0_program_baud(srcClockHz, baud);

    /* 5) Habilitar TX y RX */
    UART0->C2 = (UART_C2_TE_MASK | UART_C2_RE_MASK);
}

void uart0_set_baud(uint32_t baud)
{
    uart0_program_baud(uart0_get_src_clock_hz(), baud);
}

void uart0_enable(bool txEnable, bool rxEnable)
{
    uint8_t c2 = UART0->C2;
    if (txEnable) c2 |= UART_C2_TE_MASK; else c2 &= (uint8_t)~UART_C2_TE_MASK;
    if (rxEnable) c2 |= UART_C2_RE_MASK; else c2 &= (uint8_t)~UART_C2_RE_MASK;
    UART0->C2 = c2;
}

void uart0_write_byte(uint8_t data)
{
    while ((UART0->S1 & UART_S1_TDRE_MASK) == 0u) { /* esperar */ }
    UART0->D = data;
}

uint8_t uart0_read_byte(void)
{
    while ((UART0->S1 & UART_S1_RDRF_MASK) == 0u) { /* esperar */ }
    return UART0->D;
}

void uart0_write(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; ++i) uart0_write_byte(data[i]);
}

size_t uart0_read(uint8_t *buf, size_t maxlen)
{
    for (size_t i = 0; i < maxlen; ++i) buf[i] = uart0_read_byte();
    return maxlen;
}

void uart0_enable_rx_interrupt(bool enable)
{
    if (enable) UART0->C2 |= UART_C2_RIE_MASK;
    else        UART0->C2 &= (uint8_t)~UART_C2_RIE_MASK;
}

#ifdef UART0K64_USE_ISR
void UART0_RX_TX_IRQHandler(void)
{
    /* Orden correcto: leer S1 y luego D si hay dato (RDRF=1) */
    uint8_t s1 = UART0->S1;
    (void)s1;
    if (s1 & UART_S1_RDRF_MASK)
    {
        g_uart0_rx_data = UART0->D;  /* leer D limpia RDRF */
        g_uart0_rx_flag = true;
    }
}
#endif

