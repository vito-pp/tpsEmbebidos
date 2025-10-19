/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   todes
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>

#include "drv/board.h"
#include "drv/gpio.h"
#include "misc/timer.h"
#include "drv/SysTick.h"
#include "drv/spi.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    SPI0Master_Init();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{


	if(SPI0_isTxQueueEmpty())
	{
		SPI0_send3Bytes(1, 2, 3);
		SPI0_PushTxRx_IRQ();

		SPI0_send4Bytes(4, 5, 6, 7);
		SPI0_PushTxRx_IRQ();

		SPI0_send2Bytes(8, 9);
		SPI0_PushTxRx_IRQ();

		SPI0_send2Bytes(10, 11);
		SPI0_PushTxRx_IRQ();

		SPI0_sendByte(12);
		SPI0_PushTxRx_IRQ();

		SPI0_send4Bytes(21, 22, 23, 24);
		SPI0_PushTxRx_IRQ();

		SPI0_send3Bytes(25, 26, 27);
		SPI0_PushTxRx_IRQ();

		SPI0_send2Bytes(28, 29);
		SPI0_PushTxRx_IRQ();
	}

	static uint16_t rx_buffer[30];

	static int i = 0;
	uint16_t aux;
	for(;;)
	{
		if( (aux = SPI0_PopRxFIFO() ) != 0xFFFF)
		{
			rx_buffer[i] = aux;
			i = (i+1)%30;
		}
		else
		{
			break;
		}
	}
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
