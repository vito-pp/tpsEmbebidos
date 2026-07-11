/***************************************************************************//**
  @file     App.c
  @brief    Test simple: enviar el mismo mensaje continuamente por UART0
*******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <drv/mcal/dma.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "drv/hal/timer.h"
#include "drv/hal/NCO.h"
#include "drv/mcal/board.h"
#include "drv/mcal/gpio.h"
#include "drv/mcal/SysTick.h"
#include "drv/mcal/UART_strings.h"
#include "drv/mcal/UART.h"
#include "drv/mcal/FTM.h"
#include "drv/mcal/CMP.h"
#include "drv/mcal/pit.h"
#include "drv/mcal/DAC.h"
#include "drv/mcal/DECODE_V2.h"
#include "drv/mcal/bitstream.h"
#include "drv/mcal/matStream.h"
#include "drv/hal/matrix.h"

extern uint8_t sendingDMA;

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	CMP_Init();
	FTM_Init();

	//PIT_Init();
	dispBus_init();

	gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
	gpioMode(PORTNUM2PIN(PB,2), OUTPUT);
	gpioWrite(PORTNUM2PIN(PB,2), 0);

	gpioMode(PORTNUM2PIN(PB,3), OUTPUT);
	gpioWrite(PORTNUM2PIN(PB,3), 0);


	//DMA_Start(0);

}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	static unsigned long int i = 0;
	//DMA0->SSRT = DMA_SSRT_SSRT(0);
	//FTM0->CONTROLS[0].CnSC &= ~FTM_CnSC_CHF_MASK;
	char display[64] = {
		    'g','g','g','g','g','g','g','g',
		    'r','r','r','r','r','r','r','r',
		    'b','b','b','b','b','b','b','b',
		    'g','g','g','g','g','g','g','g',
		    'r','r','r','r','r','r','r','r',
		    'b','b','b','b','b','b','b','b',
		    'g','g','g','g','g','g','g','g',
		    'r','r','r','r','r','r','r','r'
		    };
	if(!sendingDMA)
	{
		displayMatrix(display,7,sizeof(display));
		gpioWrite(PORTNUM2PIN(PB,3), 1);
	}

	//gpioToggle(PORTNUM2PIN(PB,3));
	//WS2812_Update();

	//PWM_setDuty(50);
	/*if(i %2)
	{
		PWM_setDuty(36); //32
	}
	else
	{
		//PWM_setDuty(68);//64);
	*/
	i++;

}

/*******************************************************************************
 *******************************************************************************
    LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void delayLoop(uint32_t veces)
{
    while (veces--);
}

static void __error_handler__(void)
{
    gpioWrite(PIN_LED_RED, LED_ACTIVE);
}
