/*
 * uart_nb_layer.c
 *
 * Implementación NO BLOQUEANTE por polling usando SOLO:
 *   - uart0_try_write_byte(uint8_t b)
 *   - uart0_try_read_byte(uint8_t *out)
 *
 * Autor: UART Driver Project
 */
#include "uart_nb_layer.h"
#include "uart0_k64.h"
#include <string.h>

/* ==== Ring buffer helpers (potencia de 2) ==== */
#define RB_MASK(sz)     ((sz) - 1u)

typedef struct {
    uint8_t *buf;
    uint16_t r, w;
    uint16_t mask;
} ring_t;

/* TX/RX storage (estáticos) */
static uint8_t g_tx_mem[UARTNB_TX_RING_SZ];
static uint8_t g_rx_mem[UARTNB_RX_RING_SZ];

_Static_assert((UARTNB_TX_RING_SZ & (UARTNB_TX_RING_SZ-1u)) == 0u, "UARTNB_TX_RING_SZ must be power of two");
_Static_assert((UARTNB_RX_RING_SZ & (UARTNB_RX_RING_SZ-1u)) == 0u, "UARTNB_RX_RING_SZ must be power of two");

static ring_t tx = { g_tx_mem, 0u, 0u, RB_MASK(UARTNB_TX_RING_SZ) };
static ring_t rx = { g_rx_mem, 0u, 0u, RB_MASK(UARTNB_RX_RING_SZ) };

static uartnb_stats_t stats = {0};

/* ==== Ring ops ==== */
static inline uint16_t rb_count(const ring_t *r){
    return (uint16_t)((r->w - r->r) & r->mask);
}
static inline uint16_t rb_space(const ring_t *r){
    return (uint16_t)(r->mask - rb_count(r));
}
static inline bool rb_push(ring_t *r, uint8_t b){
    uint16_t nxt = (uint16_t)((r->w + 1u) & r->mask);
    if (nxt == r->r) return false; /* full */
    r->buf[r->w] = b;
    r->w = nxt;
    return true;
}
static inline bool rb_pop(ring_t *r, uint8_t *b){
    if (r->r == r->w) return false;
    *b = r->buf[r->r];
    r->r = (uint16_t)((r->r + 1u) & r->mask);
    return true;
}

/* ==== API ==== */
void uartnb_init(void){
    tx.r = tx.w = 0u;
    rx.r = rx.w = 0u;
    memset(&stats, 0, sizeof(stats));
}

bool uartnb_send_bytes(const uint8_t *data, size_t len){
    size_t i = 0;
    for (; i < len; i++){
        if (!rb_push(&tx, data[i])) { stats.tx_overflow++; return false; }
    }
    stats.tx_enqueued += (uint32_t)i;
    return true;
}

bool uartnb_send_str(const char *str){
    return uartnb_send_bytes((const uint8_t*)str, strlen(str));
}

bool uartnb_send_line(const char *str, uint8_t delim){
    if (!uartnb_send_str(str)) return false;
    uint8_t d = delim;
    return uartnb_send_bytes(&d, 1u);
}

bool uartnb_tx_idle(void){
    return tx.r == tx.w;
}

size_t uartnb_rx_available(void){
    return rb_count(&rx);
}

size_t uartnb_rx_read(uint8_t *out, size_t maxlen){
    size_t n = 0;
    while ((n < maxlen) && rb_pop(&rx, &out[n])) {
        n++;
    }
    return n;
}

bool uartnb_rx_readline(uint8_t *out, size_t maxlen, uint8_t delim, size_t *out_len){
    /* Buscar 'delim' en el ring sin destruirlo; si está, copiar hasta él */
    uint16_t r = rx.r;
    if (r == rx.w) return false;

    size_t len = 0;
    while (r != rx.w){
        uint8_t b = rx.buf[r];
        r = (uint16_t)((r + 1u) & rx.mask);
        if (b == delim){
            /* Tenemos línea completa: copiar desde rx.r hasta b-exclusivo */
            if (len > maxlen) return false; /* out demasiado chico */
            for (size_t i=0;i<len;i++){
                out[i] = rx.buf[(rx.r + i) & rx.mask];
            }
            /* Consumir también el delimitador */
            rx.r = (uint16_t)((rx.r + (uint16_t)len + 1u) & rx.mask);
            if (out_len) *out_len = len;
            return true;
        }
        len++;
        if (len > rx.mask) break; /* protección */
    }
    return false;
}

void uartnb_poll(void){
    /* RX: drenar todo lo disponible SIN BLOQUEAR usando try_read_byte */
    uint8_t b;
    while (uart0_try_read_byte(&b)){
        if (!rb_push(&rx, b)) {
            /* RX overflow: descartar el byte más viejo (política simple) */
            (void)rb_pop(&rx, &b);
            (void)rb_push(&rx, b);
            stats.rx_overflow++;
        } else {
            stats.rx_received_bytes++;
        }
    }

    /* TX: emitir de a 1 byte si TDRE=1 (solo try_write_byte) */
    if (tx.r != tx.w){
        uint8_t outb;
        if (rb_pop(&tx, &outb)){
            if (!uart0_try_write_byte(outb)){
                /* No pudo salir: devolvemos el byte al frente para reintentar luego */
                tx.r = (uint16_t)((tx.r - 1u) & tx.mask);
            } else {
                stats.tx_sent_bytes++;
            }
        }
    }
}

const uartnb_stats_t* uartnb_get_stats(void){
    return &stats;
}
void uartnb_clear_stats(void){
    memset(&stats, 0, sizeof(stats));
}
