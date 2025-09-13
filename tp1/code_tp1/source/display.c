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

int display_init(void)
{
	return serialData_init();
}

int display(uint8_t num, uint8_t disp)
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

	switch(disp)
	{
		case 0: sendSerialData(bcd_to_7seg[num]);break;
		case 1: sendSerialData(0x4000 | bcd_to_7seg[num]);break;
		case 2: sendSerialData(0x8000 | bcd_to_7seg[num]);break;
		case 3: sendSerialData(0xC000 | bcd_to_7seg[num]);break;
		default: break;
	}

	return 1;
}
