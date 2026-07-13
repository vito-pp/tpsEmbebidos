#include "matStream.h"
#include "FTM.h"
#include <stdint.h>
#include "MK64F12.h"
#include "hardware.h"
#include"dma.h"
#include"gpio.h"




//CNV values to achieve desiredduty cycle
//cambiar por macros parametrizadas definidas en ftm.h
#define LOGICAL_1   DC2CNV(68)
#define LOGICAL_0   DC2CNV(34)

#define DUTY_BUFFER_SIZE 1536

/* A register containing de current DC (bit value) to set should exist.
Every 1.25us it should be updated with next bit value.
*/

uint16_t duty_cycles[DUTY_BUFFER_SIZE]; //64*24+1 local array containing the data that should
                    //be send to display (%DC) + extr a byte to indicate reset (0%DC)
uint32_t current_word;
uint32_t current_bit;

uint8_t sendingDMA = 0;


void WS2812_FrameDone(void);

void WS2812_FrameDone(void)
{
    // Se terminó de transmitir todo el buffer
	static int i = 0;
	DMA0->CINT |= 0;
	i++;

    DMA_Stop(0);
    PWM_setDuty(0);

    gpioWrite(PORTNUM2PIN(PB,3), 0);
    
    sendingDMA = 0;
}
void dispBus_init(void)
{
    FTM_Init();
    DMA_Init();

    uint32_t i;
    for(i = 0; i < DUTY_BUFFER_SIZE; i++)
    {
    	duty_cycles[i] = DC2CNV(5);
    }
    //config dma source and stop

    dma_cfg_t cfg;

    cfg.ch           = 0;                          //DMA 0 
    cfg.request_src  = DMA_REQ_FTM3CH0;            // Source FTM3_CH0
    cfg.saddr        = duty_cycles;                // Origin
    cfg.daddr        = (void*)(&(FTM3->CONTROLS[0].CnV));  // to: CnV
    cfg.elem_size    = 2;                          // 16-bit (CnV es de 16 bits)
    cfg.soff         = 2;                          // Move 2 bytes on source
    cfg.doff         = 0;                          // always write to cnv
    cfg.major_count  = DUTY_BUFFER_SIZE;                // # of elements
    cfg.slast        = - sizeof(duty_cycles);   // back to begining
    cfg.dlast        = 0;                              // fixed adress
    cfg.int_major    = true;  
    cfg.on_major = (void*) WS2812_FrameDone;   // <-- callback
    cfg.user = NULL;

    DMA_Config(&cfg);


    // borrar siguientes dos lineas
    //sendingDMA = 1;
    //DMA_Start(0);
}

void WS2812_Update(void)
{
    //lleno el buffer?
    dma_cfg_t cfg;

    cfg.ch           = 0;                          //DMA 0 
    cfg.request_src  = DMA_REQ_FTM3CH0;            // Source correcto para FTM3_CH0
    cfg.saddr        = duty_cycles;                // Origin
    cfg.daddr        = (void*)&(FTM3->CONTROLS[0].CnV);  // to: CnV
    cfg.elem_size    = 2;                          // 16-bit (CnV es de 16 bits)
    cfg.soff         = 2;                          // Move 2 bytes on source
    cfg.doff         = 0;                          // always write to cnv
    cfg.major_count  = DUTY_BUFFER_SIZE;                // # of elements
    cfg.slast        = - (int32_t)(DUTY_BUFFER_SIZE * 2);   // back to begining
    cfg.dlast        = 0;                              // fixed adress
    cfg.int_major    = true;  
    cfg.on_major = (void*) WS2812_FrameDone;   // <-- callback
    cfg.user = NULL;
    DMA_Config(&cfg);

    sendingDMA=1;
    DMA_Start(0);
}

//n = # of leds to be controlled
//load matrix tu show display

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
    //duty_cycles[24*i] = LOGICAL_RESET;  //to update changes
    return;
}
