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

#define BRIGHTNESS_LEVELS 10
#define CANT_DISPLAYS 4
#define HYPHEN 10
#define NONE 11

static uint8_t pwm;
static uint16_t serialCom;

/**
 * @brief Writes character to indexed display. (0-9, '-' or none)
 * @param num: character to be written
 * @param disp: index of display for number to be displayed (0,1,2,3)
 * @return TRUE: correctly displayed character
 * @return FALSE: invalid character
 */
static bool displayDigit(uint8_t num, uint8_t disp);

int display_init(void)
{
	serialCom = 0;
	pwm = 0;
	return serialData_init();
}

void setPWM(uint8_t desired_pwm)
{
	pwm = desired_pwm % BRIGHTNESS_LEVELS;
}

void display(unsigned int number, bool hide)
{
	int i, j;
	int current_digit = number % 10;

	for(i = 0; i < CANT_DISPLAYS; i++)
	{
		if(i > 0)
		{
			if(number == 0)
			{
				current_digit = NONE;
			}
			else if(hide)
			{
				current_digit = HYPHEN;
			}
			else
			{
				current_digit = number %10;
			}
		}
		//Splits i-th display 'ON' time into 'BRIGHTNESS_LEVELS' pieces
		for(j = 0; j < BRIGHTNESS_LEVELS; j ++)
		{
			if(j <= pwm) // turns led (pwm/BRIGHTNESS_LEVELS * 100)% of the time
			{

				displayDigit(current_digit, 3 - i);
			}
			else
			{
				dispClear();
			}
		}
		number /=10;

	}
}
void dispClear(void)
{
	serialCom &= 0x3000;
	sendSerialData(serialCom);
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
static bool displayDigit(uint8_t num, uint8_t disp)
{
	if(num > NONE || disp >= CANT_DISPLAYS)
	{
		return 0;
	}
	const uint16_t bcd_to_7seg[] = {
		    0x3F0, // 0
		    0x060, // 1
		    0x5B0, // 2
		    0x4F0, // 3
		    0x660, // 4
		    0x6D0, // 5
		    0x7D0, // 6
		    0x070, // 7
		    0x7F0, // 8
		    0x6F0, // 9
			0x400, // -
			0x000  // none
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
