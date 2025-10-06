#ifndef SPIDRV_H
#define SPIDRV_H

#include <stdint.h>
#include <stdio.h>

bool SPI_init(void);

bool isSPIDataReady(void);

uint64_t getSPIData(void);

int isTxOver(void);

bool sendSPIData(uint64_t);








#endif