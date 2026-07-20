/*
 * map.c
 *
 *  Created on: 12 jul. 2026
 *      Author: Usuario
 */

#include "../drv/matrix.h"
#include "../ui/map.h"

#define FLOORS 3

static bool error[4];

static uint8_t occupants[FLOORS];

static uint8_t intensity = 7;

static uint8_t getOcupation(uint8_t floor);

static uint8_t getOcupation(uint8_t floor)
{
    floor--;
    if (floor >= FLOORS)
    {
        return 0;
    }
	return occupants[floor];
}

void setOcupation(uint8_t floor, uint8_t n)
{
    floor--;
    if(n > 4 || floor >= FLOORS)
    {
        return;
    }
    occupants[floor] = n;
    return;
}

void setErrorX(uint8_t x)
{
    x--;
    if(x>3)
    {
        return;
    }
    error[x] = true;
    return;
}

void clearErrorX(uint8_t x)
{
    x--;
    if(x>3)
    {
        return;
    }
    error[x] = false;
    return;
}


void setIntensity(uint8_t n)
{
    if(n > 7)
    {
        return;
    }
    intensity = n;
    return;
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
    
    if(error[0])
    {
        colour[54] = 'g';
        colour[55] = 'g';
        colour[62] = 'g';
        colour[63] = 'g';

    }
    if(error[1])
    {
        colour[38] = 'r';
        colour[39] = 'r';
        colour[46] = 'r';
        colour[47] = 'r';
    }
    if(error[2])
    {
        colour[22] = 'b';
        colour[23] = 'b';
        colour[30] = 'b';
        colour[31] = 'b';
    }
    if(error[3])
    {
        colour[6] = 'r';
        colour[7] = 'r';
        colour[14] = 'r';
        colour[15] = 'r';
    }

    displayMatrix(colour, intensity, 64);

    return;
}
