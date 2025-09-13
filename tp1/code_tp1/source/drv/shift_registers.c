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
#define LCLK_ACTIVE LOW
#define CLK_ACTIVE HIGH

int serialData_init(void)
{
	static int already_initialized = 0;
	if(already_initialized)
	{
		return 0;
	}
	gpioMode(SR_DATA, OUTPUT);
	gpioMode(SR_LCLK, OUTPUT);
	gpioMode(SR_CLK, OUTPUT);

	already_initialized = 1;
	return 1;
}

void sendSerialData(uint16_t data)
{
	int i;
	int contador;


	gpioWrite(SR_CLK, !CLK_ACTIVE);
	gpioWrite(SR_LCLK, LCLK_ACTIVE);

	i = DATA_SIZE;
	while(i--)
	{

		gpioWrite(SR_CLK, !CLK_ACTIVE);
		gpioWrite(SR_DATA, data & 0b1);

		gpioWrite(SR_CLK, CLK_ACTIVE);

		data = data >> 1;
	}
	gpioWrite(SR_CLK, !CLK_ACTIVE);
	gpioWrite(SR_LCLK, !LCLK_ACTIVE);
	data = 0;
	return;
}
