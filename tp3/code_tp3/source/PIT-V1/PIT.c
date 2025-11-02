#include "PIT.h"
#include "fsl_device_registers.h"

#ifndef PIT_CHANNELS
#define PIT_CHANNELS 4u
#endif

static pit_cb_t s_cb[PIT_CHANNELS] = {0};

static inline void pit_irq_enable(uint8_t ch, uint8_t prio){
    IRQn_Type irq = (IRQn_Type)(PIT0_IRQn + ch);
    NVIC_SetPriority(irq, prio);
    NVIC_EnableIRQ(irq);
}

static bool pit_ldval(uint8_t ch, uint32_t bus_hz, uint32_t freq_hz){
    if (ch >= PIT_CHANNELS || freq_hz == 0) return false;
    uint32_t ld = bus_hz / freq_hz;
    if (ld == 0) return false;
    ld -= 1u;
    PIT->CHANNEL[ch].LDVAL = PIT_LDVAL_TSV(ld);
    return true;
}

bool PIT_Init(uint8_t ch, uint32_t bus_hz, uint32_t freq_hz,
              uint8_t irq_prio, pit_cb_t cb)
{
    if (ch >= PIT_CHANNELS || cb == 0) return false;

    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
    PIT->MCR = 0; // enable module

    if (!pit_ldval(ch, bus_hz, freq_hz)) return false;

    s_cb[ch] = cb;
    PIT->CHANNEL[ch].TCTRL = PIT_TCTRL_TIE_MASK; // IRQ enable, TEN later
    pit_irq_enable(ch, irq_prio);
    return true;
}

bool PIT_SetFreq(uint8_t ch, uint32_t bus_hz, uint32_t freq_hz)
{
    bool running = (PIT->CHANNEL[ch].TCTRL & PIT_TCTRL_TEN_MASK) != 0;
    if (running) PIT_Stop(ch);
    bool ok = pit_ldval(ch, bus_hz, freq_hz);
    if (running) PIT_Start(ch);
    return ok;
}

void PIT_Start(uint8_t ch)
{
    PIT->CHANNEL[ch].TFLG = PIT_TFLG_TIF_MASK;
    PIT->CHANNEL[ch].TCTRL |= PIT_TCTRL_TEN_MASK;
}

void PIT_Stop(uint8_t ch)
{
    PIT->CHANNEL[ch].TCTRL &= ~PIT_TCTRL_TEN_MASK;
}

void PIT0_IRQHandler(void){ PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK; if (s_cb[0]) s_cb[0](); }
void PIT1_IRQHandler(void){ PIT->CHANNEL[1].TFLG = PIT_TFLG_TIF_MASK; if (s_cb[1]) s_cb[1](); }
void PIT2_IRQHandler(void){ PIT->CHANNEL[2].TFLG = PIT_TFLG_TIF_MASK; if (s_cb[2]) s_cb[2](); }
void PIT3_IRQHandler(void){ PIT->CHANNEL[3].TFLG = PIT_TFLG_TIF_MASK; if (s_cb[3]) s_cb[3](); }
