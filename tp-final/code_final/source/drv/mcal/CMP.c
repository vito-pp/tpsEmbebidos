
#include "board.h"
#include"gpio.h"
#include"CMP.h"
#include"MK64F12.h"

void CMP_Init(void)
{
	SIM->SCGC4 |= SIM_SCGC4_CMP_MASK;
    //check filter cnt
    //hyst: 0 --> 5mV, 1 --> 10mV, 2--> 20mV , 3 --> 30mV
	CMP0->CR0 = CMP_CR0_FILTER_CNT(7) | CMP_CR0_HYSTCTR(3);

    
    //check SE, COS = 1 avoids filter, check ope
    CMP0->CR1 = CMP_CR1_SE(1) | CMP_CR1_PMODE(1) |
    			CMP_CR1_WE(0)  |
                CMP_CR1_INV(0) | CMP_CR1_COS(1) |
                CMP_CR1_EN_MASK | CMP_CR1_OPE_MASK ;
    CMP0->FPR = 1; // bus-cycles units  estaria bueno q sea congruente con adc
    //might need to enable dMA later
    CMP0->SCR = CMP_SCR_DMAEN(0) | CMP_SCR_IER(0);

    //uses vin1 (vdd)
    //DACO = (V in /64) * (VOSEL[5:0] + 1) , so the DACO range is from V in /64 to V in .
    CMP0->DACCR = CMP_DACCR_VRSEL(1) |CMP_DACCR_DACEN(1) 
                | CMP_DACCR_VOSEL(31); //need to select comparation level

    //Compares CMP0_IN1 to CMP0_IN2
    CMP0->MUXCR = CMP_MUXCR_PSEL(1) | CMP_MUXCR_MSEL(7); //2

    //FILTERS ON PAGE 913 Table 36-1

    PORTC->PCR[PIN2NUM(CMP0_OUT)] = PORT_PCR_ISF_MASK | PORT_PCR_MUX(ALT6);
    PORTC->PCR[PIN2NUM(CMP0_IN1)] = PORT_PCR_ISF_MASK | PORT_PCR_MUX(ALT0);
    //PORTC->PCR[PIN2NUM(CMP0_IN2)] = PORT_PCR_ISF_MASK | PORT_PCR_MUX(ALT0);
}
