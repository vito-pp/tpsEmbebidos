
#include "board.h"
#include"gpio.h"
#include"CMP.h"


void CMP_Init(void)
{
    //check filter cnt
	CMP0.CR0 = CMP_CR0_FILTER_CNT(5) | CMP_CR0_HYSTCTR(0);
    
    //check SE, COS = 1 avoids filter, check ope
    CMP0.CR1 = CMP_CR1_SE(1) | CMP_CR1_PMODE(1) |
                CMP_CR1_INV(0) | CMP_CR1_COS(1) |
                CMP_CR1_EN_MASK;
    CMP0.FPR = 4; // bus-cycles units
    //might need to enable dMA later
    CMP0.SCR = CMP_SCR_DMAEN(0) | CMP_SCR_IER(0);

    //uses vin1 as vref if DACEN  = 1
    CMP0.DACCR = CMP_DACCR_DACEN(0) | CMP_DACCR_VOSEL(31); //need to select comparation level

    //Compares CMP0_IN1 to CMP0_IN2
    CMP0.MUXCR = CMP_MUXCR_PSEL(1) | CMP_MUXCR_MSEL(2); 

    //FILTERS ON PAGE 913 Table 36-1

    PORTC->PCR[PIN2NUM(CMP0_OUT)] = PORT_PCR_ISF_MASK | PORT_PCR_MUX(ALT6);
    PORTC->PCR[PIN2NUM(CMP0_IN1)] = PORT_PCR_ISF_MASK | PORT_PCR_MUX(ALT0);
    PORTC->PCR[PIN2NUM(CMP0_IN2)] = PORT_PCR_ISF_MASK | PORT_PCR_MUX(ALT0);
}
