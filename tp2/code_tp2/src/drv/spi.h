#ifndef SPIDRV_H
#define SPIDRV_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t SIN;
    uint8_t SOUT;
    uint8_t SCLK;
    uint8_t PCS0;
}SPI_pins;

void SPI0_pinConfig(SPI_pins* pins);

void pinConfig(uint8_t pin, uint8_t alt, bool irqc);


bool SPI_init(void);

int MCRX_config(uint8_t spi_instance, bool mstre, bool cont_scke, bool rooe,
             bool pcsis);

int CTARX_config(uint8_t spi_instance, uint8_t ctar_x, uint8_t fmsz, 
                    bool cpol , uint8_t cpha, uint8_t lsfe, uint8_t br);


bool isSPIDataReady(void);

uint64_t getSPIData(void);

int isTxOver(void);

bool sendSPIData(uint64_t data);



#endif