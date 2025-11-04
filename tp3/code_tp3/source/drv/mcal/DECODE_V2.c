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
		if((freq = IC_getFrequency()) == prev_freq)
		{
			continue;
		}
		prev_freq = freq;

		switch(IC_getCounter())
		{
		case 1:
		case 2: break;
		case 3:
		case 4: break;
		default: break;
		}

	}

}
