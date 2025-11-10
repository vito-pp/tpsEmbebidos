/**
 * @file UART_strings.c
 * @brief Implementación del manejo abstracto de cadenas en UART con buffers circulares.
 *
 * Este archivo implementa buffers circulares para TX y RX, funciones de polling para
 * transmisión y recepción no bloqueante, y manejo de cadenas. Verifica errores en recepción
 * como paridad, framing, ruido u overrun.
 */

#include "UART_strings.h"

/* -------- Buffers circulares -------- */
/**
 * @var s_tx_buf
 * @brief Buffer circular para datos de transmisión (TX).
 */
static volatile char s_tx_buf[UART_TX_BUF_SIZE];

/**
 * @var s_tx_head
 * @brief Índice de cabeza para el buffer TX (donde se inserta).
 */
static volatile size_t s_tx_head = 0;

/**
 * @var s_tx_tail
 * @brief Índice de cola para el buffer TX (donde se extrae).
 */
static volatile size_t s_tx_tail = 0;

/**
 * @var s_rx_buf
 * @brief Buffer circular para datos de recepción (RX).
 */
static volatile char s_rx_buf[UART_RX_BUF_SIZE];

/**
 * @var s_rx_head
 * @brief Índice de cabeza para el buffer RX (donde se inserta).
 */
static volatile size_t s_rx_head = 0;

/**
 * @var s_rx_tail
 * @brief Índice de cola para el buffer RX (donde se extrae).
 */
static volatile size_t s_rx_tail = 0;

/* Helpers para ring buffer */
/**
 * @brief Calcula el siguiente índice en un buffer circular.
 *
 * @param idx Índice actual.
 * @param size Tamaño del buffer.
 * @return Siguiente índice.
 */
static inline size_t rb_next(size_t idx, size_t size)
{
    return (idx + 1u) % size;
}

/**
 * @brief Verifica si el buffer circular está lleno.
 *
 * @param head Índice de cabeza.
 * @param tail Índice de cola.
 * @param size Tamaño del buffer.
 * @return True si lleno, false caso contrario.
 */
static inline bool rb_full(size_t head, size_t tail, size_t size)
{
    return rb_next(head, size) == tail;
}

/**
 * @brief Verifica si el buffer circular está vacío.
 *
 * @param head Índice de cabeza.
 * @param tail Índice de cola.
 * @return True si vacío, false caso contrario.
 */
static inline bool rb_empty(size_t head, size_t tail)
{
    return head == tail;
}

/**
 * @brief Devuelve bytes pendientes de transmitir en el buffer TX.
 *
 * @return Cantidad de bytes pendientes.
 */
size_t UART_TxPending(void)
{
    if (s_tx_head >= s_tx_tail)
        return s_tx_head - s_tx_tail;
    return (UART_TX_BUF_SIZE - s_tx_tail) + s_tx_head;
}

/**
 * @brief Devuelve bytes disponibles para leer en el buffer RX.
 *
 * @return Cantidad de bytes disponibles.
 */
size_t UART_RxAvailable(void)
{
    if (s_rx_head >= s_rx_tail)
        return s_rx_head - s_rx_tail;
    return (UART_RX_BUF_SIZE - s_rx_tail) + s_rx_head;
}

/**
 * @brief Realiza polling no bloqueante del hardware UART.
 */
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

/**
 * @brief Encola una cadena terminada en null para transmisión no bloqueante.
 *
 * @param str Cadena a enviar.
 * @return Cantidad de bytes encolados.
 */
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

/**
 * @brief Copia de manera no bloqueante una "línea" desde el buffer RX.
 *
 * @param buffer Buffer destino.
 * @param max_len Tamaño máximo del buffer.
 * @return Cantidad de bytes copiados.
 */
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