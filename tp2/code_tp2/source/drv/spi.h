#ifndef SPIDRV_H
#define SPIDRV_H

#include <stdint.h>
#include <stdbool.h>



void SPI0_PushTxRx_IRQ(void);




/**
 * @brief Configures the pins for the SPI0 to be used as masters.
 *        Used pins are specified in board.h file.
 * @return None.
 */
void SPI0Master_Init(void);

/*
 * Returns RX data or 0xFFFFFFFF if RX FIFO is empty
 */
uint32_t SPI0_PopRxFIFO(void);

void SPI0_sendByte(uint8_t data_1);

void SPI0_send2Bytes(uint8_t data_1, uint8_t data_2);

void SPI0_send3Bytes(uint8_t data_1, uint8_t data_2, uint8_t data_3);

void SPI0_send4Bytes(uint8_t data_1, uint8_t data_2, uint8_t data_3, uint8_t data_4);





#endif
