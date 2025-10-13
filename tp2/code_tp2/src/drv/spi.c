#include "spi.h"
#include <stdio.h>
#include "../CMSIS/MK64F12.h"
#include "board.h"
#include "gpio.h"


#define FRAME_SIZE 8
#define SPI_INSTANCES 3
#define CTAR_OPTIONS 2

static SPI_Type* const spi_base_adress[] = SPI_BASE_PTRS;

static PORT_Type * const kPort[] = PORT_BASE_PTRS;


int SPIx_pushTx(uint8_t spi_instance, uint16_t data, uint8_t pcs, uint8_t ctar_x,
            bool eoq, bool ctcnt)
{

    //Validate all
    spi_base_adress[spi_instance]->PUSHR |= (SPI_PUSHR_TXDATA(data)
                                        | SPI_PUSHR_CTAS(ctar_x)
                                        | SPI_PUSHR_EOQ(eoq &&1)
                                        | SPI_PUSHR_CTCNT(ctcnt &&1));
    return 1;

}


uint32_t SPIx_popRx(uint8_t spi_instance)
{
    return spi_base_adress[spi_instance]->POPR;
}

uint32_t SPIx_TXFR0(uint8_t spi_instance)
{
    return spi_base_adress[spi_instance]->TXFR0;
}


void SPI0_pinConfig(SPI_pins* pins)
{
    pins->SIN = PORTNUM2PIN(PD, 3);
    pins->SOUT = PORTNUM2PIN(PD, 2);
    pins->SCLK = PORTNUM2PIN(PD, 1);
    pins->PCS0 = PORTNUM2PIN(PD, 0);

    pinConfig(pins->SIN, ALT2, 0);
    pinConfig(pins->SOUT, ALT2, 0);
    pinConfig(pins->SCLK, ALT2, 0);
    pinConfig(pins->PCS0, ALT2, 0);
}


void pinConfig(uint8_t pin, uint8_t alt, uint8_t irqc)
{
    kPort[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] &= ~(PORT_PCR_MUX(0b111) | 
                                            PORT_PCR_IRQC(0b1111));
    kPort[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_MUX(alt);
    kPort[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_IRQC(irqc);
}


int MCRX_config(uint8_t spi_instance, bool mstre, bool cont_scke, bool rooe,
             bool pcsis)
{
    if(spi_instance >= SPI_INSTANCES)
    {
        return -1;
    }

    SPI_Type* spi_x = spi_base_adress[spi_instance];

    spi_x->MCR |= SPI_MCR_MSTR(mstre &&1);
    spi_x->MCR |= SPI_MCR_CONT_SCKE(cont_scke &&1);
    spi_x->MCR |= SPI_MCR_ROOE(rooe &&1); 
    spi_x->MCR |= SPI_MCR_PCSIS(pcsis &&1);

    return 0;
}


int CTARX_config(uint8_t spi_instance, uint8_t ctar_x, uint8_t fmsz, 
                    bool cpol , uint8_t cpha, uint8_t lsfe, uint8_t br)
{
    if(spi_instance >= SPI_INSTANCES || ctar_x >= CTAR_OPTIONS)
    {
        return -1;
    }
    SPI_Type* spi_x = spi_base_adress[spi_instance];

    if(fmsz > 15 || fmsz < 4)
    {
        return 1; //invalid
    }
    
    spi_x->CTAR[ctar_x] |= SPI_CTAR_FMSZ(fmsz &&1);
    spi_x->CTAR[ctar_x] |= SPI_CTAR_CPOL(cpol &&1);
    spi_x->CTAR[ctar_x] |= SPI_CTAR_CPHA(cpha &&1);
    spi_x->CTAR[ctar_x] |= SPI_CTAR_LSBFE(lsfe &&1);
    
    return 0;
}

int getStatus(void)
{
    //SPI_SR
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


