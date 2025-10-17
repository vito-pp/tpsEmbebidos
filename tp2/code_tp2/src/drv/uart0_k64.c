/*
 * uart0_k64.c
 *
 * Implementación UART0 mínima para FRDM-K64F.
 * Usa la función de las diapositivas para calcular el baudrate:
 *
 *   clock = ((uart == UART0) || (uart == UART1)) ? (__CORE_CLOCK__) : (__CORE_CLOCK__ >> 1);
 *   sbr   = clock / (baud << 4);                 // sbr = clock / (baud * 16)
 *   brfa  = (clock << 1) / baud - (sbr << 5);    // brfa = 2*clock/baud - 32*sbr
 *
 * y luego escribe BDH/BDL/C4.
 */

#include "uart0_k64.h"

/* ======= Función pedida en diapositivas: cálculo de baudrate ======= */
static void UART_SetBaudRate(UART_Type *uart, uint32_t baudrate)
{
    uint16_t sbr, brfa;
    uint32_t clock;

    /* UART0/UART1 usan core clock; el resto, típicamente bus clock (= core/2) */
    clock = ((uart == UART0) || (uart == UART1)) ? (__CORE_CLOCK__) : (__CORE_CLOCK__ >> 1);

    /* Valor por defecto si baud inválido */
    baudrate = ((baudrate == 0u) ? (UART_HAL_DEFAULT_BAUDRATE) :
                ((baudrate > 0x1FFFu) ? (UART_HAL_DEFAULT_BAUDRATE) : (baudrate)));

    /* Cálculos según slides */
    sbr  = (uint16_t)(clock / (baudrate << 4));              // sbr = clock/(baud*16)
    brfa = (uint16_t)(((clock << 1) / baudrate) - (sbr << 5)); // brfa = 2*clock/baud - 32*sbr

    /* Programar SBR (BDH/BDL) y BRFA (C4) */
    uart->BDH = UART_BDH_SBR((uint8_t)(sbr >> 8));
    uart->BDL = UART_BDL_SBR((uint8_t)(sbr & 0xFFu));
    uart->C4  = (uint8_t)((uart->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa & 0x1Fu));
}

/* ======= Configuración de pines PTB16/PTB17 en ALT3 ======= */
static void uart0_configure_pins(void)
{
    ENABLE_PORT_CLOCKB();
    PORTB->PCR[UART0_TX_PORTB_PIN] = PORT_PCR_MUX(3u); /* ALT3 */
    PORTB->PCR[UART0_RX_PORTB_PIN] = PORT_PCR_MUX(3u); /* ALT3 */
}

/* ======= API pública ======= */
volatile bool    g_uart0_rx_flag = false;
volatile uint8_t g_uart0_rx_data = 0u;

void uart0_init(uint32_t baud)
{
    /* 1) Clock módulo UART0 + pines */
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
    uart0_configure_pins();

    /* 2) Deshabilitar TX/RX y setear 8-N-1 */
    UART0->C2 = 0u;
    UART0->C1 = 0u; /* M=0 (8 bits), PE=0 */
    UART0->S2 = 0u;
    UART0->C3 = 0u;

    /* 3) (Opcional) habilitar FIFOs */
    UART0->PFIFO = (uint8_t)(UART_PFIFO_TXFE_MASK | UART_PFIFO_RXFE_MASK);

    /* 4) Baudrate con la función de las diapositivas */
    UART_SetBaudRate(UART0, baud);

    /* 5) Habilitar TX y RX */
    UART0->C2 = (UART_C2_TE_MASK | UART_C2_RE_MASK);
}

void uart0_set_baud(uint32_t baud)
{
    uint8_t c2 = UART0->C2;
    UART0->C2 = 0u;                 /* seguro para modificar divisores */
    UART_SetBaudRate(UART0, baud);
    UART0->C2 = c2;
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
    /* Orden correcto: leer S1 y luego D si hay RDRF */
    uint8_t s1 = UART0->S1; (void)s1;
    if (s1 & UART_S1_RDRF_MASK) {
        g_uart0_rx_data = UART0->D;
        g_uart0_rx_flag = true;
    }
}
#endif
