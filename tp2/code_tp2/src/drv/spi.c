#include "spi.h"
#include <stdio.h>
#include "../CMSIS/MK64F12.h"
#include "gpio.h"
#include "board.h"


#define FRAME_SIZE 8
#define CTAR_OPTIONS 2
#define TX_BUFFER_SIZE 30
#define RX_BUFFER_SIZE 30

static SPI_Type* const spi_base_adress[] = SPI_BASE_PTRS;

static PORT_Type * const kPort[] = PORT_BASE_PTRS;

static uint32_t tx_buffer[TX_BUFFER_SIZE];
static uint32_t rx_buffer[RX_BUFFER_SIZE];

/**
 * @brief Pushes data to the SPI0 TX FIFO with specified configuration.
 * @param data The 16-bit data to be transmitted via the SPI interface.
 * @param pcs The Peripheral Chip Select (PCS) signal to be used for the transaction.
 * @param ctar_x The Configuration Register (CTAR) index to define SPI transfer settings.
 * @param ctcnt Clear Transfer Counter (true to clear, false to not clear).
 * @return 1 on success (data successfully pushed to the transmit buffer).
 */
static int SPI0_pushTx(uint16_t data, uint8_t pcs, bool ctcnt);

/**
 * @brief Configures the multiplexing and interrupt settings for a specified pin.
 * @param pin The pin to be configured.
 * @param alt The alternate function (MUX) to assign to the pin (e.g., GPIO, SPI, UART).
 * @param irqc The interrupt configuration for the pin (e.g., interrupt type or disabled).
 * @return None (void function, no return value).
 */
static void pinConfig(uint8_t pin, uint8_t alt, bool irqc);

void pushTxRoundedBuffer(uint16_t data);

void SPI0Master_Init(void)
{
    int i = 0;
    for(i= 0; i < TX_BUFFER_SIZE; i++)
    {
        tx_buffer[i] = 0xFFFFFFFF;
    }

    bool cont_scke = 0;
    bool rooe = 0;
    bool pcsis = 0;

    //enables clock gating
    SIM->SCGC6 |= SIM_SCGC6_SPI0_MASK;

    pinConfig(SPI0_SIN, ALT2, 0);
    pinConfig(SPI0_SOUT, ALT2, 0);
    pinConfig(SPI0_SCLK, ALT2, 0);
    pinConfig(SPI0_PCS0, ALT2, 0);

    SPI_Type* spi_x = spi_base_adress[0];

    //Puede que falte habilitar mdis
    //MCR CONFIGURATION
    spi_x->MCR |= SPI_MCR_MSTR(1) | SPI_MCR_CONT_SCKE(cont_scke &&1) |
                    SPI_MCR_ROOE(rooe &&1) | SPI_MCR_PCSIS(pcsis &&1) |
                    SPI_MCR_PCSIS(pcsis &&1) | SPI_MCR_CLR_RXF(1) |
                    SPI_MCR_CLR_TXF(1);

    //CTAR0 CONFIGURATION
    bool cpol = 0;
    uint8_t cpha = 0;
    uint8_t lsfe = 0;
    uint8_t fmsz = 7;
    uint16_t br= 125;

    spi_x->CTAR[0] |= SPI_CTAR_FMSZ(fmsz &&1);
    spi_x->CTAR[0] |= SPI_CTAR_CPOL(cpol &&1);
    spi_x->CTAR[0] |= SPI_CTAR_CPHA(cpha &&1);
    spi_x->CTAR[0] |= SPI_CTAR_LSBFE(lsfe &&1);

    switch(br)
    {
        case 125: spi_x->CTAR[0] |= SPI_CTAR_PBR(5) | SPI_CTAR_BR(16); break;
        case 500: spi_x->CTAR[0] |= SPI_CTAR_PBR(5) | SPI_CTAR_BR(4); break;
        case 1000: spi_x->CTAR[0] |= SPI_CTAR_PBR(5) | SPI_CTAR_BR(2); break;
        case 2500: spi_x->CTAR[0] |= SPI_CTAR_PBR(2) | SPI_CTAR_BR(2); break;
        default: break;
    }
    

}


void pinConfig(uint8_t pin, uint8_t alt, uint8_t irqc)
{
    kPort[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] &= ~(PORT_PCR_MUX(0b111) | 
                                            PORT_PCR_IRQC(0b1111));
    kPort[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_MUX(alt);
    kPort[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_IRQC(irqc);
}

void SPI0_sendByte(uint16_t data_1)
{
    pushTxRoundedBuffer(data_1,0);
    
}
void SPI0_send3Bytes(uint16_t data_1, uint16_t data_2, uint16_t data_3)
{
    pushTxRoundedBuffer(data_1,1);
    pushTxRoundedBuffer(data_2,1);
    pushTxRoundedBuffer(data_3,0);
    
}

void pushTxRoundedBuffer(uint16_t data, bool cont)
{
    static int index = 0;
    tx_buffer[index] = data | SPI_PUSHR_CONT(cont) |SPI_PUSHR_PCS(1);
    index = (index + 1) % TX_BUFFER_SIZE;
}

void SPI0_PopRxFIFO(void)
{
    static int index = 0;
    uint16_t aux;



}


void SPI0_PushTx_IRQ(void)
{
    static int i = 0;
    if(tx_buffer[i] != 0xFFFFFFFF)
    {
        spi_base_adress[0]->PUSHR = tx_buffer[i];
        tx_buffer[i] = 0xFFFFFFFF;
        i = (i + 1) % TX_BUFFER_SIZE;
    }
    
}

void SPI0_PopRx_IRQ(void)
{
    static int i = 0;
    if(tx_buffer[i] != 0xFFFFFFFF)
    {
        spi_base_adress[0]->PUSHR = tx_buffer[i];
        tx_buffer[i] = 0xFFFFFFFF;
        i = (i + 1) % TX_BUFFER_SIZE;
    }
    
}




uint32_t SPIx_popRx(uint8_t spi_instance)
{
    return spi_base_adress[spi_instance]->POPR;
}

uint32_t SPIx_TXFR0(uint8_t spi_instance)
{
    return spi_base_adress[spi_instance]->TXFR0;
}

//fifo es de 4 x fmsz
