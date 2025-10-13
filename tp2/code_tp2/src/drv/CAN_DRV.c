#include "CAN_DRV.h"
#include <string.h>

/* ============================================================
 *   MCP25625: OPCODES SPI  (Tabla 5-1)
 * ============================================================ */
#define OPC_RESET          0xC0u
#define OPC_READ           0x03u
#define OPC_WRITE          0x02u
#define OPC_BIT_MODIFY     0x05u
#define OPC_READ_STATUS    0xA0u
#define OPC_RX_STATUS      0xB0u
#define OPC_RTS_MASK       0x80u      /* 1000 0nnn : n=buffer */
#define OPC_LOAD_TXB       0x40u      /* 0100 0abc */
#define OPC_READ_RXB       0x90u      /* 1001 0nm0 */

/* LOAD TX BUFFER target codes (a,b,c) -> ver DS */
#define LTXB0SID           0x00u  /* TXB0, address=SIDH */
#define LTXB0D0            0x01u  /* TXB0, address=Dx   */
#define LTXB1SID           0x02u
#define LTXB1D0            0x03u
#define LTXB2SID           0x04u
#define LTXB2D0            0x05u

/* READ RX BUFFER target codes (n,m) -> ver DS */
#define RRXB0SID           0x00u  /* RXB0, address=SIDH */
#define RRXB0D0            0x02u  /* RXB0, address=Dx   */
#define RRXB1SID           0x04u
#define RRXB1D0            0x06u

/* RTS bits (nnn) */
#define RTS_TXB0           0x01u
#define RTS_TXB1           0x02u
#define RTS_TXB2           0x04u

/* ============================================================
 *   MCP25625: MAPA DE REGISTROS (Tabla 4-1)
 *   (mismos offsets que MCP2515; el MCP25625 los integra)
 * ============================================================ */
#define REG_RXF0SIDH       0x00u
#define REG_RXF0SIDL       0x01u
#define REG_RXF0EID8       0x02u
#define REG_RXF0EID0       0x03u
#define REG_RXF1SIDH       0x04u
#define REG_RXF1SIDL       0x05u
#define REG_RXF1EID8       0x06u
#define REG_RXF1EID0       0x07u
#define REG_RXF2SIDH       0x08u
#define REG_RXF2SIDL       0x09u
#define REG_RXF2EID8       0x0Au
#define REG_RXF2EID0       0x0Bu
#define REG_BFPCTRL        0x0Cu
#define REG_TXRTSCTRL      0x0Du

#define REG_RXF3SIDH       0x10u
#define REG_RXF3SIDL       0x11u
#define REG_RXF3EID8       0x12u
#define REG_RXF3EID0       0x13u
#define REG_RXF4SIDH       0x14u
#define REG_RXF4SIDL       0x15u
#define REG_RXF4EID8       0x16u
#define REG_RXF4EID0       0x17u
#define REG_RXF5SIDH       0x18u
#define REG_RXF5SIDL       0x19u
#define REG_RXF5EID8       0x1Au
#define REG_RXF5EID0       0x1Bu
#define REG_TEC            0x1Cu
#define REG_REC            0x1Du

#define REG_RXM0SIDH       0x20u
#define REG_RXM0SIDL       0x21u
#define REG_RXM0EID8       0x22u
#define REG_RXM0EID0       0x23u
#define REG_RXM1SIDH       0x24u
#define REG_RXM1SIDL       0x25u
#define REG_RXM1EID8       0x26u
#define REG_RXM1EID0       0x27u
#define REG_CNF3           0x28u
#define REG_CNF2           0x29u
#define REG_CNF1           0x2Au
#define REG_CANINTE        0x2Bu
#define REG_CANINTF        0x2Cu
#define REG_EFLG           0x2Du

/* RXB0/1 Control y cabecera */
#define REG_RXB0CTRL       0x60u
#define REG_RXB0SIDH       0x61u
#define REG_RXB0SIDL       0x62u
#define REG_RXB0EID8       0x63u
#define REG_RXB0EID0       0x64u
#define REG_RXB0DLC        0x65u
#define REG_RXB0D0         0x66u

#define REG_RXB1CTRL       0x70u
#define REG_RXB1SIDH       0x71u
#define REG_RXB1SIDL       0x72u
#define REG_RXB1EID8       0x73u
#define REG_RXB1EID0       0x74u
#define REG_RXB1DLC        0x75u
#define REG_RXB1D0         0x76u

/* TXB0/1/2 */
#define REG_TXB0CTRL       0x30u
#define REG_TXB0SIDH       0x31u
#define REG_TXB0SIDL       0x32u
#define REG_TXB0EID8       0x33u
#define REG_TXB0EID0       0x34u
#define REG_TXB0DLC        0x35u
#define REG_TXB0D0         0x36u

#define REG_TXB1CTRL       0x40u
#define REG_TXB1SIDH       0x41u
#define REG_TXB1SIDL       0x42u
#define REG_TXB1EID8       0x43u
#define REG_TXB1EID0       0x44u
#define REG_TXB1DLC        0x45u
#define REG_TXB1D0         0x46u

#define REG_TXB2CTRL       0x50u
#define REG_TXB2SIDH       0x51u
#define REG_TXB2SIDL       0x52u
#define REG_TXB2EID8       0x53u
#define REG_TXB2EID0       0x54u
#define REG_TXB2DLC        0x55u
#define REG_TXB2D0         0x56u

/* CANCTRL / CANSTAT (modo) */
#define REG_CANCTRL        0x0Fu
#define REG_CANSTAT        0x0Eu

/* CANCTRL bits */
#define CANCTRL_REQOP_MASK  (0xE0u)
#define CANCTRL_REQOP_CONF  (0x80u)
#define CANCTRL_REQOP_LISTEN (0x60u)
#define CANCTRL_REQOP_LOOP   (0x40u)
#define CANCTRL_REQOP_SLEEP  (0x20u)
#define CANCTRL_REQOP_NORMAL (0x00u)
#define CANCTRL_OSM         (1u<<3) /* One-Shot Mode */
#define CANCTRL_CLKEN       (1u<<2) /* CLKOUT enable */

/* CANINTE / CANINTF bits */
#define CANINTF_RX0IF   (1u<<0)
#define CANINTF_RX1IF   (1u<<1)
#define CANINTF_TX0IF   (1u<<2)
#define CANINTF_TX1IF   (1u<<3)
#define CANINTF_TX2IF   (1u<<4)
#define CANINTF_ERRIF   (1u<<5)
#define CANINTF_WAKIF   (1u<<6)
#define CANINTF_MERRF   (1u<<7)

/* EFLG flags */
#define EFLG_TXBO      (1u<<5)
#define EFLG_TXEP      (1u<<4)
#define EFLG_RXEP      (1u<<3)

/* RXBCTRL bits */
#define RXBCTRL_BUKT   (1u<<2)  /* rollover RXB0->RXB1 */
#define RXBCTRL_RXM_ALL (0x60u) /* 11: deshabilita filtros/mascaras (recibe todo) */
#define RXBCTRL_RXM_FILT (0x00u)

/* Helpers locales */
static struct {
    can_drv_io_t io;
    uint32_t bitrate;
    bool use_one_shot;
    bool enable_wakeup;
    bool enable_clkout;
} s_ctx;

/* ------------------------------------------------------------
 *   SPI helpers
 * ------------------------------------------------------------ */
static inline void spi_begin(void){ s_ctx.io.cs_assert(); }
static inline void spi_end(void){ s_ctx.io.cs_deassert(); }

static void spi_cmd1(uint8_t cmd)
{
    uint8_t tx = cmd; s_ctx.io.spi_xfer(&tx, NULL, 1);
}
static void spi_txn(const uint8_t *tx, uint8_t *rx, size_t n)
{
    s_ctx.io.spi_xfer(tx, rx, n);
}

static void wr8(uint8_t addr, uint8_t val)
{
    spi_begin();
    uint8_t tx[3] = {OPC_WRITE, addr, val};
    spi_txn(tx, NULL, 3);
    spi_end();
}
static void wrN(uint8_t addr, const uint8_t *buf, size_t n)
{
    spi_begin();
    uint8_t hdr[2] = {OPC_WRITE, addr};
    spi_txn(hdr, NULL, 2);
    spi_txn(buf, NULL, n);
    spi_end();
}
static void rdN(uint8_t addr, uint8_t *buf, size_t n)
{
    spi_begin();
    uint8_t hdr[2] = {OPC_READ, addr};
    spi_txn(hdr, NULL, 2);
    memset(buf, 0, n);
    spi_txn(NULL, buf, n);
    spi_end();
}
static void bit_modify(uint8_t addr, uint8_t mask, uint8_t data)
{
    spi_begin();
    uint8_t tx[4] = {OPC_BIT_MODIFY, addr, mask, data};
    spi_txn(tx, NULL, 4);
    spi_end();
}
static uint8_t read_status(void)
{
    spi_begin();
    uint8_t tx = OPC_READ_STATUS, st = 0;
    spi_txn(&tx, NULL, 1);
    spi_txn(NULL, &st, 1);
    spi_end();
    return st;
}
static uint8_t rx_status(void)
{
    spi_begin();
    uint8_t tx = OPC_RX_STATUS, st = 0;
    spi_txn(&tx, NULL, 1);
    spi_txn(NULL, &st, 1);
    spi_end();
    return st;
}
static void cmd_reset(void)
{
    spi_begin();
    spi_cmd1(OPC_RESET);
    spi_end();
    if (s_ctx.io.delay_us) s_ctx.io.delay_us(10); /* tRL >= 2us */
}
static void cmd_rts(uint8_t mask)
{
    spi_begin();
    uint8_t tx = (uint8_t)(OPC_RTS_MASK | (mask & 0x07u));
    spi_txn(&tx, NULL, 1);
    spi_end();
}

/* ------------------------------------------------------------
 *   Codificación IDs STD/EXT a registros SIDH/SIDL/EID8/EID0
 * ------------------------------------------------------------ */
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

/* ------------------------------------------------------------
 *   Bit timing para 125 kbit/s (Fosc típico 16 MHz)
 *   TQ = 2*(BRP+1)/Fosc; elegimos 16 TQ -> BRP=3; SJW=1
 *   PRSEG=2, PHSEG1=6, PHSEG2=7: 1+2+6+7 = 16 TQ
 * ------------------------------------------------------------ */
static void compute_cnf_125k(uint32_t fosc_hz, uint8_t *cnf1, uint8_t *cnf2, uint8_t *cnf3)
{
    (void)fosc_hz; /* para 16MHz esta elección es estándar robusta */
    uint8_t BRP = 3;   /* TQ=500ns a 16MHz */
    uint8_t SJW = 0;   /* 1 TQ */
    uint8_t PRSEG  = 2-1; /* Reg guarda (value-1) */
    uint8_t PHSEG1 = 6-1;
    uint8_t PHSEG2 = 7-1;
    /* CNF1: SJW<7:6> | BRP<5:0> */
    *cnf1 = (uint8_t)((SJW<<6) | (BRP & 0x3Fu));
    /* CNF2: BTLMODE=1 | SAM=0 | PHSEG1<5:3> | PRSEG<2:0> */
    *cnf2 = (uint8_t)( (1u<<7) | (0u<<6) | ((PHSEG1 & 0x07u)<<3) | (PRSEG & 0x07u) );
    /* CNF3: SOF=0 | WAKFIL=0 | PHSEG2<2:0> */
    *cnf3 = (uint8_t)(PHSEG2 & 0x07u);
}

/* ------------------------------------------------------------
 *   Selección de buffer TX libre (TXREQ=0)
 * ------------------------------------------------------------ */
static int find_free_txb(uint8_t *ctrl_addr, uint8_t *sid_addr, uint8_t *dlc_addr, uint8_t *d0_addr, uint8_t *rts_bit, uint8_t *ltxb_sid, uint8_t *ltxb_d0)
{
    uint8_t ctrl;
    rdN(REG_TXB0CTRL, &ctrl, 1);
    if ((ctrl & 0x08u) == 0) { /* TXREQ=bit3 */
        *ctrl_addr = REG_TXB0CTRL; *sid_addr = REG_TXB0SIDH; *dlc_addr = REG_TXB0DLC; *d0_addr = REG_TXB0D0;
        *rts_bit = RTS_TXB0; *ltxb_sid = (uint8_t)LTXB0SID; *ltxb_d0 = (uint8_t)LTXB0D0; return 0;
    }
    rdN(REG_TXB1CTRL, &ctrl, 1);
    if ((ctrl & 0x08u) == 0) {
        *ctrl_addr = REG_TXB1CTRL; *sid_addr = REG_TXB1SIDH; *dlc_addr = REG_TXB1DLC; *d0_addr = REG_TXB1D0;
        *rts_bit = RTS_TXB1; *ltxb_sid = (uint8_t)LTXB1SID; *ltxb_d0 = (uint8_t)LTXB1D0; return 1;
    }
    rdN(REG_TXB2CTRL, &ctrl, 1);
    if ((ctrl & 0x08u) == 0) {
        *ctrl_addr = REG_TXB2CTRL; *sid_addr = REG_TXB2SIDH; *dlc_addr = REG_TXB2DLC; *d0_addr = REG_TXB2D0;
        *rts_bit = RTS_TXB2; *ltxb_sid = (uint8_t)LTXB2SID; *ltxb_d0 = (uint8_t)LTXB2D0; return 2;
    }
    return -1;
}

/* ------------------------------------------------------------
 *   API
 * ------------------------------------------------------------ */
can_status_t CAN_DRV_Reset(void)
{
    cmd_reset();
    return CAN_OK;
}

static can_status_t set_mode_and_wait(uint8_t reqop)
{
    bit_modify(REG_CANCTRL, CANCTRL_REQOP_MASK, reqop);
    /* Poll CANSTAT OPMOD until matches. */
    for (uint32_t i=0;i<10000;i++){
        uint8_t stat;
        rdN(REG_CANSTAT, &stat, 1);
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
    wr8(REG_CANINTE, (uint8_t)(CANINTF_RX0IF|CANINTF_RX1IF|CANINTF_TX0IF|CANINTF_TX1IF|CANINTF_TX2IF|CANINTF_ERRIF|CANINTF_WAKIF|CANINTF_MERRF));
}

static void clear_all_intf(void)
{
    wr8(REG_CANINTF, 0x00);
    wr8(REG_EFLG, 0x00);
}

can_status_t CAN_DRV_Init(const can_drv_config_t *cfg)
{
    if (!cfg || !cfg->io.spi_xfer || !cfg->io.cs_assert || !cfg->io.cs_deassert) return CAN_ERR_PARAM;

    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.io = cfg->io;
    s_ctx.bitrate = cfg->bitrate ? cfg->bitrate : CAN_DRV_DEFAULT_BITRATE;
    s_ctx.use_one_shot = cfg->use_one_shot;
    s_ctx.enable_wakeup = cfg->enable_wakeup;
    s_ctx.enable_clkout = cfg->enable_clkout;

    /* Reset por SPI y STBY/RESET si están disponibles */
    if (s_ctx.io.set_stby) s_ctx.io.set_stby(false);  /* STBY=0 (transceiver activo) */
    if (s_ctx.io.set_reset){ s_ctx.io.set_reset(false); if (s_ctx.io.delay_us) s_ctx.io.delay_us(5); s_ctx.io.set_reset(true); if (s_ctx.io.delay_us) s_ctx.io.delay_us(10); }
    cmd_reset();

    /* Entramos en Configuration */
    if (set_mode_and_wait(CANCTRL_REQOP_CONF) != CAN_OK) return CAN_ERR_TIMEOUT;

    /* Bit timing */
    if (s_ctx.bitrate != 125000u) {
        /* Sólo implementamos cálculo robusto para 125k (TP). Para otros, extender. */
        return CAN_ERR_PARAM;
    }
    uint8_t cnf1, cnf2, cnf3;
    compute_cnf_125k(s_ctx.io.mcp_clk_hz ? s_ctx.io.mcp_clk_hz : 16000000u, &cnf1, &cnf2, &cnf3);
    wr8(REG_CNF1, cnf1);
    wr8(REG_CNF2, cnf2);
    wr8(REG_CNF3, cnf3);

    /* RXB0/RXB1: usar filtros/máscaras, habilitar rollover RXB0->RXB1 */
    wr8(REG_RXB0CTRL, RXBCTRL_RXM_FILT | RXBCTRL_BUKT);
    wr8(REG_RXB1CTRL, RXBCTRL_RXM_FILT);

    /* One-Shot / CLKOUT */
    uint8_t canctrl = 0;
    rdN(REG_CANCTRL, &canctrl, 1);
    canctrl &= ~(CANCTRL_OSM | CANCTRL_CLKEN);
    if (s_ctx.use_one_shot) canctrl |= CANCTRL_OSM;
    if (s_ctx.enable_clkout) canctrl |= CANCTRL_CLKEN;
    wr8(REG_CANCTRL, canctrl);

    /* INTs básicos */
    enable_default_interrupts();
    clear_all_intf();

    /* Aplicar ventana de aceptación por defecto del TP (0x100..0x107) */
    CAN_DRV_ApplyTPAcceptanceWindow();

    /* Normal mode */
    return CAN_DRV_SetMode(CAN_MODE_NORMAL);
}

can_status_t CAN_DRV_SetBitrate(uint32_t bitrate_bps)
{
    if (bitrate_bps != 125000u) return CAN_ERR_PARAM;
    /* Guardar modo actual */
    uint8_t stat; rdN(REG_CANSTAT, &stat, 1);
    uint8_t prev = stat & CANCTRL_REQOP_MASK;

    if (set_mode_and_wait(CANCTRL_REQOP_CONF) != CAN_OK) return CAN_ERR_TIMEOUT;

    uint8_t cnf1, cnf2, cnf3;
    compute_cnf_125k(s_ctx.io.mcp_clk_hz ? s_ctx.io.mcp_clk_hz : 16000000u, &cnf1, &cnf2, &cnf3);
    wr8(REG_CNF1, cnf1);
    wr8(REG_CNF2, cnf2);
    wr8(REG_CNF3, cnf3);

    return set_mode_and_wait(prev);
}

/* ------------------------------------------------------------
 *   Máscaras / Filtros
 * ------------------------------------------------------------ */
static void write_mask(uint8_t base_sid, uint32_t id, can_id_type_t ide)
{
    uint8_t b[4]; uint8_t SIDH, SIDL, EID8, EID0;
    id_to_regs(id, ide, &SIDH, &SIDL, &EID8, &EID0);
    b[0]=SIDH; b[1]=SIDL; b[2]=EID8; b[3]=EID0;
    wrN(base_sid, b, 4);
}
static void write_filter(uint8_t base_sid, uint32_t id, can_id_type_t ide)
{
    write_mask(base_sid, id, ide);
}

can_status_t CAN_DRV_SetMasksFilters(uint32_t mask0, can_id_type_t mask0_ide,
                                     uint32_t mask1, can_id_type_t mask1_ide,
                                     const can_filter_t *filters, size_t nfilters,
                                     bool enable_rxb0_rollover)
{
    /* Solo en Configuration */
    if (set_mode_and_wait(CANCTRL_REQOP_CONF) != CAN_OK) return CAN_ERR_TIMEOUT;

    /* Máscaras */
    write_mask(REG_RXM0SIDH, mask0, mask0_ide);
    write_mask(REG_RXM1SIDH, mask1, mask1_ide);

    /* Filtros (secuenciales: RXF0..RXF5) */
    const uint8_t fl_sid[6] = {REG_RXF0SIDH,REG_RXF1SIDH,REG_RXF2SIDH,REG_RXF3SIDH,REG_RXF4SIDH,REG_RXF5SIDH};
    size_t count = (nfilters>6)?6:nfilters;
    for (size_t i=0;i<count;i++){
        write_filter(fl_sid[i], filters[i].id, filters[i].ide);
    }

    /* Rollover */
    uint8_t v = enable_rxb0_rollover ? (RXBCTRL_RXM_FILT | RXBCTRL_BUKT) : RXBCTRL_RXM_FILT;
    wr8(REG_RXB0CTRL, v);
    wr8(REG_RXB1CTRL, RXBCTRL_RXM_FILT);

    /* Volver a Normal */
    return CAN_DRV_SetMode(CAN_MODE_NORMAL);
}

/* Aceptar únicamente STD IDs 0x100..0x107 */
can_status_t CAN_DRV_ApplyTPAcceptanceWindow(void)
{
    can_filter_t fl[6] = {0};
    /* Podemos usar máscara para aceptar 0x100..0x107:
       Mask = 0x7F8 (ID bits: 10..3 comparan; 2..0 libres) */
    uint32_t mask = 0x7F8u; /* std: 0b11111111000 */

    /* RXF0..RXF5: seteamos 0x100, 0x101, ..., 0x105 (p.ej.)
       y dejamos RXF? restantes con 0x106, 0x107 */
    for (int i=0;i<6;i++){ fl[i].id = (uint32_t)(0x100u + i); fl[i].ide = CAN_ID_STD; }

    /* Entrar a config, escribir máscaras y filtros, rollover ON */
    return CAN_DRV_SetMasksFilters(mask, CAN_ID_STD, mask, CAN_ID_STD, fl, 6, true);
}

/* ------------------------------------------------------------
 *   TX / RX
 * ------------------------------------------------------------ */
/**
 * @brief Transmite un frame CAN usando el primer buffer TX libre del MCP25625.
 *
 * Carga ID (STD/EXT), DLC y datos en el buffer disponible (TXB0/1/2) mediante
 * las instrucciones SPI LOAD TX BUFFER y luego dispara la transmisión con RTS.
 * Si @p timeout_ms > 0, hace espera activa hasta que el buffer señale TXxIF o
 * se agote el tiempo.
 *
 * @pre El driver debe estar inicializado (CAN_DRV_Init) y el controlador en modo Normal/Loopback.
 * @pre @p frame != NULL y 0 <= frame->dlc <= 8.
 *
 * @param[in] frame       Puntero al frame CAN a enviar (ID, tipo, RTR, DLC, data).
 * @param[in] timeout_ms  Tiempo máximo a esperar la finalización de TX (ms).
 *                        Use 0 para retorno inmediato (no bloqueante).
 *
 * @return can_status_t
 * @retval CAN_OK          Trama cargada y (si hubo espera) confirmada por TXxIF.
 * @retval CAN_ERR_PARAM   Parámetros inválidos (frame NULL o DLC > 8).
 * @retval CAN_ERR_BUSY    No hay buffers de transmisión libres (TXREQ=1 en los 3 TXB).
 * @retval CAN_ERR_TIMEOUT Se agotó el @p timeout_ms sin ver el flag TXxIF.
 *
 * @note En modo One-Shot (OSM=1), el hardware NO reintenta la transmisión.
 * @note La función limpia el flag TXxIF del buffer utilizado cuando espera con timeout.
 * @post Si timeout_ms==0, la función retorna tras emitir RTS (no garantiza que el frame salió al bus).
 *
 * @par Ejemplo
 * @code
 * can_frame_t f = { .id = 0x103, .ide = CAN_ID_STD, .rtr=false, .dlc=3,
 *                   .data = {'O','6','7'} };
 * CAN_DRV_Transmit(&f, 5); // espera hasta 5 ms a que termine la TX
 * @endcode
 */
can_status_t CAN_DRV_Transmit(const can_frame_t *frame, uint32_t timeout_ms)
{
    if (!frame || frame->dlc > CAN_MAX_DLEN) return CAN_ERR_PARAM; 
    // Valida puntero y que el tamaño de datos (DLC) no exceda 8. Si falla, error de parámetro.

    /* Elegir TX buffer libre */
    uint8_t ctrl, sid, dlc, d0, rts_bit, ltxb_sid, ltxb_d0; 
    // Variables donde se guardan direcciones de registro y bits asociados al buffer elegido.
    int buf = find_free_txb(&ctrl, &sid, &dlc, &d0, &rts_bit, &ltxb_sid, &ltxb_d0);
    // Busca un buffer de transmisión (TXB0/1/2) cuyo bit TXREQ esté libre.
    if (buf < 0) return CAN_ERR_BUSY;
    // Si no hay buffers libres, reporta que el hardware está ocupado.

    /* Cargar ID */
    uint8_t idb[4]; id_to_regs(frame->id, frame->ide, &idb[0], &idb[1], &idb[2], &idb[3]);
    // Convierte el ID (std/extendido) al formato de registros del MCP: SIDH, SIDL, EID8, EID0.

    /* LOAD TX BUFFER to SID */
    spi_begin(); 
    // Baja CS para comenzar una transacción SPI.
    uint8_t cmd = (uint8_t)(OPC_LOAD_TXB | ltxb_sid);
    // Arma la instrucción "LOAD TX BUFFER" apuntando al registro de cabecera SID del buffer seleccionado.
    spi_txn(&cmd, NULL, 1);
    // Envía la instrucción.
    spi_txn(idb, NULL, 4);
    // Escribe los 4 bytes de ID en los registros SIDH/SIDL/EID8/EID0 del TX buffer.
    spi_end();
    // Sube CS para terminar la transacción.

    /* DLC (RTR si corresponde) */
    uint8_t dlc_val = (uint8_t)(frame->dlc & 0x0Fu);
    // Prepara el valor DLC (0..8) en los 4 bits menos significativos.
    if (frame->rtr) dlc_val |= 0x40u; /* RTR en TXBxDLC */
    // Si es un frame RTR (remote request), setea el bit RTR en el registro DLC.
    wr8(dlc, dlc_val);
    // Escribe el registro TXBxDLC del buffer elegido.

    /* Datos si no es RTR */
    if (!frame->rtr && frame->dlc){
        // Si no es una solicitud remota y hay datos para enviar...
        spi_begin();
        // Baja CS para nueva transacción.
        cmd = (uint8_t)(OPC_LOAD_TXB | ltxb_d0);
        // Instrucción "LOAD TX BUFFER" apuntando al primer byte de datos (D0) del buffer.
        spi_txn(&cmd, NULL, 1);
        // Envía la instrucción.
        spi_txn(frame->data, NULL, frame->dlc);
        // Escribe los 'dlc' bytes de datos (D0..D7).
        spi_end();
        // Termina transacción SPI.
    }

    /* One-shot ya configurado en CANCTRL si aplica */
    // No hace nada aquí: si el modo One-Shot está activo, ya quedó configurado en INIT.

    /* Disparar RTS */
    cmd_rts(rts_bit);
    // Emite la instrucción RTS para solicitar la transmisión en el buffer seleccionado.

    /* Esperar a TXIF (opcional) */
    if (timeout_ms){
        // Si se pasó un timeout, se hace espera activa hasta que el buffer indique TX completado.
        uint32_t loops = (timeout_ms*1000u)/50u + 1u;
        // Calcula cuántos ciclos de espera de 50 us entran en el timeout total.
        while (loops--){
            uint8_t intf; rdN(REG_CANINTF, &intf, 1);
            // Lee CANINTF (flags de interrupciones) para ver si TXxIF se activó.
            if ( (buf==0 && (intf & CANINTF_TX0IF)) ||
                 (buf==1 && (intf & CANINTF_TX1IF)) ||
                 (buf==2 && (intf & CANINTF_TX2IF)) ) {
                /* limpiar flag del buffer */
                uint8_t clr = (buf==0)?CANINTF_TX0IF: (buf==1)?CANINTF_TX1IF:CANINTF_TX2IF;
                // Determina cuál flag de TX limpiar (según el buffer usado).
                bit_modify(REG_CANINTF, clr, 0x00);
                // Limpia el flag en CANINTF usando BIT MODIFY.
                return CAN_OK;
                // Retorna OK: transmisión completada.
            }
            if (s_ctx.io.delay_us) s_ctx.io.delay_us(50);
            // Espera 50 us entre polls para no saturar el bus SPI/CPU.
        }
        return CAN_ERR_TIMEOUT;
        // Si se agotó el tiempo, reporta timeout.
    }
    return CAN_OK;
    // Si no se pidió esperar (timeout_ms=0), retorna OK tras disparar RTS.
}

/**
 * @brief Intenta leer un frame desde RXB0 o RXB1 (polling).
 *
 * Consulta el estado de recepción con RX STATUS; si hay un mensaje pendiente en
 * RXB0 o RXB1, lo lee con READ RX BUFFER, decodifica ID (STD/EXT), RTR, DLC y
 * payload, y limpia el flag RX0IF/RX1IF correspondiente.
 *
 * @pre El driver debe estar inicializado (CAN_DRV_Init) y el controlador en modo Normal/Loopback.
 * @pre @p out_frame != NULL.
 *
 * @param[out] out_frame  Estructura donde se copia el frame recibido.
 *
 * @return can_status_t
 * @retval CAN_OK         Se leyó correctamente un frame y se limpió el flag RXxIF.
 * @retval CAN_ERR_PARAM  @p out_frame es NULL.
 * @retval CAN_ERR_NO_MSG No hay mensajes pendientes en RXB0 ni RXB1.
 *
 * @note Esta función no bloquea; si no hay mensajes, retorna inmediatamente.
 * @note Si el frame es RTR, el bit RTR se refleja en out_frame->rtr y out_frame->dlc puede ser > 0 (según origen).
 *
 * @par Ejemplo
 * @code
 * can_frame_t rcv;
 * if (CAN_DRV_Receive(&rcv) == CAN_OK) {
 *     // usar rcv.id, rcv.dlc, rcv.data[0..dlc-1]
 * }
 * @endcode
 */
can_status_t CAN_DRV_Receive(can_frame_t *out_frame)
{
    if (!out_frame) return CAN_ERR_PARAM;
    // Valida que el puntero de salida no sea NULL.

    /* Chequear RX STATUS rápidamente */
    uint8_t rs = rx_status();
    // Ejecuta la instrucción RX STATUS (consulta rápida del estado de recepción).
    bool has0 = (rs & 0x40u) != 0; /* bit6: message in RXB0 */
    // El bit 6 indica si hay un mensaje pendiente en RXB0.
    bool has1 = (rs & 0x80u) != 0; /* bit7: message in RXB1 */
    // El bit 7 indica si hay un mensaje pendiente en RXB1.
    uint8_t sel = 0xFFu;
    // Mantiene cuál buffer se leyó (solo informativo/depuración).
    uint8_t cmd = OPC_READ; (void)cmd;
    // Variable no usada (residuo de interfaz homogénea), se silencia el warning con (void).

    if (!has0 && !has1) return CAN_ERR_NO_MSG;
    // Si no hay mensajes en ninguno de los buffers, devuelve “no hay mensaje”.

    uint8_t SIDH, SIDL, EID8, EID0, DLC;
    // Variables para almacenar cabecera del mensaje recibido.
    uint8_t data[8] = {0};
    // Buffer temporal para los datos.

    if (has0){
        /* READ RX BUFFER (RXB0 SIDH) */
        spi_begin();
        // Baja CS para iniciar lectura por SPI.
        uint8_t c = (uint8_t)(OPC_READ_RXB | RRXB0SID);
        // Prepara instrucción READ RX BUFFER apuntando al SID de RXB0.
        spi_txn(&c, NULL, 1);
        // Envía la instrucción.
        spi_txn(NULL, &SIDH, 1);
        spi_txn(NULL, &SIDL, 1);
        spi_txn(NULL, &EID8, 1);
        spi_txn(NULL, &EID0, 1);
        spi_txn(NULL, &DLC, 1);
        // Lee secuencialmente los registros de cabecera: SIDH, SIDL, EID8, EID0, y DLC.
        uint8_t dlen = (uint8_t)(DLC & 0x0Fu);
        // Extrae longitud de datos (DLC) de los 4 bits bajos.
        if (dlen) spi_txn(NULL, data, dlen);
        // Si hay datos, los lee (D0..Dn).
        spi_end();
        // Termina la transacción SPI.
        sel = 0; /* RXB0 */
        // Marca que el origen fue RXB0 (informativo).
        /* limpiar RX0IF */
        bit_modify(REG_CANINTF, CANINTF_RX0IF, 0x00);
        // Limpia el flag de recepción RX0IF en CANINTF.
    } else {
        /* RXB1 */
        spi_begin();
        // Baja CS para iniciar lectura de RXB1.
        uint8_t c = (uint8_t)(OPC_READ_RXB | RRXB1SID);
        // Instrucción READ RX BUFFER para SID de RXB1.
        spi_txn(&c, NULL, 1);
        // Envía la instrucción.
        spi_txn(NULL, &SIDH, 1);
        spi_txn(NULL, &SIDL, 1);
        spi_txn(NULL, &EID8, 1);
        spi_txn(NULL, &EID0, 1);
        spi_txn(NULL, &DLC, 1);
        // Lee cabecera de RXB1.
        uint8_t dlen = (uint8_t)(DLC & 0x0Fu);
        // DLC (número de bytes de datos).
        if (dlen) spi_txn(NULL, data, dlen);
        // Si hay datos, los lee.
        spi_end();
        // Fin de transacción SPI.
        sel = 1;
        // Marca que el origen fue RXB1.
        bit_modify(REG_CANINTF, CANINTF_RX1IF, 0x00);
        // Limpia el flag RX1IF.
    }

    /* Decodificar ID */
    if (SIDL & (1u<<3)){ /* EXIDE=1 -> extendido */
        // Si el bit EXIDE está en 1, el mensaje usa ID extendido (29 bits).
        uint32_t sid = ((uint32_t)SIDH<<3) | (uint32_t)(SIDL>>5);
        // Reconstruye los 11 bits de SID a partir de SIDH:SIDL.
        uint32_t eid = ((uint32_t)(SIDL & 0x03u)<<16) | ((uint32_t)EID8<<8) | (uint32_t)EID0;
        // Reconstruye los 18 bits bajos (EID) a partir de SIDL[1:0], EID8, EID0.
        out_frame->id = (sid<<18) | eid;
        // Arma el ID completo de 29 bits: SID (11) seguido de EID (18).
        out_frame->ide = CAN_ID_EXT;
        // Marca tipo extendido.
    } else {
        uint32_t sid = ((uint32_t)SIDH<<3) | (uint32_t)(SIDL>>5);
        // Reconstruye el ID estándar de 11 bits desde SIDH:SIDL.
        out_frame->id = sid & 0x7FFu;
        // Asegura que quede en 11 bits.
        out_frame->ide = CAN_ID_STD;
        // Marca tipo estándar.
    }
    out_frame->rtr = (DLC & 0x40u) ? true:false;
    // El bit RTR se comparte en el registro DLC cuando el frame es RTR; lo mapea al campo booleano.
    out_frame->dlc = (uint8_t)(DLC & 0x0Fu);
    // Copia el valor de DLC (0..8) a la estructura de salida.
    if (out_frame->dlc) memcpy(out_frame->data, data, out_frame->dlc);
    // Si hay payload, copia los bytes leídos al buffer de salida.

    (void)sel;
    // 'sel' no se usa después; esto evita warning de variable no usada.
    return CAN_OK;
    // Indica que se obtuvo correctamente un mensaje.
}


/**
 * @brief Lee un snapshot de eventos/flags del registro CANINTF.
 *
 * Llena @p ev con el estado actual de los flags de recepción (RX0IF, RX1IF),
 * transmisión (TX0IF, TX1IF, TX2IF), error (MERRF) y wake-up (WAKIF).
 * No limpia los flags; solo informa.
 *
 * @pre El driver debe estar inicializado.
 * @pre @p ev != NULL.
 *
 * @param[out] ev  Estructura con los flags mapeados a booleanos.
 *
 * @return can_status_t
 * @retval CAN_OK         Lectura exitosa.
 * @retval CAN_ERR_PARAM  @p ev es NULL.
 *
 * @note Use CAN_DRV_Receive para leer y limpiar RXxIF, o CAN_DRV_Transmit (con timeout)
 *       para limpiar TXxIF del buffer usado. Para limpiar flags manualmente, use CAN_DRV_BitModify.
 *
 * @par Ejemplo
 * @code
 * can_events_t ev;
 * CAN_DRV_PollEvents(&ev);
 * if (ev.rx0 || ev.rx1) { (void)CAN_DRV_Receive(&frame); }
 * @endcode
 */
can_status_t CAN_DRV_PollEvents(can_events_t *ev)
{
    if (!ev) return CAN_ERR_PARAM;
    // Valida puntero de salida.

    uint8_t intf; rdN(REG_CANINTF, &intf, 1);
    // Lee el registro CANINTF (flags de interrupción/estado).

    ev->rx0 = (intf & CANINTF_RX0IF)!=0;
    // true si hay mensaje pendiente en RXB0.
    ev->rx1 = (intf & CANINTF_RX1IF)!=0;
    // true si hay mensaje pendiente en RXB1.
    ev->tx0 = (intf & CANINTF_TX0IF)!=0;
    // true si TXB0 terminó transmisión.
    ev->tx1 = (intf & CANINTF_TX1IF)!=0;
    // true si TXB1 terminó transmisión.
    ev->tx2 = (intf & CANINTF_TX2IF)!=0;
    // true si TXB2 terminó transmisión.
    ev->err = (intf & CANINTF_MERRF)!=0;
    // true si hubo error (Message Error/Bus error general).
    ev->wak = (intf & CANINTF_WAKIF)!=0;
    // true si hubo wake-up desde Sleep.
    return CAN_OK;
    // Devuelve OK con el snapshot de eventos.
}


/**
 * @brief Obtiene contadores de error (TEC/REC) y estado de error (passive, bus-off).
 *
 * Lee TEC (Transmit Error Counter), REC (Receive Error Counter) y EFLG (Error Flag).
 * Informa si el controlador se encuentra en bus-off y/o estado de error pasivo.
 *
 * @pre El driver debe estar inicializado.
 * @pre @p out != NULL.
 *
 * @param[out] out  Estructura con contadores y flags de error.
 *
 * @return can_status_t
 * @retval CAN_OK         Lectura exitosa.
 * @retval CAN_ERR_PARAM  @p out es NULL.
 *
 * @note Bus-off (TXBO=1) indica que el nodo está desconectado lógicamente del bus.
 *       Error-passive (TXEP/RXEP=1) indica que el nodo limita su señalización de errores.
 *
 * @par Ejemplo
 * @code
 * can_error_counters_t ec;
 * CAN_DRV_GetErrorCounters(&ec);
 * if (ec.bus_off) {
 *     // realizar recuperación (e.g., forzar modo Config y volver a Normal)
 * }
 * @endcode
 */

can_status_t CAN_DRV_GetErrorCounters(can_error_counters_t *out)
{
    if (!out) return CAN_ERR_PARAM;
    // Valida puntero de salida.

    uint8_t tec, rec, eflg;
    rdN(REG_TEC, &tec, 1);
    // Lee TEC: Transmit Error Counter (contador de errores TX).
    rdN(REG_REC, &rec, 1);
    // Lee REC: Receive Error Counter (contador de errores RX).
    rdN(REG_EFLG, &eflg, 1);
    // Lee EFLG: flags de error/estado (Error Warning, Error Passive, Bus-Off, etc.).

    out->tx_err = tec;
    // Copia TEC al resultado.
    out->rx_err = rec;
    // Copia REC al resultado.
    out->bus_off = (eflg & EFLG_TXBO)!=0;
    // Marca bus-off si el bit TXBO está activo.
    out->error_passive = ((eflg & (EFLG_TXEP|EFLG_RXEP))!=0);
    // Marca error-passive si cualquiera de TXEP o RXEP está activo.
    return CAN_OK;
    // Devuelve OK con los contadores/flags cargados.
}


/* ------------------------------------------------------------
 *   Acceso de bajo nivel para debug
 * ------------------------------------------------------------ */
can_status_t CAN_DRV_ReadReg(uint8_t addr, uint8_t *val)
{
    if (!val) return CAN_ERR_PARAM;
    rdN(addr, val, 1);
    return CAN_OK;
}
can_status_t CAN_DRV_WriteReg(uint8_t addr, uint8_t val)
{
    wr8(addr, val);
    return CAN_OK;
}
can_status_t CAN_DRV_BitModify(uint8_t addr, uint8_t mask, uint8_t data)
{
    bit_modify(addr, mask, data);
    return CAN_OK;
}
