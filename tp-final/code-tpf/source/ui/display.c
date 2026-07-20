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
#include <stdint.h>
#include <stdbool.h>

#include "../drv/shift_registers.h"
#include "display.h"

#define CANT_DISPLAYS 4
#define NONE 11

typedef enum
{
    DISPLAY_MODE_CLEAR,
    DISPLAY_MODE_NUMBER,
    DISPLAY_MODE_HYPHENS
} display_mode_t;

static uint8_t pwm;
static uint16_t serialCom;
static display_mode_t display_mode;
static unsigned int display_number;
static bool display_hide;
static uint8_t display_length;

/**
 * @brief Writes character to indexed display. (0-9, '-' or none)
 * @param num: character to be written
 * @param disp: index of display for number to be displayed (0,1,2,3)
 * @return TRUE: correctly displayed character
 * @return FALSE: invalid character
 */
bool displayDigit(uint8_t num, uint8_t disp);
static void displayClearHardware(void);

int display_init(void)
{
	serialCom = 0;
	pwm = 0;
	display_mode = DISPLAY_MODE_CLEAR;
	return serialData_init();
}

void setPWM(uint8_t desired_pwm)
{
	pwm = desired_pwm % BRIGHTNESS_LEVELS;
}

void display(unsigned int number, bool hide, uint8_t lenght)
{
	display_number = number;
	display_hide = hide;
	display_length = lenght;
	display_mode = DISPLAY_MODE_NUMBER;
}

void displayHyphens(void)
{
	display_mode = DISPLAY_MODE_HYPHENS;
}

void dispClear(void)
{
	display_mode = DISPLAY_MODE_CLEAR;
	displayClearHardware();
}

void displayRefresh(void)
{
	int i;
	int j;
	int current_digit;
	unsigned int number = display_number;
	bool hide = display_hide;
	uint8_t lenght = display_length;

	if (display_mode == DISPLAY_MODE_CLEAR)
	{
		displayClearHardware();
		return;
	}

	if (display_mode == DISPLAY_MODE_HYPHENS)
	{
		for(i = 0; i < CANT_DISPLAYS; i++)
		{
			for(j = 0; j < BRIGHTNESS_LEVELS; j++)
			{
				if(j <= pwm)
				{
					displayDigit(HYPHEN, 3 - i);
				}
				else
				{
					displayClearHardware();
				}
			}
		}
		return;
	}

	current_digit = number % 10;
	for(i = 0; i < CANT_DISPLAYS; i++)
	{
		if(i > 0)
		{
			if(number == 0 && i > lenght)
			{
				current_digit = NONE;
			}
			else if(hide)
			{
				current_digit = HYPHEN;
			}
			else
			{
				current_digit = number % 10;
			}
		}

		for(j = 0; j < BRIGHTNESS_LEVELS; j++)
		{
			if(j <= pwm)
			{
				displayDigit(current_digit, 3 - i);
			}
			else
			{
				displayClearHardware();
			}
		}
		number /= 10;
	}
}

static void displayClearHardware(void)
{
	serialCom &= 0x00C0; // keep digit select (SEL1:SEL0), blank all segments
	sendSerialData(serialCom);
}

bool turnOnLED(uint8_t led)
{
	led--;
	if(led >= 3)
	{
		return 0; //Invalid led
	}

	turnOffLEDs();

	switch(led)
	{
		// Custom board: shift register has no status LEDs (U5 QC-QH unused),
		// and these bits would collide with segments C/D. No-op on hardware.
		case 0: break;
		case 1: break;
		case 2: break;
	}
	return 1;
}


void turnOffLEDs(void)
{
	// Custom board: no status LEDs on the shift register. No-op.
}

bool displayDigit(uint8_t num, uint8_t disp)
{
	if(num > NONE || disp >= CANT_DISPLAYS)
	{
		return 0;
	}
	const uint16_t bcd_to_7seg[] = {
		    0xFC00, // 0  A B C D E F
		    0x6000, // 1  B C
		    0xDA00, // 2  A B G E D
		    0xF200, // 3  A B G C D
		    0x6600, // 4  F G B C
		    0xB600, // 5  A F G C D
		    0xBE00, // 6  A F G E D C
		    0xE000, // 7  A B C
		    0xFE00, // 8  A B C D E F G
		    0xF600, // 9  A B C D F G
			0x0200, // -  G
			0x0000  // none
	};

	serialCom = 0x0000; // rebuild: segments (b8-15) + SEL1:SEL0 (b7:6)
	switch(disp)
	{
		case 0: serialCom |=  (bcd_to_7seg[num]);break;
		case 1: serialCom |= (0x0040 | bcd_to_7seg[num]);break; // SEL0
		case 2: serialCom |= (0x0080 | bcd_to_7seg[num]);break; // SEL1
		case 3: serialCom |= (0x00C0 | bcd_to_7seg[num]);break; // SEL1|SEL0
		default: break;
	}
	sendSerialData(serialCom);

	return 1;
}

// // funcion que hizo gonza recien
// static void shiftDisplay(void)
// {
//     static int last = -1;
//     static int offset = 0;

//     uint64_t i = getNumber();

//     // Count decimal digits (0 -> 1 digit)
//     int aux = i;
//     int length = 0;
//     while(aux)
//     {
//         aux = aux/10;
//         length++;
//     }

//     // Reset scroll if number changed or fits in 4
//    /* if (i != last || length <= 4) 
//     {
//         last = i;
//         offset = 0;
//     }*/

//     if (length <= 4) 
//     {
//         offset = 0;
//         display(i);
//         return;
//     }

//     // Compute 10^(length - (offset + 4))
//     int shift = length - (offset + 4);
//     int div = 1;
//     while (shift-- > 0) 
//     {
//     div *= 10;
//     }
//     int window = (i / div) % 10000;  // take the 4-digit slice
//     display(window);

//     // Advance and wrap offset
//     offset++;
//     if (offset > length - 4) 
//     {
//         offset = 0;
//     }
// }
