#ifndef CAN_DRV_H
#define CAN_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ==========================
 *  CONFIG / DEFINES ÚTILES
 * ========================== */

/* Bitrate por defecto para el TP: 125 kbit/s */
#ifndef CAN_DRV_DEFAULT_BITRATE
#define CAN_DRV_DEFAULT_BITRATE   125000u
#endif

/* IDs del TP: 0x100 .. 0x107 (ID base + #grupo) */
#define CAN_TP_ID_BASE            0x100u
#define CAN_TP_ID_MIN             0x100u
#define CAN_TP_ID_MAX             0x107u

/* Tamaños estándar del protocolo CAN */
#define CAN_MAX_DLEN              8u

/* Tipos de frame */
typedef enum {
    CAN_ID_STD = 0,   /* 11-bit */
    CAN_ID_EXT = 1    /* 29-bit */
} can_id_type_t;

/* Modos del controlador */
typedef enum {
    CAN_MODE_CONFIG = 0,
    CAN_MODE_NORMAL,
    CAN_MODE_SLEEP,
    CAN_MODE_LISTEN_ONLY,
    CAN_MODE_LOOPBACK
} can_mode_t;

/* Resultado genérico */
typedef enum {
    CAN_OK = 0,
    CAN_ERR_TIMEOUT,
    CAN_ERR_PARAM,
    CAN_ERR_HW,
    CAN_ERR_BUSY,
    CAN_ERR_NO_MSG
} can_status_t;

/* Estructura de un frame CAN */
typedef struct {
    uint32_t id;           /* ID (11 o 29 bits) */
    can_id_type_t ide;     /* STD/EXT */
    bool rtr;              /* Remote Transmission Request */
    uint8_t dlc;           /* 0..8 */
    uint8_t data[CAN_MAX_DLEN];
} can_frame_t;

/* Contadores de error (EML) */
typedef struct {
    uint8_t tx_err;     /* TEC */
    uint8_t rx_err;     /* REC */
    bool    error_passive;
    bool    bus_off;
} can_error_counters_t;

/* ==========================
 *  ABSTRACCIONES DE HW
 * ========================== */

/* Callbacks de SPI/GPIO proporcionados por la capa BSP/MCU */
typedef struct {
    /* SPI full-duplex transfer. Si rx==NULL, se descarta lectura. Si tx==NULL, se envían 0x00. */
    void (*spi_xfer)(const uint8_t *tx, uint8_t *rx, size_t len);

    /* Control de Chip Select (activo en bajo) */
    void (*cs_assert)(void);
    void (*cs_deassert)(void);

    /* Delays (al menos us para reset/OST). Si no se provee, se asume no-bloqueante. */
    void (*delay_us)(uint32_t us);

    /* GPIOs opcionales del MCP25625/PlacaCAN */
    void (*set_reset)(bool level);  /* RESET (activo en bajo). Puede ser NULL si no está cableado */
    void (*set_stby)(bool level);   /* STBY  (activo en alto).  Puede ser NULL si no está cableado */
    bool (*get_int)(void);          /* INT   (activo en bajo).  Puede ser NULL si no se usa */

    /* Relojes */
    uint32_t mcp_clk_hz;            /* Frecuencia del reloj del MCP25625 (p.ej. 16 MHz del cristal) */
} can_drv_io_t;

/* ==========================
 *  CONFIG INICIAL
 * ========================== */

typedef struct {
    can_drv_io_t io;                /* Callbacks de HW */
    uint32_t bitrate;               /* bps. Si 0 => CAN_DRV_DEFAULT_BITRATE */
    bool use_one_shot;              /* true: One-Shot mode (no reintenta TX) */
    bool enable_wakeup;             /* true: habilita WAKIE para salir de Sleep por bus */
    bool enable_clkout;             /* true: habilita CLKOUT si se quiere derivar reloj */
} can_drv_config_t;

/* ==========================
 *  API DE ALTO NIVEL
 * ========================== */

/* Inicializa el MCP25625 y lo deja en modo NORMAL listo para operar */
can_status_t CAN_DRV_Init(const can_drv_config_t *cfg);

/* Cambia de modo (bloqueante hasta que se confirme el modo) */
can_status_t CAN_DRV_SetMode(can_mode_t mode);

/* Reconfigura el bitrate en caliente (lleva a CONFIG, programa CNF1..3, vuelve al modo previo) */
can_status_t CAN_DRV_SetBitrate(uint32_t bitrate_bps);

/* Lee contadores de error/estado rápido */
can_status_t CAN_DRV_GetErrorCounters(can_error_counters_t *out);

/* Configura máscaras y filtros (std/ext). 
 *  - mask0/mask1: máscaras para RXB0/RXB1.
 *  - Cada filtro en filters[] se asigna secuencialmente a RXB0 (2) y RXB1 (4).
 */
typedef struct {
    uint32_t id;
    can_id_type_t ide;
} can_filter_t;

can_status_t CAN_DRV_SetMasksFilters(uint32_t mask0, can_id_type_t mask0_ide,
                                     uint32_t mask1, can_id_type_t mask1_ide,
                                     const can_filter_t *filters, size_t nfilters,
                                     bool enable_rxb0_rollover);

/* Helper: setea filtros para el TP (acepta std IDs 0x100..0x107) */
can_status_t CAN_DRV_ApplyTPAcceptanceWindow(void);

/* Transmite un frame. Usa el primer TX buffer libre. timeout_ms=0 => no bloquear. */
can_status_t CAN_DRV_Transmit(const can_frame_t *frame, uint32_t timeout_ms);

/* Intenta leer un frame de RXB0/RXB1 (polling). Devuelve CAN_ERR_NO_MSG si no hay. */
can_status_t CAN_DRV_Receive(can_frame_t *out_frame);

/* Limpia flags de interrupción relevantes y retorna si hay eventos pendientes (bitfield) */
typedef struct {
    bool rx0;   /* Mensaje en RXB0 */
    bool rx1;   /* Mensaje en RXB1 */
    bool tx0;   /* TXB0 completó */
    bool tx1;   /* TXB1 completó */
    bool tx2;   /* TXB2 completó */
    bool err;   /* Error general (MERRF) */
    bool wak;   /* Wake-up detectado */
} can_events_t;

can_status_t CAN_DRV_PollEvents(can_events_t *ev);

/* Helper del TP: empaqueta payload de ángulos ("R|C|O" + ASCII del valor, sin '\0') */
static inline uint8_t CAN_TP_EncodeAnglePayload(char angleId, const char *asciiVal,
                                                uint8_t *out /* >=5 bytes */)
{
    /* angleId (1) + hasta 4 ASCII digits/signos. No agrega terminador. */
    uint8_t i = 0;
    out[i++] = (uint8_t)angleId;
    for (; asciiVal && *asciiVal && i < 5; ++i) out[i] = (uint8_t)asciiVal[i-1];
    return i;
}

/* Helper del TP: bitfield de LED: "1JKL0RGB2" en un solo byte (J=centenas, K=decenas, L=unidades) */
static inline uint8_t CAN_TP_EncodeLedByte(uint8_t groupNumber, bool r, bool g, bool b)
{
    /* 1 J K L 0 R G B 2
       bits: [7]1, [6:4]=JKL, [3]=0, [2]=R, [1]=G, [0]=B  (se envía como un solo byte de datos) */
    uint8_t jkl = groupNumber & 0x07u;   /* 0..7 */
    return (uint8_t)( (1u<<7) | (jkl<<4) | (0u<<3) | ((r?1u:0u)<<2) | ((g?1u:0u)<<1) | (b?1u:0u) );
}

/* Resetea el dispositivo vía SPI (instrucción RESET) o pin RESET si está disponible */
can_status_t CAN_DRV_Reset(void);

/* Lee/Escribe registros crudos (debug) */
can_status_t CAN_DRV_ReadReg(uint8_t addr, uint8_t *val);
can_status_t CAN_DRV_WriteReg(uint8_t addr, uint8_t val);
can_status_t CAN_DRV_BitModify(uint8_t addr, uint8_t mask, uint8_t data);

#ifdef __cplusplus
}
#endif
#endif /* CAN_DRV_H */
