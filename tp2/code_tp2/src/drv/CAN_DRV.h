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
#ifndef CAN_DRV_DEFAULT_BITRATE
#define CAN_DRV_DEFAULT_BITRATE   125000u
#endif

#define CAN_TP_ID_BASE            0x100u
#define CAN_TP_ID_MIN             0x100u
#define CAN_TP_ID_MAX             0x107u

#define CAN_MAX_DLEN              8u

typedef enum {
    CAN_ID_STD = 0,   /* 11-bit */
    CAN_ID_EXT = 1    /* 29-bit */
} can_id_type_t;

typedef enum {
    CAN_MODE_CONFIG = 0,
    CAN_MODE_NORMAL,
    CAN_MODE_SLEEP,
    CAN_MODE_LISTEN_ONLY,
    CAN_MODE_LOOPBACK
} can_mode_t;

typedef enum {
    CAN_OK = 0,
    CAN_ERR_TIMEOUT,
    CAN_ERR_PARAM,
    CAN_ERR_HW,
    CAN_ERR_BUSY,
    CAN_ERR_NO_MSG
} can_status_t;

typedef struct {
    uint32_t id;
    can_id_type_t ide;
    bool rtr;
    uint8_t dlc;
    uint8_t data[CAN_MAX_DLEN];
} can_frame_t;

typedef struct {
    uint8_t tx_err;
    uint8_t rx_err;
    bool    error_passive;
    bool    bus_off;
} can_error_counters_t;

/* ==========================
 *  ABSTRACCIONES DE HW
 * ========================== */
typedef struct {
    /* Delays opcionales para timeouts u OST */
    void (*delay_us)(uint32_t us);

    /* GPIOs opcionales del MCP25625/PlacaCAN */
    void (*set_reset)(bool level);  /* RESET (activo en bajo) */
    void (*set_stby)(bool level);   /* STBY  (activo en alto) */
    bool (*get_int)(void);          /* INT   (activo en bajo) */

    /* Reloj del MCP (usado para CNFx si querés otros bitrates) */
    uint32_t mcp_clk_hz;            /* típico 16 MHz */
} can_drv_io_t;

typedef struct {
    can_drv_io_t io;
    uint32_t bitrate;             /* Si 0 => CAN_DRV_DEFAULT_BITRATE */
    bool use_one_shot;
    bool enable_wakeup;
    bool enable_clkout;
} can_drv_config_t;

/* ==========================
 *  API DE ALTO NIVEL
 * ========================== */
can_status_t CAN_DRV_Init(const can_drv_config_t *cfg);
can_status_t CAN_DRV_SetMode(can_mode_t mode);
can_status_t CAN_DRV_SetBitrate(uint32_t bitrate_bps);
can_status_t CAN_DRV_GetErrorCounters(can_error_counters_t *out);

typedef struct {
    uint32_t id;
    can_id_type_t ide;
} can_filter_t;

can_status_t CAN_DRV_SetMasksFilters(uint32_t mask0, can_id_type_t mask0_ide,
                                     uint32_t mask1, can_id_type_t mask1_ide,
                                     const can_filter_t *filters, size_t nfilters,
                                     bool enable_rxb0_rollover);

can_status_t CAN_DRV_ApplyTPAcceptanceWindow(void);

can_status_t CAN_DRV_Transmit(const can_frame_t *frame, uint32_t timeout_ms);
can_status_t CAN_DRV_Receive(can_frame_t *out_frame);

typedef struct {
    bool rx0;
    bool rx1;
    bool tx0;
    bool tx1;
    bool tx2;
    bool err;
    bool wak;
} can_events_t;

can_status_t CAN_DRV_PollEvents(can_events_t *ev);

static inline uint8_t CAN_TP_EncodeAnglePayload(char angleId, const char *asciiVal,
                                                uint8_t *out /* >=5 bytes */)
{
    uint8_t i = 0;
    out[i++] = (uint8_t)angleId;
    for (; asciiVal && *asciiVal && i < 5; ++i) out[i] = (uint8_t)asciiVal[i-1];
    return i;
}

static inline uint8_t CAN_TP_EncodeLedByte(uint8_t groupNumber, bool r, bool g, bool b)
{
    uint8_t jkl = groupNumber & 0x07u;
    return (uint8_t)( (1u<<7) | (jkl<<4) | (0u<<3) | ((r?1u:0u)<<2) | ((g?1u:0u)<<1) | (b?1u:0u) );
}

can_status_t CAN_DRV_Reset(void);

can_status_t CAN_DRV_ReadReg(uint8_t addr, uint8_t *val);
can_status_t CAN_DRV_WriteReg(uint8_t addr, uint8_t val);
can_status_t CAN_DRV_BitModify(uint8_t addr, uint8_t mask, uint8_t data);

#ifdef __cplusplus
}
#endif
#endif /* CAN_DRV_H */
