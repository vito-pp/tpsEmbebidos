#include "UART_strings.h"

/* -------- Buffers circulares -------- */
static volatile char s_tx_buf[UART_TX_BUF_SIZE];
static volatile size_t s_tx_head = 0;
static volatile size_t s_tx_tail = 0;

static volatile char s_rx_buf[UART_RX_BUF_SIZE];
static volatile size_t s_rx_head = 0;
static volatile size_t s_rx_tail = 0;

/* Helpers para ring buffer */
static inline size_t rb_next(size_t idx, size_t size)
{
    return (idx + 1u) % size;
}

static inline bool rb_full(size_t head, size_t tail, size_t size)
{
    return rb_next(head, size) == tail;
}

static inline bool rb_empty(size_t head, size_t tail)
{
    return head == tail;
}

size_t UART_TxPending(void)
{
    if (s_tx_head >= s_tx_tail)
        return s_tx_head - s_tx_tail;
    return (UART_TX_BUF_SIZE - s_tx_tail) + s_tx_head;
}

size_t UART_RxAvailable(void)
{
    if (s_rx_head >= s_rx_tail)
        return s_rx_head - s_rx_tail;
    return (UART_RX_BUF_SIZE - s_rx_tail) + s_rx_head;
}

void UART_Poll(void)
{
    /* RX: leer mientras haya datos */
    while (UART0->S1 & UART_S1_RDRF_MASK)
    {
        uint8_t s1 = UART0->S1;
        char c = (char)UART0->D; /* leer limpia los flags */

        /* si hubo paridad, framing, ruido u overrun, NO encolar el byte */
        if (s1 & (UART_S1_PF_MASK | UART_S1_FE_MASK |
                  UART_S1_NF_MASK | UART_S1_OR_MASK))
        {
            continue;
        }

        size_t next = rb_next(s_rx_head, UART_RX_BUF_SIZE);
        if (rb_full(s_rx_head, s_rx_tail, UART_RX_BUF_SIZE))
        {
            s_rx_tail = rb_next(s_rx_tail, UART_RX_BUF_SIZE);
        }
        s_rx_buf[s_rx_head] = c;
        s_rx_head = next;
    }

    /* TX: mientras el HW esté listo y haya datos */
    while ((UART0->S1 & UART_S1_TDRE_MASK) && !rb_empty(s_tx_head, s_tx_tail))
    {
        UART0->D = s_tx_buf[s_tx_tail];
        s_tx_tail = rb_next(s_tx_tail, UART_TX_BUF_SIZE);
    }
}

size_t UART_SendString(const char *str)
{
    if (!str)
        return 0;

    size_t enq = 0;
    while (*str)
    {
        size_t next = rb_next(s_tx_head, UART_TX_BUF_SIZE);
        if (next == s_tx_tail)
        {
            /* Buffer TX lleno: salir (no bloquear) */
            break;
        }
        s_tx_buf[s_tx_head] = *str++;
        s_tx_head = next;
        enq++;
    }

    /* Empujar inmediatamente lo que se pueda */
    UART_Poll();
    return enq;
}

int UART_ReceiveString(char *buffer, size_t max_len)
{
    if (!buffer || max_len == 0)
        return 0;

    size_t i = 0;
    bool endline = false;

    while ((i < (max_len - 1)) && !rb_empty(s_rx_head, s_rx_tail))
    {
        char c = s_rx_buf[s_rx_tail];
        s_rx_tail = rb_next(s_rx_tail, UART_RX_BUF_SIZE);

        if (c == '\n' || c == '\r')
        {
            endline = true;
            break; /* no incluir el fin de línea */
        }
        buffer[i++] = c;
    }

    buffer[i] = '\0';
    (void)endline; /* útil si más adelante querés diferenciar línea completa o parcial */
    return (int)i;
}
