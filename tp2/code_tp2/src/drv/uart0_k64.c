/*
 * uart0_k64.c
 *
 * Implementación UART0 mínima para FRDM-K64F, alineada al TP:
 * - Inicio de módulo, pines PB16/PB17 ALT3, cálculo de SBR/BRFA con sobremuestreo 16x.
 * - TX/RX por polling (TDRE/RDRF).
 * - ISR de RX opcional (un byte + flag).
 */

#include "uart0_k64.h"

/* =====================================
 *  Helpers locales
 * =====================================
 */

/* Determina la frecuencia fuente de UART0 leyendo SOPT2->UART0SRC
 * y usando los helpers del SDK para obtener la frecuencia en Hz. */
static uint32_t uart0_get_src_clock_hz(void)
{
    uint32_t srcSel = (SIM->SOPT2 & SIM_SOPT2_UART0SRC_MASK) >> SIM_SOPT2_UART0SRC_SHIFT;
    switch (srcSel)
    {
        case 0u: /* MCGFLLCLK o MCGPLLCLK/2, según PLLFLLSEL */
            return CLOCK_GetFreq(kCLOCK_PllFllSelClk);
        case 1u: /* OSCERCLK */
            return CLOCK_GetFreq(kCLOCK_Osc0ErClk);
        case 3u: /* MCGIRCLK */
            return CLOCK_GetFreq(kCLOCK_McgInternalRefClk);
        default: /* Valor reservado -> fallback razonable */
            return CLOCK_GetFreq(kCLOCK_PllFllSelClk);
    }
}

/* Programa SBR/BRFA para UART0 a partir de srcClockHz y baud.
 * Fórmula: baud = srcClock / (16 * (SBR + BRFA/32)) */
static void uart0_program_baud(uint32_t srcClockHz, uint32_t baud)
{
    uint16_t sbr = (uint16_t)(srcClockHz / (16u * baud));
    if (sbr == 0u) sbr = 1u;
    if (sbr > 0x1FFFu) sbr = 0x1FFFu; /* 13 bits */

    uint32_t brfa = ((2u * srcClockHz) / baud) - (32u * (uint32_t)sbr);
    if (brfa > 31u) brfa = 31u;

    /* Guardar C2 y deshabilitar TX/RX antes de tocar los divisores */
    uint8_t c2 = UART0->C2;
    UART0->C2 = 0u;

    UART0->BDH = (uint8_t)((UART0->BDH & ~UART_BDH_SBR_MASK) | UART_BDH_SBR((sbr >> 8) & 0x1Fu));
    UART0->BDL = (uint8_t)(sbr & 0xFFu);
    UART0->C4  = (uint8_t)((UART0->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa & 0x1Fu));

    UART0->C2 = c2; /* restaurar */
}

/* Configura los pines PB16 (RX) y PB17 (TX) en ALT3 (UART0). */
static void uart0_configure_pins(void)
{
    ENABLE_PORT_CLOCKB();
    PORTB->PCR[UART0_TX_PORTB_PIN] = PORT_PCR_MUX(3u);
    PORTB->PCR[UART0_RX_PORTB_PIN] = PORT_PCR_MUX(3u);
}

/* =====================================
 *  API pública
 * =====================================
 */

volatile bool    g_uart0_rx_flag = false;
volatile uint8_t g_uart0_rx_data = 0u;

void uart0_init(uint32_t baud)
{
    /* 1) Clock del módulo y pines */
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
    uart0_configure_pins();

    /* 2) Deshabilitar TX/RX y limpiar configuración básica (8-N-1) */
    UART0->C2 = 0u;
    UART0->C1 = 0u; /* M=0 (8 bits), PE=0 */
    UART0->S2 = 0u;
    UART0->C3 = 0u;

    /* 3) (Opcional) habilitar FIFOs */
    UART0->PFIFO = (uint8_t)(UART_PFIFO_TXFE_MASK | UART_PFIFO_RXFE_MASK);

    /* 4) Programar baudrate según la fuente efectiva */
    uint32_t srcClockHz = uart0_get_src_clock_hz();
    uart0_program_baud(srcClockHz, baud);

    /* 5) Habilitar TX y RX */
    UART0->C2 = (UART_C2_TE_MASK | UART_C2_RE_MASK);
}

void uart0_set_baud(uint32_t baud)
{
    uint32_t srcClockHz = uart0_get_src_clock_hz();
    uart0_program_baud(srcClockHz, baud);
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
