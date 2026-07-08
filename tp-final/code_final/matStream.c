#include "matStream.h"
#include "FTM.h"
#include <stdint.h>

/* A register containing de current DC (bit value) to set should exist.
Every 1.25us it should be updated with next bit value.
*/


void updateDisplay(void);

uint8_t duty_cycles[1537]; //64*24+1 local array containing the data that should
                    //be send to display (%DC) + extr a byte to indicate reset (0%DC)
uint32_t current_word;
uint32_t current_bit;

//CNV values to achieve desiredduty cycle
//cambiar por macros parametrizadas definidas en ftm.h
#define LOGICAL_1   200
#define LOGICAL_0   300
#define RESET 0

void dispBus_init(void)
{
    FTM_init();
    //config dma source and stop
}

//n = # of leds to be controlled
//load matrix tu show display

void updateDisplay(void)
{
    return; //enable dma request
}
void loadDisplay(uint32_t * word, size_t n)
{
    //Transforms bits from word to physical signals.
    int i, j;
    //For each word
    for(i = 0; i < n; i++)
    {
        //For each bit in word
        for(j = 0; j < 24; j++) //convertir luego en operador ternario para leve opt
        {
            if(word[i] & (1<<j)) //boolean value of i-bit
            {
                duty_cycles[24*i + j] = LOGICAL_1;
            }
            else
            {
                duty_cycles[24*i + j] = LOGICAL_0;

            }
        }
    }
    duty_cycles[24*i] = LOGICAL_RESET;  //to update changes
    return;
}