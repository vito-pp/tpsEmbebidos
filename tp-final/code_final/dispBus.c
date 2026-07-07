#include "dispBus.h"
#include "FTM.h"
#include <stdint.h>

/* A register containing de current DC (bit value) to set should exist.
Every 1.25us it should be updated with next bit value.
*/

void sendDispData(uint32_t * data, size_t size);
void sendWord(uint32_t word);
void updateQueue(void);

uint32_t word[64 + 1]; //local array containing the data that should
                    //be send to display + extr abit to indicate reset
uint32_t current_word;
uint32_t current_bit;

void dispBus_init(void)
{
    FTM_init();
}

//n = # of leds to be controlled
void sendDispData(uint32_t * requested_word, size_t n)
{
    //Saves 
    int i;
    for(i = 0; i < n; i++)
    {
        word[i] = requested_word[i];
    }
    word[i] = -1;


    armo el buffer de DC

    le paso a dma el buffer
}

//Queue should be updated every (24+1) *1.25 us = time to send a word
//This function could
void updateQueue(void)
{
    static int i;
    if(bit_reset)
    {
        current_word = word[i++];
    }
}