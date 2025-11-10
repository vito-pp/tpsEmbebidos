/*
 * DECODE_V2.c
 *
 *  Created on: 4 nov. 2025
 *      Author: Facundo Juli
 */

#include "drv/mcal/DECODE_V2.h"
#include "drv/mcal/FTM.h"

#define CONT_MAX 12 
static bool reading = 0;

void clearReadingFlag(void) //Should be called when reading finished
{
	reading = 0;
}

void setReadingFlag(void)
{
	reading = 1;
}
bool getReadingFlag(void)
{
	return reading;
}
//Returns 2 if error
uint8_t processBit(void)
{
	uint8_t bit;
	int freq = (int) IC_getFrequency();
	/*static int i = 0;
	static int buffer[1000];
	buffer[i] = freq;
	if(i == 999)
	{
		i = 0;
	}
	i++;
	*/
	switch(freq)
	{
	case 1200: bit = 1; break;
	case 2200: bit = 0; break;
	default: bit = 2; break; // ERROR
	} 
	return bit;//bit;
}


bool bitStartDetected(void)
{
	if((IC_getBitStart()==2) && !reading)
	{
		IC_clearBitStart();
		return 1;
	}
	return 0;
	//se detectó un bit start
	//Trigger timer Tt < ~ 833 us  / (833us -  Tt)* Nbits < 833 us
	// 833us(1-1/Nbits) < Tt < 833us
	/*static uint8_t prev = 3;
	uint8_t current = processBit();

	if((current == 0) && (current == prev) &&  !reading) // requires 2 consecutive samples of a 0, to trigger better.
	{
		prev = 3;
		return 1;
	}
	prev = current;
	return 0;
    */
}
