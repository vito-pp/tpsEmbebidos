/***************************************************************************//**
  @file     App.c
  @brief    Test simple: enviar el mismo mensaje continuamente por UART0
*******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "drv/hal/timer.h"
#include "drv/mcal/board.h"
#include "drv/mcal/gpio.h"
#include "drv/mcal/SysTick.h"
#include "drv/mcal/UART_strings.h"
#include "drv/mcal/UART.h"
#include "drv/mcal/dma.h"
#include "drv/mcal/FTM.h"
#include "drv/mcal/CMP.h"
#include "drv/mcal/pit.h"
#include "drv/mcal/DECODE_V2.h"
#include "drv/mcal/bitstream.h"


/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/
uint8_t bit_stream[11];
uint16_t data_stream[20];
bool finished = 0;
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
bool finishStatus(void)
{
	return finished;
}
void finishedReading(void)
{
	finished=1;
}
void clearReading(void)
{
	finished = 0;
}
static void ftm_cb(void* user);
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	CMP_Init();
	FTM_Init();
	PIT_Init();
	gpioMode(PORTNUM2PIN(PB,2), OUTPUT);
	gpioWrite(PORTNUM2PIN(PB,2), 0);
	gpioMode(PORTNUM2PIN(PB,3), OUTPUT);
	gpioWrite(PORTNUM2PIN(PB,3), 0);

	pit_cfg_t pit_cfg =
 	 {
		.ch = 0,
		.load_val = PIT_TICKS_FROM_US(830),
		.periodic = true,
		.int_en = true,
		.callback = ftm_cb,
		.user = NULL
 	 };
	PIT_Config(&pit_cfg);
	PIT_Stop(0);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
  if (bitStartDetected())
  {

	  gpioToggle(PORTNUM2PIN(PB,2));
	  setReadingFlag();
	  clearFinished();
	  //gpioToggle(PORTNUM2PIN(PB,3));
	  PIT_Start(0);
	  //gpioToggle(PORTNUM2PIN(PB,3));
  }
  if(finishStatus())
  {
	  bit_stream[0] += 0;
  }

  //PWM_setDuty(50);
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

static void ftm_cb(void* user)
{
	gpioToggle(PORTNUM2PIN(PB,3));
	static uint8_t cnt = 1;
	bit_stream[cnt]  = processBit();
/*/////////////////////////////////////////
	static uint8_t cnt = 0;
	static int i=0;
	data_stream[i] = data_stream[i]<< cnt | processBit() ;
*/////////////////////////////////////////
	cnt++;
	if (cnt == 11)//|| (bit_stream[i][cnt-1] == 2)) //bit[cnt-1 ]== 2 => error
	{
		PIT_Stop(0);
		//DELAY
		//int n = 100000;
		//while(n--);
		cnt = 1;
		/////////
		//i++;
		//data_stream[i]= 0;
		//cnt= 0;
		//////////
		clearReadingFlag();
		finishedReading();
		IC_clearBitStart();
		gpioToggle(PORTNUM2PIN(PB,2));
	}
	gpioToggle(PORTNUM2PIN(PB,3));
	/*if(i == 10)
	{
		i= 0;
	}*/



}

