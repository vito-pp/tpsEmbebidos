#ifndef SPIDRV_H
#define SPIDRV_H

#include <stdint.h>
#include <stdbool.h>

bool SPI_init(void);

void configCTAR0(uint8_t x);

bool isSPIDataReady(void);

uint64_t getSPIData(void);

int isTxOver(void);

bool sendSPIData(uint64_t data);



#endif