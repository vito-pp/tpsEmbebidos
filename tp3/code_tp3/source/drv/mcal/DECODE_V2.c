/*
 * DECODE_V2.c
 *
 *  Created on: 4 nov. 2025
 *      Author: Facundo Juli
 */

#include "DECODE_V2.h"
#include "FTM.h"

#define CONT_MAX 12 
static int not_reading = 1;

uint8_t processByte(void)
{
	static int cont = 0;
	int freq;
	static uint16_t data = 0;
	static int reset = 1;

	data = data << 1;

	freq = (int) IC_getFrequency();
	
	if(freq == 1200)
	{
		data = data | 1;
	}
	else if(freq != 2200)
	{
		cont = 0;
		data = 0;
		not_reading = 1;
		return 15; // ERROR invalid frequency
	}
	cont++;
	if(cont == CONT_MAX) //All bits have been succesfully read
	{
		cont = 0;
		data = 0;
		not_reading = 1;
	} 
	
	return cont;
}

int activateTimer(void) 
{
	if(bitStartDetected() && not_reading)
	{
		//se detectó un bit start
		//Trigger timer Tt < ~ 833 us  / (833us -  Tt)* Nbits < 833 us
		// 833us(1-1/Nbits) < Tt < 833us
		not_reading = 0;
		return 1;
	}
	return 0;
	/*
	 * if(timer)
	 * {
	 * if(timer == 9 tics)
	 * 		break
	 * 		freq = IC_getFrequency();
	 * if(freq > 2k)
	 * 		bit = 0;
	 * else if(fre1 < 2k)
	 * 		bit = 1;
	 * }
	 */
	
}
int bitStartDetected(void)
{
	int freq, bit_counter;

	if((freq = IC_getFrequency()) == 2200) //Conviene detectar asi, asi se que mi Tinicial esta ok
	{
		
		return 1;
	}

	return 0;
}

#include <stdint.h>

uint8_t NCO2PWM(uint16_t lut)
{
    const uint16_t MAX12 = (1u<<12) - 1u; // 4095
    if (lut > MAX12) 
	{
		lut = MAX12;         // clamp
	}
    // 0..4095 -> 0..99 
    return (uint8_t)(((uint32_t)lut * 100u) >> 12);
	//return (uint8_t)(((uint32_t)lut * 99) /MAX12);

}
