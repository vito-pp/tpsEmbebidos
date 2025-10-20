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
#include "drv/can.h"


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
	CAN_Init();
	timerInit();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	tim_id_t id = timerGetId();
	timerStart(id, 10000000, TIM_MODE_PERIODIC, SPI0_pushTxFIFO);

	timerUpdate();

	/*if(SPI0_isTxQueueEmpty())
	{
		int ku = 0;
		uint8_t data1[] = {0xFF, 0x00, 0x0F, 0x0F, 0xF0, 0xF0, 0b10101010,  0b10101010};
		//SPI0_sendNBytes(data1, 7);
		SPI0_sendNBytes(data1, 8);
		SPI0_pushTxFIFO();
	}*/

	uint8_t data[2] = {0b10101010, 0b11001100};

	uint8_t read_data[16];
	if(CAN_rxStatus())
	{
		CAN_readData(data);
	}
	CAN_sendData(data, 2);


	static uint16_t rx_buffer[30];

	uint8_t read[16];
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
