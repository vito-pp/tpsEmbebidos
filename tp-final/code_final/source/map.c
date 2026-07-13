/*
 * map.c
 *
 *  Created on: 12 jul. 2026
 *      Author: Usuario
 */

#include "drv/hal/matrix.h"
#include"map.h"

#define FLOORS 3


uint8_t getOcupation(uint8_t floor);


uint8_t getOcupation(uint8_t floor)
{
	int n = 0;
	switch(floor)
	{
	case 1: n = 4; break;
	case 2: n = 2; break;
	case 3: n = 3; break;
	default: break;
	}
	return n;
}


void loadMap(void)
{
    int i;
    char id;
    char colour[64];

    for(i= 0; i < 64; i++)
    {
        colour[i] = 0;
    }
    for(i = 0; i < FLOORS; i++)
    {
        uint8_t n = getOcupation(i+1);

        while(n)
        {
            switch(n)
            {
                case 1: id = 'r'; break;
                case 2: id = 'b'; break;
                case 3: id = 'g'; break;
                case 4: id = 'p'; break;
                default: id = 0; break;
            }
            colour[(2*(4-n))*8 + i*2] = id;
            colour[(2*(4-n))*8 + i*2 + 1] = id;

            colour[(2*(4-n) + 1)*8 + i*2] = id;
            colour[(2*(4-n) + 1)*8 + i*2 + 1] = id;
            n--;
        }
    }

    displayMatrix(colour, 7, 64);

    return;
}
