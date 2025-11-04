/*
 * DECODE_V2.c
 *
 *  Created on: 4 nov. 2025
 *      Author: Facundo Juli
 */

#include "DECODE_V2.h"
#include "FTM.h"


void decodeByte(void)
{
	int freq, counter;
	int prev_freq = IC_getFrequency();
	for(;;)
	{
		if((freq = IC_getFrequency()) != prev_freq) //capaz conviene detectar si es igual a 2k2?
		{
			//se detectó un bit start
			//Trigger timer Tt < ~ 833 us  / (833us -  Tt)* Nbits < 833 us
			// 833us(1-1/Nbits) < Tt < 833us
			break;
		}
		prev_freq = freq;
	}

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
