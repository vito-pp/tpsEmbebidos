#include "drv/dma.h"

#include <stddef.h>
#include "MK64F12.h"
#include "hardware.h"

typedef struct {
    dma_cb_t on_major_cb;
    void *user_param;
    bool active;
} dma_state_t;

static bool initialized = false;
static dma_state_t dma_ch_states[DMA_NUM_CH];

static int size2code(uint8_t bytes)
{
    switch (bytes) {
    case 1:
        return 0;
    case 2:
        return 1;
    case 4:
        return 2;
    default:
        return -1;
    }
}

int DMA_Init(void)
{
    if (initialized) {
        return 0;
    }

    SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
    SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

    DMA0->CR = DMA_CR_ERCA_MASK | DMA_CR_HOE_MASK;

    DMA0->ERR = 0xFFFFFFFFu;

    initialized = true;

    return 0;
}

int DMA_Config(const dma_cfg_t *cfg)
{
    int size_code;

    if (cfg == NULL) {
        return -1;
    }

    if (cfg->ch >= DMA_NUM_CH) {
        return -2;
    }

    size_code = size2code(cfg->elem_size);
    if (size_code < 0) {
        return -3;
    }

    DMA_Init();

    /*
     * Disable this channel before changing its config.
     */
    DMA0->CERQ = DMA_CERQ_CERQ(cfg->ch);

    /*
     * Disable DMAMUX channel before reconfiguring it.
     */
    DMAMUX->CHCFG[cfg->ch] = 0;

    dma_ch_states[cfg->ch].on_major_cb = cfg->on_major;
    dma_ch_states[cfg->ch].user_param = cfg->user;
    dma_ch_states[cfg->ch].active = false;

    NVIC_ClearPendingIRQ((IRQn_Type)(DMA0_IRQn + cfg->ch));
    NVIC_EnableIRQ((IRQn_Type)(DMA0_IRQn + cfg->ch));

    DMA0->TCD[cfg->ch].SADDR = (uint32_t)cfg->saddr;
    DMA0->TCD[cfg->ch].SOFF = cfg->soff;

    DMA0->TCD[cfg->ch].ATTR =
        DMA_ATTR_SSIZE(size_code) |
        DMA_ATTR_DSIZE(size_code);

    DMA0->TCD[cfg->ch].NBYTES_MLNO = cfg->elem_size;

    DMA0->TCD[cfg->ch].SLAST = cfg->slast;

    DMA0->TCD[cfg->ch].DADDR = (uint32_t)cfg->daddr;
    DMA0->TCD[cfg->ch].DOFF = cfg->doff;
    DMA0->TCD[cfg->ch].CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(cfg->major_count);
    DMA0->TCD[cfg->ch].DLAST_SGA = cfg->dlast;
    DMA0->TCD[cfg->ch].BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(cfg->major_count);

    DMA0->TCD[cfg->ch].CSR = 0;
    if (cfg->int_major) {
        DMA0->TCD[cfg->ch].CSR |= DMA_CSR_INTMAJOR_MASK;
    }

    /*
     * Connect DMA channel to request source.
     */
    DMAMUX->CHCFG[cfg->ch] =
        DMAMUX_CHCFG_SOURCE((uint8_t)cfg->request_src) |
        DMAMUX_CHCFG_ENBL_MASK;

    return 0;
}

int DMA_Start(uint8_t ch)
{
    if (!initialized) {
        return -1;
    }

    if (ch >= DMA_NUM_CH) {
        return -2;
    }

    dma_ch_states[ch].active = true;

    DMA0->CINT = DMA_CINT_CINT(ch);
    DMA0->CERR = DMA_CERR_CERR(ch);

    DMA0->SERQ = DMA_SERQ_SERQ(ch);

    return 0;
}

int DMA_Stop(uint8_t ch)
{
    if (!initialized) {
        return -1;
    }

    if (ch >= DMA_NUM_CH) {
        return -2;
    }

    dma_ch_states[ch].active = false;

    DMA0->CERQ = DMA_CERQ_CERQ(ch);

    return 0;
}

static void DMA_IRQHandler_Common(uint8_t ch)
{
    DMA0->CINT = DMA_CINT_CINT(ch);

    if (ch < DMA_NUM_CH && dma_ch_states[ch].on_major_cb != NULL) {
        dma_ch_states[ch].on_major_cb(dma_ch_states[ch].user_param);
    }
}

__ISR__ DMA0_IRQHandler(void)
{
    DMA_IRQHandler_Common(0);
}

__ISR__ DMA1_IRQHandler(void)
{
    DMA_IRQHandler_Common(1);
}

__ISR__ DMA2_IRQHandler(void)
{
    DMA_IRQHandler_Common(2);
}

__ISR__ DMA3_IRQHandler(void)
{
    DMA_IRQHandler_Common(3);
}

__ISR__ DMA4_IRQHandler(void)
{
    DMA_IRQHandler_Common(4);
}

__ISR__ DMA5_IRQHandler(void)
{
    DMA_IRQHandler_Common(5);
}

__ISR__ DMA6_IRQHandler(void)
{
    DMA_IRQHandler_Common(6);
}

__ISR__ DMA7_IRQHandler(void)
{
    DMA_IRQHandler_Common(7);
}

__ISR__ DMA8_IRQHandler(void)
{
    DMA_IRQHandler_Common(8);
}

__ISR__ DMA9_IRQHandler(void)
{
    DMA_IRQHandler_Common(9);
}

__ISR__ DMA10_IRQHandler(void)
{
    DMA_IRQHandler_Common(10);
}

__ISR__ DMA11_IRQHandler(void)
{
    DMA_IRQHandler_Common(11);
}

__ISR__ DMA12_IRQHandler(void)
{
    DMA_IRQHandler_Common(12);
}

__ISR__ DMA13_IRQHandler(void)
{
    DMA_IRQHandler_Common(13);
}

__ISR__ DMA14_IRQHandler(void)
{
    DMA_IRQHandler_Common(14);
}

__ISR__ DMA15_IRQHandler(void)
{
    DMA_IRQHandler_Common(15);
}
