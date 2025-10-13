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

/**
 * @brief Pushes data to the SPI TX FIFO with specified configuration.
 * @param spi_instance The SPI module instance to be used (e.g., 0 for SPI0, 1 for SPI1).
 * @param data The 16-bit data to be transmitted via the SPI interface.
 * @param pcs The Peripheral Chip Select (PCS) signal to be used for the transaction.
 * @param ctar_x The Configuration Register (CTAR) index to define SPI transfer settings.
 * @param eoq Indicates whether this is the End of Queue (EOQ) 
 *              for the transfer (true for EOQ, false otherwise).
 * @param ctcnt Clear Transfer Counter (true to clear, false to not clear).
 * @return 1 on success (data successfully pushed to the transmit buffer).
 */
int SPIx_pushTx(uint8_t spi_instance, uint16_t data, uint8_t pcs, uint8_t ctar_x, bool ctcnt);

/**
 * @brief Pops data from the SPI RX FIFO.
 * @param spi_instance The SPI module instance to be used (e.g., 0 for SPI0, 1 for SPI1).
 * @return The 32-bit data read from the SPI receive buffer (POPR register).
 */
uint32_t SPIx_popRx(uint8_t spi_instance);

/**
 * @brief For debug purposes: Returns data contained in SPI TX FIFO
 *          It does not pop/push data to the FIFO.
 * @param spi_instance The SPI module instance to be used (e.g., 0 for SPI0, 1 for SPI1).
 * @return The 32-bit SPI data to be shifted out
 */
uint32_t SPIx_TXFR0(uint8_t spi_instance);

/**
 * @brief Configures the pins for the SPI0 module, and the Module Configuration Register (MCR).
 * @param pins Pointer to an SPI_pins structure containing
 *             the pin assignments for SPI0 (SIN, SOUT, SCLK, PCS0).
 * @param mstre Specifies whether the SPI operates in master mode (true) or slave mode (false).
 * @param cont_scke Enables the Serial Communication Clock (SCK) to run continuously.
 * @param rooe Enables Receive FIFO Overflow Overwrite
 * @param pcsis Sets the Peripheral Chip Select (PCS) signals inactive state. 
 *              FALSE -->  low , TRUE --> high
 * @return None.
 */
void SPI0_Init(SPI_pins* pins, bool mstre, bool cont_scke, bool rooe, bool pcsis);

/**
 * @brief Configures the multiplexing and interrupt settings for a specified pin.
 * @param pin The pin to be configured.
 * @param alt The alternate function (MUX) to assign to the pin (e.g., GPIO, SPI, UART).
 * @param irqc The interrupt configuration for the pin (e.g., interrupt type or disabled).
 * @return None (void function, no return value).
 */
void pinConfig(uint8_t pin, uint8_t alt, bool irqc);

/**
 * @brief Configures the Clock and Transfer Attributes Register (CTAR) for the specified SPI instance.
 * @param spi_instance The SPI module instance to be configured (e.g., 0 for SPI0, 1 for SPI1).
 * @param ctar_x The CTAR register index to configure (e.g., 0 for CTAR0, 1 for CTAR1).
 * @param fmsz The frame size in bits for the SPI transfer.
 *              Real Frame Size = fmsz + 1,  ( 4 <= fmsz <= 15 bits)
 * @param cpol Selects the inactive state of the Serial Communications Clock (SCK)
 *          TRUE  --> Inactive Low
 *          FALSE --> Inactive High
 * @param cpha The clock phase.
 *          FALSE --> Data is captured on the leading edge of SCK and changed on the following edge.
 *          TRUE --> Data is changed on the leading edge of SCK and captured on the following edge.
 * @param lsfe Specifies the bit order.
 *          TRUE  --> for LSB first 
 *          FASLE --> for MSB first
 * @param br BR value in 10 kbps ( Bps = br . 10k bps) options (125, 5, 10, 25)
 * @return 0 on success 
 *         -1 if spi_instance or ctar_x is invalid
 *          1 if fmsz is out of valid range (4-15).
 */
int CTARX_config(uint8_t spi_instance, uint8_t ctar_x, uint8_t fmsz, 
                    bool cpol , uint8_t cpha, uint8_t lsfe, uint16_t br);




bool SPI_init(void);

bool isSPIDataReady(void);

uint64_t getSPIData(void);

int isTxOver(void);

bool sendSPIData(uint64_t data);



#endif