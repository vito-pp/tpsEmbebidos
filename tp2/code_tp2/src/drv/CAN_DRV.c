#include "CAN_DRV.h"
#include "spi.h"          /* <- SPI de tu compañero */
#include <string.h>

/* ===== MCP25625: OPCODES SPI ===== */
#define OPC_RESET          0xC0u
#define OPC_READ           0x03u
#define OPC_WRITE          0x02u
#define OPC_READ_STATUS    0xA0u
#define OPC_RX_STATUS      0xB0u
#define OPC_RTS_MASK       0x80u

/* ===== MAPA DE REGISTROS (subset usado) ===== */
#define REG_RXF0SIDH 0x00u
#define REG_RXF0SIDL 0x01u
#define REG_RXF0EID8 0x02u
#define REG_RXF0EID0 0x03u
#define REG_RXF1SIDH 0x04u
#define REG_RXF1SIDL 0x05u
#define REG_RXF1EID8 0x06u
#define REG_RXF1EID0 0x07u
#define REG_RXF2SIDH 0x08u
#define REG_RXF2SIDL 0x09u
#define REG_RXF2EID8 0x0Au
#define REG_RXF2EID0 0x0Bu
#define REG_RXF3SIDH 0x10u
#define REG_RXF3SIDL 0x11u
#define REG_RXF3EID8 0x12u
#define REG_RXF3EID0 0x13u
#define REG_RXF4SIDH 0x14u
#define REG_RXF4SIDL 0x15u
#define REG_RXF4EID8 0x16u
#define REG_RXF4EID0 0x17u
#define REG_RXF5SIDH 0x18u
#define REG_RXF5SIDL 0x19u
#define REG_RXF5EID8 0x1Au
#define REG_RXF5EID0 0x1Bu

#define REG_TEC      0x1Cu
#define REG_REC      0x1Du

#define REG_RXM0SIDH 0x20u
#define REG_RXM0SIDL 0x21u
#define REG_RXM0EID8 0x22u
#define REG_RXM0EID0 0x23u
#define REG_RXM1SIDH 0x24u
#define REG_RXM1SIDL 0x25u
#define REG_RXM1EID8 0x26u
#define REG_RXM1EID0 0x27u

#define REG_CNF3     0x28u
#define REG_CNF2     0x29u
#define REG_CNF1     0x2Au
#define REG_CANINTE  0x2Bu
#define REG_CANINTF  0x2Cu
#define REG_EFLG     0x2Du
#define REG_CANSTAT  0x0Eu
#define REG_CANCTRL  0x0Fu

/* RXB0 */
#define REG_RXB0CTRL 0x60u
#define REG_RXB0SIDH 0x61u
#define REG_RXB0SIDL 0x62u
#define REG_RXB0EID8 0x63u
#define REG_RXB0EID0 0x64u
#define REG_RXB0DLC  0x65u
#define REG_RXB0D0   0x66u

/* RXB1 */
#define REG_RXB1CTRL 0x70u
#define REG_RXB1SIDH 0x71u
#define REG_RXB1SIDL 0x72u
#define REG_RXB1EID8 0x73u
#define REG_RXB1EID0 0x74u
#define REG_RXB1DLC  0x75u
#define REG_RXB1D0   0x76u

/* TXB0 */
#define REG_TXB0CTRL 0x30u
#define REG_TXB0SIDH 0x31u
#define REG_TXB0SIDL 0x32u
#define REG_TXB0EID8 0x33u
#define REG_TXB0EID0 0x34u
#define REG_TXB0DLC  0x35u
#define REG_TXB0D0   0x36u

/* TXB1 */
#define REG_TXB1CTRL 0x40u
#define REG_TXB1SIDH 0x41u
#define REG_TXB1SIDL 0x42u
#define REG_TXB1EID8 0x43u
#define REG_TXB1EID0 0x44u
#define REG_TXB1DLC  0x45u
#define REG_TXB1D0   0x46u

/* TXB2 */
#define REG_TXB2CTRL 0x50u
#define REG_TXB2SIDH 0x51u
#define REG_TXB2SIDL 0x52u
#define REG_TXB2EID8 0x53u
#define REG_TXB2EID0 0x54u
#define REG_TXB2DLC  0x55u
#define REG_TXB2D0   0x56u

/* CANCTRL bits */
#define CANCTRL_REQOP_MASK   0xE0u
#define CANCTRL_REQOP_CONF   0x80u
#define CANCTRL_REQOP_LISTEN 0x60u
#define CANCTRL_REQOP_LOOP   0x40u
#define CANCTRL_REQOP_SLEEP  0x20u
#define CANCTRL_REQOP_NORMAL 0x00u
#define CANCTRL_OSM          (1u<<3)
#define CANCTRL_CLKEN        (1u<<2)

/* CANINTE / CANINTF bits */
#define CANINTF_RX0IF (1u<<0)
#define CANINTF_RX1IF (1u<<1)
#define CANINTF_TX0IF (1u<<2)
#define CANINTF_TX1IF (1u<<3)
#define CANINTF_TX2IF (1u<<4)
#define CANINTF_ERRIF (1u<<5)
#define CANINTF_WAKIF (1u<<6)
#define CANINTF_MERRF (1u<<7)

/* EFLG bits */
#define EFLG_TXBO     (1u<<5)
#define EFLG_TXEP     (1u<<4)
#define EFLG_RXEP     (1u<<3)

/* RXBCTRL bits */
#define RXBCTRL_BUKT     (1u<<2)
#define RXBCTRL_RXM_FILT (0x00u)

/* Contexto mínimo (sin punteros de SPI) */
static struct {
    can_drv_io_t io;
    uint32_t bitrate;
    bool use_one_shot;
    bool enable_wakeup;
    bool enable_clkout;
} s_ctx;

/* ===================== SPI bridge (usando spi.h) ===================== */
/* Nota: tu SPI usa ring buffers + IRQ. En cada envío entran N “eco” en RX.
 * Para READ (3 bytes): tomamos el 3er POP como dato válido.
 */

static inline void spi_send1(uint8_t b)
{
    SPI0_sendByte(b);               /* CS se maneja internamente por PCS y CONT=0 */
    (void)SPI0_PopRxFIFO();         /* basura #1 */
}

static inline uint8_t spi_read1_reg(uint8_t addr)
{
    /* READ, addr, dummy -> el 3er POP es el dato */
    SPI0_send3Bytes(OPC_READ, addr, 0x00);
    (void)SPI0_PopRxFIFO();         /* echo READ   */
    (void)SPI0_PopRxFIFO();         /* echo ADDR   */
    uint16_t v = SPI0_PopRxFIFO();  /* DATA        */
    return (uint8_t)(v & 0xFFu);
}

static inline void spi_write1_reg(uint8_t addr, uint8_t val)
{
    /* WRITE, addr, val */
    SPI0_send3Bytes(OPC_WRITE, addr, val);
    (void)SPI0_PopRxFIFO();         /* echo WRITE  */
    (void)SPI0_PopRxFIFO();         /* echo ADDR   */
    (void)SPI0_PopRxFIFO();         /* echo VAL    */
}

static inline uint8_t spi_read_status_cmd(uint8_t cmd) /* READ_STATUS o RX_STATUS */
{
    /* cmd, dummy -> el 2do POP es el dato */
    SPI0_send2Bytes(cmd, 0x00);
    (void)SPI0_PopRxFIFO();         /* echo cmd    */
    uint16_t v = SPI0_PopRxFIFO();  /* status      */
    return (uint8_t)(v & 0xFFu);
}

/* BIT MODIFY por software: read-modify-write */
static inline void spi_bit_modify_soft(uint8_t addr, uint8_t mask, uint8_t data)
{
    uint8_t cur = spi_read1_reg(addr);
    uint8_t neu = (uint8_t)((cur & ~mask) | (data & mask));
    spi_write1_reg(addr, neu);
}

/* ==================== Helpers de ID y timing (igual que antes) ==================== */
static void id_to_regs(uint32_t id, can_id_type_t ide,
                       uint8_t *SIDH, uint8_t *SIDL, uint8_t *EID8, uint8_t *EID0)
{
    if (ide == CAN_ID_STD) {
        uint16_t sid = (uint16_t)(id & 0x7FFu);
        *SIDH = (uint8_t)(sid >> 3);
        *SIDL = (uint8_t)((sid & 0x07u) << 5);
        *EID8 = 0;
        *EID0 = 0;
    } else {
        uint32_t eid = id & 0x1FFFFFFFu;
        uint16_t sid = (uint16_t)((eid >> 18) & 0x7FFu);
        uint16_t eidh = (uint16_t)((eid >> 8) & 0xFFu);
        uint8_t  eidl = (uint8_t)(eid & 0xFFu);
        *SIDH = (uint8_t)(sid >> 3);
        *SIDL = (uint8_t)(((sid & 0x07u) << 5) | (1u<<3) /*EXIDE*/ | ((eid >> 16) & 0x03u));
        *EID8 = (uint8_t)eidh;
        *EID0 = (uint8_t)eidl;
    }
}

static void compute_cnf_125k(uint32_t fosc_hz, uint8_t *cnf1, uint8_t *cnf2, uint8_t *cnf3)
{
    (void)fosc_hz;
    uint8_t BRP = 3;   /* 16 TQ @16MHz */
    uint8_t SJW = 0;
    uint8_t PRSEG  = 2-1;
    uint8_t PHSEG1 = 6-1;
    uint8_t PHSEG2 = 7-1;
    *cnf1 = (uint8_t)((SJW<<6) | (BRP & 0x3Fu));
    *cnf2 = (uint8_t)( (1u<<7) | ((PHSEG1 & 0x07u)<<3) | (PRSEG & 0x07u) );
    *cnf3 = (uint8_t)(PHSEG2 & 0x07u);
}

/* ==================== Find TX buffer libre ==================== */
static int find_free_txb(uint8_t *ctrl_addr, uint8_t *sid_addr, uint8_t *dlc_addr, uint8_t *d0_addr, uint8_t *rts_bit)
{
    uint8_t ctrl = spi_read1_reg(REG_TXB0CTRL);
    if ((ctrl & 0x08u) == 0) {
        *ctrl_addr = REG_TXB0CTRL; *sid_addr = REG_TXB0SIDH; *dlc_addr = REG_TXB0DLC; *d0_addr = REG_TXB0D0;
        *rts_bit = 0x01u; return 0;
    }
    ctrl = spi_read1_reg(REG_TXB1CTRL);
    if ((ctrl & 0x08u) == 0) {
        *ctrl_addr = REG_TXB1CTRL; *sid_addr = REG_TXB1SIDH; *dlc_addr = REG_TXB1DLC; *d0_addr = REG_TXB1D0;
        *rts_bit = 0x02u; return 1;
    }
    ctrl = spi_read1_reg(REG_TXB2CTRL);
    if ((ctrl & 0x08u) == 0) {
        *ctrl_addr = REG_TXB2CTRL; *sid_addr = REG_TXB2SIDH; *dlc_addr = REG_TXB2DLC; *d0_addr = REG_TXB2D0;
        *rts_bit = 0x04u; return 2;
    }
    return -1;
}

/* ==================== API ==================== */
can_status_t CAN_DRV_Reset(void)
{
    SPI0_sendByte(OPC_RESET);
    (void)SPI0_PopRxFIFO();
    if (s_ctx.io.delay_us) s_ctx.io.delay_us(10);
    return CAN_OK;
}

static can_status_t set_mode_and_wait(uint8_t reqop)
{
    /* CANCTRL.REQOP = reqop (bit modify por soft) */
    spi_bit_modify_soft(REG_CANCTRL, CANCTRL_REQOP_MASK, reqop);

    for (uint32_t i=0;i<10000;i++){
        uint8_t stat = spi_read1_reg(REG_CANSTAT);
        if ((stat & CANCTRL_REQOP_MASK) == reqop) return CAN_OK;
        if (s_ctx.io.delay_us) s_ctx.io.delay_us(50);
    }
    return CAN_ERR_TIMEOUT;
}

can_status_t CAN_DRV_SetMode(can_mode_t mode)
{
    uint8_t req;
    switch(mode){
        case CAN_MODE_CONFIG:      req = CANCTRL_REQOP_CONF; break;
        case CAN_MODE_NORMAL:      req = CANCTRL_REQOP_NORMAL; break;
        case CAN_MODE_SLEEP:       req = CANCTRL_REQOP_SLEEP; break;
        case CAN_MODE_LISTEN_ONLY: req = CANCTRL_REQOP_LISTEN; break;
        case CAN_MODE_LOOPBACK:    req = CANCTRL_REQOP_LOOP; break;
        default: return CAN_ERR_PARAM;
    }
    return set_mode_and_wait(req);
}

static void enable_default_interrupts(void)
{
    /* RX0/1, TX0/1/2, ERR, WAK, MERR */
    spi_write1_reg(REG_CANINTE, (uint8_t)(CANINTF_RX0IF|CANINTF_RX1IF|CANINTF_TX0IF|CANINTF_TX1IF|CANINTF_TX2IF|CANINTF_ERRIF|CANINTF_WAKIF|CANINTF_MERRF));
}

static void clear_all_intf(void)
{
    spi_write1_reg(REG_CANINTF, 0x00);
    spi_write1_reg(REG_EFLG, 0x00);
}

can_status_t CAN_DRV_Init(const can_drv_config_t *cfg)
{
    if (!cfg) return CAN_ERR_PARAM;

    /* Guardamos config */
    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.io = cfg->io;
    s_ctx.bitrate = cfg->bitrate ? cfg->bitrate : CAN_DRV_DEFAULT_BITRATE;
    s_ctx.use_one_shot = cfg->use_one_shot;
    s_ctx.enable_wakeup = cfg->enable_wakeup;
    s_ctx.enable_clkout = cfg->enable_clkout;

    /* Inicializar SPI0 maestro (driver de tu compañero) */
    SPI0Master_Init(); /* configura pines y SPI0 */  /* spi.c/h */  /* NOLINT */
    /* :contentReference[oaicite:3]{index=3} :contentReference[oaicite:4]{index=4} */

    /* Reset físico opcional + RESET por comando */
    if (s_ctx.io.set_stby)  s_ctx.io.set_stby(false);
    if (s_ctx.io.set_reset) { s_ctx.io.set_reset(false); if (s_ctx.io.delay_us) s_ctx.io.delay_us(5); s_ctx.io.set_reset(true); if (s_ctx.io.delay_us) s_ctx.io.delay_us(10); }
    CAN_DRV_Reset();

    /* Configuration mode */
    if (set_mode_and_wait(CANCTRL_REQOP_CONF) != CAN_OK) return CAN_ERR_TIMEOUT;

    /* Bit timing (125k) */
    if (s_ctx.bitrate != 125000u) return CAN_ERR_PARAM;
    uint8_t cnf1, cnf2, cnf3;
    compute_cnf_125k(s_ctx.io.mcp_clk_hz ? s_ctx.io.mcp_clk_hz : 16000000u, &cnf1, &cnf2, &cnf3);
    spi_write1_reg(REG_CNF1, cnf1);
    spi_write1_reg(REG_CNF2, cnf2);
    spi_write1_reg(REG_CNF3, cnf3);

    /* RXB0/RXB1 config */
    spi_write1_reg(REG_RXB0CTRL, (uint8_t)(RXBCTRL_RXM_FILT | RXBCTRL_BUKT));
    spi_write1_reg(REG_RXB1CTRL, (uint8_t)(RXBCTRL_RXM_FILT));

    /* One-Shot / CLKOUT */
    uint8_t canctrl = spi_read1_reg(REG_CANCTRL);
    canctrl &= (uint8_t)~(CANCTRL_OSM | CANCTRL_CLKEN);
    if (s_ctx.use_one_shot)  canctrl |= CANCTRL_OSM;
    if (s_ctx.enable_clkout) canctrl |= CANCTRL_CLKEN;
    spi_write1_reg(REG_CANCTRL, canctrl);

    enable_default_interrupts();
    clear_all_intf();

    /* Filtros por defecto del TP */
    CAN_DRV_ApplyTPAcceptanceWindow();

    return CAN_DRV_SetMode(CAN_MODE_NORMAL);
}

can_status_t CAN_DRV_SetBitrate(uint32_t bitrate_bps)
{
    if (bitrate_bps != 125000u) return CAN_ERR_PARAM;

    uint8_t prev = spi_read1_reg(REG_CANSTAT) & CANCTRL_REQOP_MASK;
    if (set_mode_and_wait(CANCTRL_REQOP_CONF) != CAN_OK) return CAN_ERR_TIMEOUT;

    uint8_t cnf1, cnf2, cnf3;
    compute_cnf_125k(s_ctx.io.mcp_clk_hz ? s_ctx.io.mcp_clk_hz : 16000000u, &cnf1, &cnf2, &cnf3);
    spi_write1_reg(REG_CNF1, cnf1);
    spi_write1_reg(REG_CNF2, cnf2);
    spi_write1_reg(REG_CNF3, cnf3);

    return set_mode_and_wait(prev);
}

/* ==================== Máscaras / Filtros ==================== */
static void write_mask_block(uint8_t base_sid, uint32_t id, can_id_type_t ide)
{
    uint8_t SIDH, SIDL, EID8, EID0;
    id_to_regs(id, ide, &SIDH, &SIDL, &EID8, &EID0);
    spi_write1_reg(base_sid + 0, SIDH);
    spi_write1_reg(base_sid + 1, SIDL);
    spi_write1_reg(base_sid + 2, EID8);
    spi_write1_reg(base_sid + 3, EID0);
}

static void write_filter_block(uint8_t base_sid, uint32_t id, can_id_type_t ide)
{
    write_mask_block(base_sid, id, ide);
}

can_status_t CAN_DRV_SetMasksFilters(uint32_t mask0, can_id_type_t mask0_ide,
                                     uint32_t mask1, can_id_type_t mask1_ide,
                                     const can_filter_t *filters, size_t nfilters,
                                     bool enable_rxb0_rollover)
{
    if (set_mode_and_wait(CANCTRL_REQOP_CONF) != CAN_OK) return CAN_ERR_TIMEOUT;

    /* Máscaras */
    write_mask_block(REG_RXM0SIDH, mask0, mask0_ide);
    write_mask_block(REG_RXM1SIDH, mask1, mask1_ide);

    /* Filtros RXF0..RXF5 */
    const uint8_t fl_sid[6] = {REG_RXF0SIDH,REG_RXF1SIDH,REG_RXF2SIDH,REG_RXF3SIDH,REG_RXF4SIDH,REG_RXF5SIDH};
    size_t count = (nfilters>6)?6:nfilters;
    for (size_t i=0;i<count;i++){
        write_filter_block(fl_sid[i], filters[i].id, filters[i].ide);
    }

    /* Rollover */
    uint8_t v = enable_rxb0_rollover ? (RXBCTRL_RXM_FILT | RXBCTRL_BUKT) : RXBCTRL_RXM_FILT;
    spi_write1_reg(REG_RXB0CTRL, v);
    spi_write1_reg(REG_RXB1CTRL, RXBCTRL_RXM_FILT);

    return CAN_DRV_SetMode(CAN_MODE_NORMAL);
}

/* ==================== TX / RX ==================== */
can_status_t CAN_DRV_Transmit(const can_frame_t *frame, uint32_t timeout_ms)
{
    if (!frame || frame->dlc > CAN_MAX_DLEN) return CAN_ERR_PARAM;

    /* Elegir buffer libre */
    uint8_t ctrl, sid, dlc, d0, rts_bit;
    int buf = find_free_txb(&ctrl, &sid, &dlc, &d0, &rts_bit);
    if (buf < 0) return CAN_ERR_BUSY;

    /* Programar ID */
    uint8_t sidh,sidl,eid8,eid0;
    id_to_regs(frame->id, frame->ide, &sidh, &sidl, &eid8, &eid0);
    spi_write1_reg(sid + 0, sidh);
    spi_write1_reg(sid + 1, sidl);
    spi_write1_reg(sid + 2, eid8);
    spi_write1_reg(sid + 3, eid0);

    /* DLC (+RTR en bit 6 del DLC para TX) */
    uint8_t dlc_val = (uint8_t)(frame->dlc & 0x0Fu);
    if (frame->rtr) dlc_val |= 0x40u;
    spi_write1_reg(dlc, dlc_val);

    /* Datos */
    if (!frame->rtr && frame->dlc){
        for (uint8_t i=0; i<frame->dlc; i++){
            spi_write1_reg((uint8_t)(d0 + i), frame->data[i]);
        }
    }

    /* RTS */
    SPI0_sendByte((uint8_t)(OPC_RTS_MASK | rts_bit));
    (void)SPI0_PopRxFIFO();

    /* Espera opcional a TXxIF */
    if (timeout_ms){
        uint32_t loops = (timeout_ms*1000u)/50u + 1u;
        while (loops--){
            uint8_t intf = spi_read1_reg(REG_CANINTF);
            if ( (buf==0 && (intf & CANINTF_TX0IF)) ||
                 (buf==1 && (intf & CANINTF_TX1IF)) ||
                 (buf==2 && (intf & CANINTF_TX2IF)) ) {
                /* limpiar flag del buffer */
                uint8_t clr = (buf==0)?CANINTF_TX0IF: (buf==1)?CANINTF_TX1IF:CANINTF_TX2IF;
                /* bit modify suave */
                spi_bit_modify_soft(REG_CANINTF, clr, 0x00);
                return CAN_OK;
            }
            if (s_ctx.io.delay_us) s_ctx.io.delay_us(50);
        }
        return CAN_ERR_TIMEOUT;
    }
    return CAN_OK;
}

can_status_t CAN_DRV_Receive(can_frame_t *out_frame)
{
    if (!out_frame) return CAN_ERR_PARAM;

    /* RX_STATUS rápido (cmd de 2 bytes) */
    uint8_t rs = spi_read_status_cmd(OPC_RX_STATUS);
    bool has0 = (rs & 0x40u) != 0;
    bool has1 = (rs & 0x80u) != 0;
    if (!has0 && !has1) return CAN_ERR_NO_MSG;

    /* Leer cabecera desde registros RXBx */
    uint8_t SIDH, SIDL, EID8, EID0, DLC;
    uint8_t base = has0 ? REG_RXB0SIDH : REG_RXB1SIDH;
    uint8_t baseDLC = has0 ? REG_RXB0DLC  : REG_RXB1DLC;
    uint8_t baseD0  = has0 ? REG_RXB0D0   : REG_RXB1D0;

    SIDH = spi_read1_reg(base + 0);
    SIDL = spi_read1_reg(base + 1);
    EID8 = spi_read1_reg(base + 2);
    EID0 = spi_read1_reg(base + 3);
    DLC  = spi_read1_reg(baseDLC);

    /* Decodificar ID */
    if (SIDL & (1u<<3)){
        uint32_t sid = ((uint32_t)SIDH<<3) | (uint32_t)(SIDL>>5);
        uint32_t eid = ((uint32_t)(SIDL & 0x03u)<<16) | ((uint32_t)EID8<<8) | (uint32_t)EID0;
        out_frame->id  = (sid<<18) | eid;
        out_frame->ide = CAN_ID_EXT;
    } else {
        uint32_t sid = ((uint32_t)SIDH<<3) | (uint32_t)(SIDL>>5);
        out_frame->id  = sid & 0x7FFu;
        out_frame->ide = CAN_ID_STD;
    }

    out_frame->rtr = (DLC & 0x40u) ? true:false;
    out_frame->dlc = (uint8_t)(DLC & 0x0Fu);

    if (out_frame->dlc){
        for (uint8_t i=0;i<out_frame->dlc;i++){
            out_frame->data[i] = spi_read1_reg((uint8_t)(baseD0 + i));
        }
    }

    /* Limpiar flag RXxIF */
    uint8_t clr = has0 ? CANINTF_RX0IF : CANINTF_RX1IF;
    spi_bit_modify_soft(REG_CANINTF, clr, 0x00);

    return CAN_OK;
}

can_status_t CAN_DRV_PollEvents(can_events_t *ev)
{
    if (!ev) return CAN_ERR_PARAM;
    uint8_t intf = spi_read1_reg(REG_CANINTF);
    ev->rx0 = (intf & CANINTF_RX0IF)!=0;
    ev->rx1 = (intf & CANINTF_RX1IF)!=0;
    ev->tx0 = (intf & CANINTF_TX0IF)!=0;
    ev->tx1 = (intf & CANINTF_TX1IF)!=0;
    ev->tx2 = (intf & CANINTF_TX2IF)!=0;
    ev->err = (intf & CANINTF_MERRF)!=0;
    ev->wak = (intf & CANINTF_WAKIF)!=0;
    return CAN_OK;
}

can_status_t CAN_DRV_GetErrorCounters(can_error_counters_t *out)
{
    if (!out) return CAN_ERR_PARAM;
    uint8_t tec = spi_read1_reg(REG_TEC);
    uint8_t rec = spi_read1_reg(REG_REC);
    uint8_t eflg= spi_read1_reg(REG_EFLG);
    out->tx_err = tec;
    out->rx_err = rec;
    out->bus_off = (eflg & EFLG_TXBO)!=0;
    out->error_passive = ((eflg & (EFLG_TXEP|EFLG_RXEP))!=0);
    return CAN_OK;
}

/* ==================== Acceso de bajo nivel (debug) ==================== */
can_status_t CAN_DRV_ReadReg(uint8_t addr, uint8_t *val)
{
    if (!val) return CAN_ERR_PARAM;
    *val = spi_read1_reg(addr);
    return CAN_OK;
}
can_status_t CAN_DRV_WriteReg(uint8_t addr, uint8_t val)
{
    spi_write1_reg(addr, val);
    return CAN_OK;
}
can_status_t CAN_DRV_BitModify(uint8_t addr, uint8_t mask, uint8_t data)
{
    spi_bit_modify_soft(addr, mask, data);
    return CAN_OK;
}

/* ==================== Helper: filtros TP ==================== */
can_status_t CAN_DRV_ApplyTPAcceptanceWindow(void)
{
    can_filter_t fl[6] = {0};
    uint32_t mask = 0x7F8u; /* 0x100..0x107 */
    for (int i=0;i<6;i++){ fl[i].id = (uint32_t)(0x100u + i); fl[i].ide = CAN_ID_STD; }
    return CAN_DRV_SetMasksFilters(mask, CAN_ID_STD, mask, CAN_ID_STD, fl, 6, true);
}
