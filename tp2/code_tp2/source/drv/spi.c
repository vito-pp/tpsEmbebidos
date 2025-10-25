#include "spi.h"
#include <stdio.h>
#include "../CMSIS/MK64F12.h"
#include "gpio.h"
#include "board.h"
#include "hardware.h"
#include "../misc/timer.h"

#define TX_BUFFER_SIZE 30
#define RX_BUFFER_SIZE 30

static SPI_Type* const spi_base_adress[] = SPI_BASE_PTRS;

static PORT_Type * const kPort[] = PORT_BASE_PTRS;

static uint32_t tx_buffer[TX_BUFFER_SIZE];
static uint32_t rx_buffer[RX_BUFFER_SIZE];

/**
 * @brief Configures the multiplexing and interrupt settings for a specified pin.
 * @param pin The pin to be configured.
 * @param alt The alternate function (MUX) to assign to the pin (e.g., GPIO, SPI, UART).
 * @param irqc The interrupt configuration for the pin (e.g., interrupt type or disabled).
 * @return None (void function, no return value).
 */
static void pinConfig(uint8_t pin, uint8_t alt, uint8_t irqc);

static void pushTxRoundedBuffer(uint8_t data, bool cont);

void sclkIRQ(void);

void sclkIRQ(void)
{
	static int cont = 0;
	//cont llega hasta 12

	if(cont == (31))
	{
		SPI0_FlushRX();
		SPI0_pushTxFIFO();
	}


	cont = (cont + 1 ) & 0b11111; //size of 4 bytes tx


}

void SPI0_FlushRX(void)
{
	static int j = 0; //Rx FIFO index
	SPI_Type * spi_x = (SPI_Type*) spi_base_adress[0];
	while((spi_x->SR & SPI_SR_RXCTR_MASK) >>4)
	{
	   	rx_buffer[j] = spi_base_adress[0]->POPR;
	   	j = (j + 1) % RX_BUFFER_SIZE;
	}
}


//Pushes message to HW FIFO
//Pushes last received message to SW FIFO
void SPI0_pushTxFIFO(void)
{
    static int i = 0; //Tx FIFO index



    uint32_t aux = 0;

    int counter = 0;
    //Flushes HW FIFO
    do
    {
    	aux = tx_buffer[i];
        uint8_t aux2 = aux  & 0xFF;

        if(tx_buffer[i] != 0xFFFFFFFF)
        {
       	    spi_base_adress[0]->PUSHR = tx_buffer[i];
       	    tx_buffer[i] = 0xFFFFFFFF;
       	    i = (i + 1) % TX_BUFFER_SIZE;
       	    counter++;
       	    while(!SPI0_isTxComplete());


        }
        else
        {
        	break;
        }
    }
    while((aux & SPI_PUSHR_CONT_MASK) && counter < 4); //Checks if last Tx is the EOQ message.
    										// no more than 4 consecutives push
}

void clearTxFlag(void)
{
	(spi_base_adress[0])->SR = SPI_SR_TCF_MASK;
}

bool SPI0_isTxComplete(void)
{
	if((spi_base_adress[0])->SR & SPI_SR_TCF_MASK)
	{
		(spi_base_adress[0])->SR = SPI_SR_TCF_MASK;
		return 1;
	}
	else
	{
		return 0;
	}

}

void flushTxFIFO(void)
{
	int i = 0;
		    for(i= 0; i < TX_BUFFER_SIZE; i++)
		    {
		        tx_buffer[i] = 0xFFFFFFFF;
		    }
		    for(i= 0; i < RX_BUFFER_SIZE; i++)
		    {
		        rx_buffer[i] = 0xFFFFFFFF;
		}
}
void SPI0Master_Init(void)
{
	flushTxFIFO();
    bool cont_scke = 0;
    bool rooe = 0;
    bool pcsis = 1;
    bool mdis = 0;
    bool halt = 0;

    //enables clock gating
    SIM->SCGC6 |= SIM_SCGC6_SPI0_MASK;

    pinConfig(SPI0_SIN, ALT2, 0);
    pinConfig(SPI0_SOUT, ALT2, 0);
    pinConfig(SPI0_SCLK, ALT2, 0);
    pinConfig(SPI0_PCS0, ALT2, 0);

    gpioMode(SPI0_CS_GPIO, INPUT);
    gpioIRQ(SPI0_CS_GPIO, PORT_PCR_IRQC_INT_RISING, SPI0_FlushRX);

    gpioMode(SPI0_CLK_GPIO, INPUT);
    gpioIRQ(SPI0_CLK_GPIO, PORT_PCR_IRQC_INT_RISING, sclkIRQ );



    SPI_Type* spi_x = spi_base_adress[0];

    //Puede que falte habilitar mdis
    //MCR CONFIGURATION
    spi_x->MCR = SPI_MCR_MSTR(1) | SPI_MCR_CONT_SCKE(cont_scke &&1) |
                    SPI_MCR_ROOE(rooe &&1) | SPI_MCR_PCSIS(pcsis &&1) |
                    SPI_MCR_PCSIS(pcsis &&1) | SPI_MCR_CLR_RXF(1) |
                    SPI_MCR_CLR_TXF(1) | SPI_MCR_MDIS(mdis) | SPI_MCR_HALT(halt);

    //CTAR0 CONFIGURATION
    bool cpol = 0;
    uint8_t cpha = 0;
    uint8_t lsfe = 1;
    uint8_t fmsz = 7;
    uint16_t br= 40;

    spi_x->CTAR[0] |= SPI_CTAR_FMSZ(fmsz &&1);
    spi_x->CTAR[0] |= SPI_CTAR_CPOL(cpol &&1);
    spi_x->CTAR[0] |= SPI_CTAR_CPHA(cpha &&1);
    spi_x->CTAR[0] |= SPI_CTAR_LSBFE(lsfe &&1);


    //SCK baud rate = (fP /PBR) x [(1+DBR)/BR)]
    switch(br)
    {
    	case 9: spi_x->CTAR[0] |= SPI_CTAR_PBR(0b10) | SPI_CTAR_BR(0b1010); break;
        case 20: spi_x->CTAR[0] |= SPI_CTAR_PBR(0b10) | SPI_CTAR_BR(0b1001); break;
        case 40: spi_x->CTAR[0] |= SPI_CTAR_PBR(0b10) | SPI_CTAR_BR(0b1000); break;
        case 80: spi_x->CTAR[0] |= SPI_CTAR_PBR(0b10) | SPI_CTAR_BR(0b110); break;
        default: break;
    }
    
    spi_x->SR |= SPI_SR_TCF_MASK;


}


void pinConfig(uint8_t pin, uint8_t alt, uint8_t irqc)
{
    kPort[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] &= ~(PORT_PCR_MUX(0b111) | 
                                            PORT_PCR_IRQC(0b1111));
    kPort[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_MUX(alt);
    kPort[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_IRQC(irqc);
}

bool SPI0_isTxQueueEmpty(void)
{
	int i;
	for(i= 0; i < TX_BUFFER_SIZE; i++)
	{
		if(tx_buffer[i] != 0xFFFFFFFF)
		{
			return 0;
		}
	}
	return 1;
}

void SPI0_sendNBytes(uint8_t* data, uint8_t n_bytes)
{
	int i;
	for(i = 0; i < n_bytes -1; i++)
	{
		pushTxRoundedBuffer(data[i],1);
	}

	pushTxRoundedBuffer(data[i], 0);

}
void SPI0_sendByte(uint8_t data_1)
{
    pushTxRoundedBuffer(data_1,0);
    
}

void SPI0_send2Bytes(uint8_t data_1, uint8_t data_2)
{
    pushTxRoundedBuffer(data_1,1);
    pushTxRoundedBuffer(data_2,0);
    
}

void SPI0_send3Bytes(uint8_t data_1, uint8_t data_2, uint8_t data_3)
{
    pushTxRoundedBuffer(data_1,1);
    pushTxRoundedBuffer(data_2,1);
    pushTxRoundedBuffer(data_3,0);
    
}

void SPI0_send4Bytes(uint8_t data_1, uint8_t data_2, uint8_t data_3, uint8_t data_4)
{
    pushTxRoundedBuffer(data_1,1);
    pushTxRoundedBuffer(data_2,1);
    pushTxRoundedBuffer(data_3,1);
    pushTxRoundedBuffer(data_4,0);
    
}

void pushTxRoundedBuffer(uint8_t data, bool cont)
{
    static int index = 0;
    tx_buffer[index] = data | SPI_PUSHR_CONT(cont) | SPI_PUSHR_PCS(1);
    index = (index + 1) % TX_BUFFER_SIZE;
}

uint32_t SPI0_PopRxFIFO(void)
{
    static int index = 0;
    uint32_t aux;
    if(rx_buffer[index] != 0xFFFFFFFF)
    {
        aux = rx_buffer[index];
        rx_buffer[index] = 0xFFFFFFFF;
        index = (index + 1) % RX_BUFFER_SIZE;
        return aux;
    }
    return 0xFFFFFFFF;
}
