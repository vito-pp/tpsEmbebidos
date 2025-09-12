/*
 * shift_registers.c
 *
 *  Created on: 9 sep. 2025
 *      Author: Usuario
 */


#include "board.h"
#include "timer.h"
#include "gpio.h"
#include "shift_registers.h"
#include <stdint.h>

#define DATA_SIZE 16 //contains serial data buffer size in bits
#define EN_ACTIVE LOW
#define CLK_ACTIVE HIGH
#define CLOCK 10

int serialData_init(void) //may receive clk
{
	static int already_initialized = 0;
	if(already_initialized)
	{
		return 1;
	}
	gpioMode(SR_DATA, OUTPUT);
	gpioMode(SR_ENABLE, OUTPUT);
	gpioMode(SR_CLK, OUTPUT);

	already_initialized = 1;
	return 1;
}

void sendSerialData(uint16_t data)
{
	int i;
	int contador;


	gpioWrite(SR_CLK, !CLK_ACTIVE);
	gpioWrite(SR_ENABLE, EN_ACTIVE);
	//for(i = 0; i < DATA_SIZE; i++)
	i = DATA_SIZE;
	while(i--)
	{

		gpioWrite(SR_CLK, !CLK_ACTIVE);
		gpioWrite(SR_DATA, data & 0b1);
		//contador = CLOCK;
		//while(contador--);
		gpioWrite(SR_CLK, CLK_ACTIVE);
		//contador = CLOCK;
		//while(contador--);
		data = data >> 1;
	}
	gpioWrite(SR_CLK, !CLK_ACTIVE);
	gpioWrite(SR_ENABLE, !EN_ACTIVE);
	data = 0;
	return;
}
