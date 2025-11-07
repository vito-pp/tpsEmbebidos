///FACULI


#include "FTM.h"
#include "gpio.h"
#include "PORT.H"



void PWM_Init(void);
void PWM_ISR(void);

uint16_t PWM_modulus = 10000-1; // Freq = 50Meg/(16*1000) = sysclck /((pwm_modulus+1)*Prescale)
uint16_t PWM_duty    = 1000;//5000-1;

static uint32_t ic_freq;
static uint8_t ic_counter;

void IC_ISR(void);

double IC_getFrequency(void)
{
	return ic_freq;
}

uint8_t IC_getCounter(void)
{
	return ic_counter;
}
void IC_clearCounter(void)
{
	ic_counter = 0;
}
__ISR__ FTM3_IRQHandler(void)
{
	IC_ISR();
}

void IC_ISR(void) //FTM3 CH5 PTC9 as IC
{
	//gpioToggle(PORTNUM2PIN(PB,2));
	//gpioToggle(PORTNUM2PIN(PB,3));
	static uint32_t med1,med2,med;
	static uint8_t  state=0;
	static int freqs[1000];
	static int i = 0;
	FTM_ClearInterruptFlag (FTM3, FTM_CH_5);
	uint32_t freq = 0;

	if(state==0)
	{
		med1=FTM_GetCounter (FTM3, FTM_CH_5); //
		state=1;
	}
	else if(state==1)
	{
		med2=FTM_GetCounter (FTM3, FTM_CH_5);
		if(FTM3->SC & FTM_SC_TOF_MASK) //overflow
		{
			FTM_ClearOverflowFlag(FTM3);
			if ((med2 < med1)) //ignor if med1 < med2
			{
				med2 += 0xFFFF;
			}

		}

		med=med2-med1;

		freq = (int)(50e6/16.0)/(2.0*med);/// BusClock=sysclk/2= 50MHz
		state=0;



		if(freq > 2000 && freq < 2700)
		{
			ic_freq= 2200;
		}
		else if(freq > 900&& freq< 1500)
		{
			ic_freq= 1200;
		}
		else
		{
			//gpioWrite(PORTNUM2PIN(PB,2), 1);
			//int n = 100000;
			//while(n--);
			//ic_freq = 0;

			freqs[i] = freq; //aparentemente solo erra por OF

			if(i == 999)
			{
				i = 0;
			}
			i++;

			//gpioToggle(PORTNUM2PIN(PB,2));

		}
	}

	//gpioToggle(PORTNUM2PIN(PB,3));
	//gpioToggle(PORTNUM2PIN(PB,2));

}


__ISR__ FTM0_IRQHandler(void)
{
	FTM_ClearOverflowFlag (FTM0);
}

void PWM_setDuty(char duty)
{
	if (duty>100)
	{
		return;
	}
	else if(duty < 0)
	{
		return;
	}

	PWM_duty = (uint16_t) (duty * PWM_modulus/100.0);
	FTM_SetCounter(FTM0, 0, PWM_duty);  //change DC
}



void FTM_Init (void)
{
	SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;
	SIM->SCGC6 |= SIM_SCGC6_FTM1_MASK;
	SIM->SCGC6 |= SIM_SCGC6_FTM2_MASK;
	SIM->SCGC3 |= SIM_SCGC3_FTM2_MASK;
	SIM->SCGC3 |= SIM_SCGC3_FTM3_MASK;

	NVIC_EnableIRQ(FTM0_IRQn);
	NVIC_EnableIRQ(FTM1_IRQn);
	NVIC_EnableIRQ(FTM2_IRQn);
	NVIC_EnableIRQ(FTM3_IRQn);

	FTM0->PWMLOAD = FTM_PWMLOAD_LDOK_MASK | 0x0F;
	FTM1->PWMLOAD = FTM_PWMLOAD_LDOK_MASK | 0x0F;
	FTM2->PWMLOAD = FTM_PWMLOAD_LDOK_MASK | 0x0F;
	FTM3->PWMLOAD = FTM_PWMLOAD_LDOK_MASK | 0x0F;

	PWM_Init();
	IC_Init();
}


void IC_Init (void)
{
	PCRstr UserPCR;
	/*// PTC8 as GPIO
	UserPCR.PCR=false;			// Default All false, Set only those needed

	UserPCR.FIELD.DSE=true;
	UserPCR.FIELD.MUX=PORT_mGPIO;
	UserPCR.FIELD.IRQC=PORT_eDisabled;

	PORT_Configure2 (PORTC,8,UserPCR);

	GPIO_SetDirection(PTC, 8, GPIO__OUT);*/


	// PTC9 as IC (FTM3-CH5)
	UserPCR.PCR=false;			// Default All false, Set only those needed

	UserPCR.FIELD.DSE=true;
	UserPCR.FIELD.MUX=PORT_mAlt3;
	UserPCR.FIELD.IRQC=PORT_eDisabled;

	PORTC->PCR[9]=UserPCR.PCR ;

	/// BusClock=sysclk/2= 50MHz
	/// Set prescaler = divx32 => Timer Clock = 32 x (1/BusClock)= 32x1/50MHz= 0.64 useg
	//--- medidor

	FTM_SetPrescaler(FTM3, FTM_PSC_x16);	 				// Set Prescaler = divx32
	FTM3->CNTIN=0x0000;				  		  				// Free running
	FTM3->MOD=0xFFFF;
	FTM_SetWorkingMode(FTM3,FTM_CH_5,FTM_mInputCapture);   // Select IC Function
	FTM_SetInputCaptureEdge (FTM3, FTM_CH_5,FTM_eEither);  // Capture on both edges or rising
	FTM_SetInterruptMode (FTM3,FTM_CH_5, true);            // Enable interrupts
	FTM_StartClock(FTM3);                                  // Select BusClk
}

/// FTM PWM Example

// To Test Connect PC9(IC)-PC8(GPIO)
// or PC9(IC)-PC1(OC)

void PWM_Init (void)
{
		PCRstr UserPCR;

		UserPCR.PCR=0;			// Default All false, Set only those needed

		UserPCR.FIELD.DSE=1;
		UserPCR.FIELD.MUX=0x4;
		UserPCR.FIELD.IRQC=0x0;

        //FTM0_CH0 PC1
        PORTC->PCR[1]=UserPCR.PCR ;

        FTM_SetPrescaler(FTM0, FTM_PSC_x16);
        FTM_SetModulus(FTM0, PWM_modulus);
       // FTM_SetOverflowMode(FTM0, true);
        FTM_SetWorkingMode(FTM0, 0, FTM_mPulseWidthModulation);			// MSA  / B
        FTM_SetPulseWidthModulationLogic(FTM0, 0, FTM_lAssertedHigh);   // ELSA / B
        FTM_SetCounter(FTM0, 0, PWM_duty);
        FTM_StartClock(FTM0);

}




// Setters

void FTM_SetPrescaler (FTM_t ftm, FTM_Prescal_t data)
{
	ftm->SC = (ftm->SC & ~FTM_SC_PS_MASK) | FTM_SC_PS(data);
}

void FTM_SetModulus (FTM_t ftm, FTMData_t data)
{
	ftm->CNTIN = 0X00;
	ftm->CNT = 0X00;
	ftm->MOD = FTM_MOD_MOD(data);
}

FTMData_t FTM_GetModulus (FTM_t ftm)
{
	return ftm->MOD & FTM_MOD_MOD_MASK;
}

void FTM_StartClock (FTM_t ftm)
{
	ftm->SC |= FTM_SC_CLKS(0x01);
}

void FTM_StopClock (FTM_t ftm)
{
	ftm->SC &= ~FTM_SC_CLKS(0x01);
}

void FTM_SetOverflowMode (FTM_t ftm, bool mode)
{
	ftm->SC = (ftm->SC & ~FTM_SC_TOIE_MASK) | FTM_SC_TOIE(mode);
}

bool FTM_IsOverflowPending (FTM_t ftm)
{
	return ftm->SC & FTM_SC_TOF_MASK;
}

void FTM_ClearOverflowFlag (FTM_t ftm)
{
	ftm->SC &= ~FTM_SC_TOF_MASK;
}

void FTM_SetWorkingMode (FTM_t ftm, FTMChannel_t channel, FTMMode_t mode)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~(FTM_CnSC_MSB_MASK | FTM_CnSC_MSA_MASK)) |
			                      (FTM_CnSC_MSB((mode >> 1) & 0X01) | FTM_CnSC_MSA((mode >> 0) & 0X01));
}

FTMMode_t FTM_GetWorkingMode (FTM_t ftm, FTMChannel_t channel)
{
	return (ftm->CONTROLS[channel].CnSC & (FTM_CnSC_MSB_MASK | FTM_CnSC_MSA_MASK)) >> FTM_CnSC_MSA_SHIFT;
}

void FTM_SetInputCaptureEdge (FTM_t ftm, FTMChannel_t channel, FTMEdge_t edge)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~(FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) |
				                  (FTM_CnSC_ELSB((edge >> 1) & 0X01) | FTM_CnSC_ELSA((edge >> 0) & 0X01));
}

FTMEdge_t FTM_GetInputCaptureEdge (FTM_t ftm, FTMChannel_t channel)
{
	return (ftm->CONTROLS[channel].CnSC & (FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) >> FTM_CnSC_ELSA_SHIFT;
}

void FTM_SetOutputCompareEffect (FTM_t ftm, FTMChannel_t channel, FTMEffect_t effect)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~(FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) |
				                  (FTM_CnSC_ELSB((effect >> 1) & 0X01) | FTM_CnSC_ELSA((effect >> 0) & 0X01));
}

FTMEffect_t FTM_GetOutputCompareEffect (FTM_t ftm, FTMChannel_t channel)
{
	return (ftm->CONTROLS[channel].CnSC & (FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) >> FTM_CnSC_ELSA_SHIFT;
}

void FTM_SetPulseWidthModulationLogic (FTM_t ftm, FTMChannel_t channel, FTMLogic_t logic)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~(FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) |
				                  (FTM_CnSC_ELSB((logic >> 1) & 0X01) | FTM_CnSC_ELSA((logic >> 0) & 0X01));
}

FTMLogic_t FTM_GetPulseWidthModulationLogic (FTM_t ftm, FTMChannel_t channel)
{
	return (ftm->CONTROLS[channel].CnSC & (FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) >> FTM_CnSC_ELSA_SHIFT;
}

void FTM_SetCounter (FTM_t ftm, FTMChannel_t channel, FTMData_t data)
{
	ftm->CONTROLS[channel].CnV = FTM_CnV_VAL(data);
}

FTMData_t FTM_GetCounter (FTM_t ftm, FTMChannel_t channel)
{
	return ftm->CONTROLS[channel].CnV & FTM_CnV_VAL_MASK;
}

void FTM_SetInterruptMode (FTM_t ftm, FTMChannel_t channel, bool mode)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~FTM_CnSC_CHIE_MASK) | FTM_CnSC_CHIE(mode);
}

bool FTM_IsInterruptPending (FTM_t ftm, FTMChannel_t channel)
{
	return ftm->CONTROLS[channel].CnSC & FTM_CnSC_CHF_MASK;
}

void FTM_ClearInterruptFlag (FTM_t ftm, FTMChannel_t channel)
{
	ftm->CONTROLS[channel].CnSC &= ~FTM_CnSC_CHF_MASK;
}

