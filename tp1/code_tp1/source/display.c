/*
 * mcuDisplay.c
 *
 * Implementación del driver para display 7 segmentos multiplexado.
 *
 * Permite mostrar hasta 4 caracteres a la vez en los dígitos D0..D3.
 * Soporta desplazamiento de texto (modo MOVE) o selección fija de posición.
 *
 *  Created on: 1 sep. 2025
 *  Author: mGonzalo Louzao
 */
#include "drv/shift_registers.h"
#include "display.h"
#include <stdint.h>
#include <stdbool.h>


uint8_t pwm;
uint16_t serialCom;


int display_init(void)
{
	serialCom = 0;
	pwm = 0;
	return serialData_init();
}

void setPWM(uint8_t desired_pwm)
{
	pwm = desired_pwm %4;
}

void display(unsigned int number)
{
	int i, j, aux = number, length = 0;

	//Counts numbers' length (quantity of digits)
	while(aux)
	{
		aux = aux/10;
		length++;
	}

	aux = number;
	for(i = 0; i < length; i++)
	{
		//Splits 'ON' time into 4 pieces
		for(j = 0; j < 4; j ++)
		{
			if(j < pwm) // turns led (PWM/4 * 100)% of the time
			{
				displayDigit(aux % 10, i % 4);
			}
			else
			{
				dispClear();
			}
		}
	}
}
void dispClear(void)
{
	serialCom &= 0x3000;
	sendSerialData(serialCom);
	return;
}

bool turnOnLED(uint8_t led)
{
	if(led>3)
	{
		return 0; //Invalid led
	}
	switch(led)
	{
		case 0: serialCom |= 0x1000 ; break;
		case 1: serialCom |= 0x2000 ; break;
		case 2: serialCom |= 0x3000 ; break;
	}
	sendSerialData(serialCom);
	return 1;

}

bool turnOffLED(uint8_t led)
{
	if(led>3)
		{
			return 0; //Invalid led
		}
		switch(led)
		{
			case 0: serialCom &= ~0x1000 ; break;
			case 1: serialCom &= ~0x2000 ; break;
			case 2: serialCom &= ~0x3000 ; break;
		}
		sendSerialData(serialCom);
		return 1;
}
bool displayDigit(uint8_t num, uint8_t disp)
{
	//int data = bcd7Seg(num%10);
	if(num >9 || disp >3)
	{
		return 0;
	}
	const uint16_t bcd_to_7seg[10] = {
		    0x3F0, // 0
		    0x060, // 1
		    0x5B0, // 2
		    0x4F0, // 3
		    0x660, // 4
		    0x6D0, // 5
		    0x7D0, // 6
		    0x070, // 7
		    0x7F0, // 8
		    0x6F0  // 9
	};

	serialCom &= 0x3000; //Resets all bits except led ones.
	switch(disp)
	{
		case 0: serialCom |=  (bcd_to_7seg[num]);break;
		case 1: serialCom |= (0x4000 | bcd_to_7seg[num]);break;
		case 2: serialCom |= (0x8000 | bcd_to_7seg[num]);break;
		case 3: serialCom |= (0xC000 | bcd_to_7seg[num]);break;
		default: break;
	}
	sendSerialData(serialCom);

	return 1;
}
