/*
 * mcuDisplay.c
 *
 * Implementación del driver para display 7 segmentos multiplexado.
 *
 * Permite mostrar hasta 4 caracteres a la vez en los dígitos D0..D3.
 * Soporta desplazamiento de texto (modo MOVE) o selección fija de posición.
 *
 *  Created on: 1 sep. 2025
 *  Author: Gonzalo Louzao
 */
#include "drv/shift_registers.h"

int display(uint8_t num, uint8_t disp, uint8_t led)
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