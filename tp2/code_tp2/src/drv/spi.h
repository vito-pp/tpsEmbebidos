#ifndef SPIDRV_H
#define SPIDRV_H

#include <stdint.h>
#include <stdbool.h>


/**
 * @brief Configures the pins for the SPI0 to be used as masters.
 *        Used pins are specified in board.h file.
 * @return None.
 */
void SPI0Master_Init(void);

void SPI0_PushTx_IRQ(void);

void SPI0_PopRx_IRQ(void);

/**
 * @brief Pops data from the SPI RX FIFO.
 * @param spi_instance The SPI module instance to be used (e.g., 0 for SPI0, 1 for SPI1).
 * @return The 32-bit data read from the SPI receive buffer (POPR register).
 */
uint32_t SPIx_popRx(uint8_t spi_instance);

void SPI0_sendByte(uint16_t data_1);

void SPI0_send3Bytes(uint16_t data_1, uint16_t data_2, uint16_t data_3);





#endif