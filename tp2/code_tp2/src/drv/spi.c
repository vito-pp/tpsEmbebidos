#include "spi.h"
#include <stdio.h>
#include "../CMSIS/MK64F12.h"
#define FRAME_SIZE 8

static SPI_Type* const spi_adresses[] = SPI_BASE_PTRS;

bool SPI_init(void)
{
    //Config Ctars
    //spi0 (4 tx fifo y 4 rx fifo)
    //Wake up mode? no, se puede perder first word
    //Spi _SR (interrupt)
    
}

void configCTAR0(uint8_t x)
{
    //if CURRENT TRASNFERING
    //          RETURN
    // DO NOT CONGI CTAR WHILE TRANSFER
    SPI_Type* spix = spi_adresses[x];

    
}

bool isSPIDataReady(void)
{

}

uint64_t getSPIData(void)
{

}

int isTxOver(void)
{

}

bool sendSPIData(uint64_t data)
{

}


