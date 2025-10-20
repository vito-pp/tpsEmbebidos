/*
 * uart0_k64.c (No-Blocking Polling)
 *
 * Implementación UART0 no bloqueante por polling para FRDM-K64F.
 * Conserva funciones bloqueantes como wrappers para compatibilidad.
 */
#include "uart0_k64.h"
#include "../../SDK/startup/hardware.h"

/* ======= Función pedida en diapositivas: cálculo de baudrate ======= */
static void UART_SetBaudRate(UART_Type *uart, uint32_t baudrate)
{
    uint16_t sbr, brfa;
    uint32_t clock;

    clock = ((uart == UART0) || (uart == UART1)) ? (__CORE_CLOCK__) : (__CORE_CLOCK__ >> 1);

    baudrate = ((baudrate == 0u) ? (UART_HAL_DEFAULT_BAUDRATE) :
                ((baudrate > 0x1FFFu) ? (UART_HAL_DEFAULT_BAUDRATE) : (baudrate)));

    sbr  = (uint16_t)(clock / (baudrate << 4));                // sbr = clock/(baud*16)
    brfa = (uint16_t)(((clock << 1) / baudrate) - (sbr << 5)); // brfa = 2*clock/baud - 32*sbr

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

/* ======= Estado de errores ======= */
static uart0_error_counters_t g_errs = {0,0,0,0};
const uart0_error_counters_t* uart0_get_error_counters(void){ return &g_errs; }
void uart0_clear_error_counters(void){ g_errs.overrun=g_errs.noise=g_errs.framing=g_errs.parity=0u; }

/* ======= API pública ======= */
volatile bool    g_uart0_rx_flag = false;
volatile uint8_t g_uart0_rx_data = 0u;

void uart0_init(uint32_t baud)
{
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
    uart0_configure_pins();

    UART0->C2 = 0u;
    UART0->C1 = 0u; /* 8 bits, sin paridad */
    UART0->S2 = 0u;
    UART0->C3 = 0u;

    /* (Opcional) habilitar FIFOs si están disponibles */
    UART0->PFIFO = (uint8_t)(UART_PFIFO_TXFE_MASK | UART_PFIFO_RXFE_MASK);

    UART_SetBaudRate(UART0, baud);

    UART0->C2 = (UART_C2_TE_MASK | UART_C2_RE_MASK);
}

void uart0_set_baud(uint32_t baud)
{
    uint8_t c2 = UART0->C2;
    UART0->C2 = 0u;
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

/* ======= Helpers de flags ======= */
static inline void uart0_handle_rx_errors(uint8_t s1_snapshot){
    if (s1_snapshot & UART_S1_OR_MASK) g_errs.overrun++;
    if (s1_snapshot & UART_S1_NF_MASK) g_errs.noise++;
    if (s1_snapshot & UART_S1_FE_MASK) g_errs.framing++;
    if (s1_snapshot & UART_S1_PF_MASK) g_errs.parity++;
}

bool uart0_tx_ready(void){
    return (UART0->S1 & UART_S1_TDRE_MASK) != 0u;
}

bool uart0_rx_ready(void){
    return (UART0->S1 & UART_S1_RDRF_MASK) != 0u;
}

/* ======= NO BLOQUEANTES ======= */
bool uart0_try_write_byte(uint8_t b){
    if (!uart0_tx_ready()) return false;
    UART0->D = b; /* escribir D limpia TDRE */
    return true;
}

bool uart0_try_read_byte(uint8_t *out){
    uint8_t s1 = UART0->S1;
    if ((s1 & UART_S1_RDRF_MASK) == 0u) return false;
    uart0_handle_rx_errors(s1);
    *out = UART0->D; /* leer D limpia RDRF y errores latched */
    return true;
}

size_t uart0_write_partial(const uint8_t *buf, size_t len){
    size_t i = 0;
    while (i < len){
        if (!uart0_tx_ready()) break;
        UART0->D = buf[i++];
    }
    return i;
}

size_t uart0_read_partial(uint8_t *buf, size_t maxlen){
    size_t n = 0;
    while (n < maxlen){
        uint8_t s1 = UART0->S1;
        if ((s1 & UART_S1_RDRF_MASK) == 0u) break;
        uart0_handle_rx_errors(s1);
        buf[n++] = UART0->D;
    }
    return n;
}

/* ======= WRAPPERS BLOQUEANTES (compat) ======= */
void uart0_write_byte(uint8_t data){
    while (!uart0_tx_ready()) { /* esperar */ }
    UART0->D = data;
}

uint8_t uart0_read_byte(void){
    uint8_t s1;
    do { s1 = UART0->S1; } while ((s1 & UART_S1_RDRF_MASK) == 0u);
    uart0_handle_rx_errors(s1);
    return UART0->D;
}

void uart0_write(const uint8_t *data, size_t len){
    size_t i = 0;
    while (i < len){
        i += uart0_write_partial(&data[i], len - i);
    }
}

size_t uart0_read(uint8_t *buf, size_t maxlen){
    size_t n = 0;
    while (n < maxlen){
        n += uart0_read_partial(&buf[n], maxlen - n);
    }
    return n;
}

/* ======= RX IRQ opcional (no usada en polling) ======= */
void uart0_enable_rx_interrupt(bool enable){
    if (enable) UART0->C2 |= UART_C2_RIE_MASK;
    else        UART0->C2 &= (uint8_t)~UART_C2_RIE_MASK;
}

#ifdef UART0K64_USE_ISR
void UART0_RX_TX_IRQHandler(void){
    uint8_t s1;
    while ( (s1 = UART0->S1), (s1 & UART_S1_RDRF_MASK) ){
        uart0_handle_rx_errors(s1);
        g_uart0_rx_data = UART0->D;
        g_uart0_rx_flag = true;
    }
}
#endif
