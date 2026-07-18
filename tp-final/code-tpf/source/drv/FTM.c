#include "drv/FTM.h"

void FTM_Init(void)
{
    /*
     * Enable PORTD clock for PTD0.
     * Enable FTM3 clock.
     */
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
    SIM->SCGC3 |= SIM_SCGC3_FTM3_MASK;

    /*
     * PTD0 = FTM3_CH0.
     * MUX = 4 selects FTM3_CH0 on PTD0.
     * DSE = high drive strength.
     */
    PORTD->PCR[0] = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK;

    /*
     * Disable write protection.
     */
    FTM3->MODE |= FTM_MODE_WPDIS_MASK;

    /*
     * Stop timer while configuring.
     */
    FTM3->SC = 0;

    /*
     * Edge-aligned PWM.
     */
    FTM3->CNTIN = 0;
    FTM3->CNT = 0;
    FTM3->MOD = WS2812_PWM_MOD;

    /*
     * FTM3_CH0:
     * MSB:MSA = 10 -> edge-aligned PWM
     * ELSB:ELSA = 10 -> high-true pulses
     * DMA = 1 -> channel event triggers DMA request
     *
     * Do NOT enable CHIE here.
     * We want DMA requests, not a CPU interrupt at the WS2812 bit rate.
     */
    FTM3->CONTROLS[0].CnSC =
        FTM_CnSC_MSB_MASK |
        FTM_CnSC_ELSB_MASK |
        FTM_CnSC_DMA_MASK;

    /*
     * Initial duty.
     */
    FTM3->CONTROLS[0].CnV = DC2CNV(0);

    /*
     * Prescaler = 1.
     * CLKS = 01 -> system/bus clock.
     */
    FTM3->SC = FTM_SC_PS(0) | FTM_SC_CLKS(1);
}

void PWM_setDuty(uint8_t duty_percent)
{
    if (duty_percent > 100u) {
        duty_percent = 100u;
    }

    FTM3->CONTROLS[0].CnV = DC2CNV(duty_percent);
}
