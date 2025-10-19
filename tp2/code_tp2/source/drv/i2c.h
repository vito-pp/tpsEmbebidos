/***************************************************************************//**
  @file     i2c.h
  @brief    Driver for the I2C protocol for the Kinetis MCU
  @author   John Pork
 ******************************************************************************/

#ifndef _I2C_H_
#define _I2C_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../SDK/CMSIS/MK64F12_features.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define I2C_NUMBER_OF_CHANNELS FSL_FEATURE_SOC_I2C_COUNT // three I2C modules on the Kinetis K64 
#define I2C_POLLING_FLAG true // Set to true to use polling instead of IRQs 
#define I2C_RESTART 1<<8 // Reapeted start bit 8-bit symbol
#define I2C_READ    2<<8 // Read bit 8-bit symbol
#define I2C_MAX_SEQUENCE_LEN 100

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum
{
    I2C_AVAILABLE,
    I2C_BUSY,
    I2C_ERROR
} I2C_Status_e;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
/**
 * @brief Should be called at the start of the progam.
 * @param channel   I2C module number of kinetis, could be 0, 1, 2. Check for 
 * available modules on your device. Define I2C_NUMBER_OF_CHANNELS appropiately.
 * @param baud_rate Sets the baud rate in bps.
 * @return false on error, true on success.
 */
bool I2C_MasterInit(uint8_t channel, uint16_t baud_rate);

/**
 * @brief Sends a command/data sequence over I2C with support for restarts, 
 * writes, and reads. This function executes a complete I2C sequence that can 
 * include multiple writes, restarts, and reads. Each transmission 
 * automatically begins with a START condition and ends with a STOP condition, 
 * so these do not need to be specified in the sequence.
 *
 * The sequence is represented as an array of @c uint16_t elements (not 
 * @c uint8_t) to support out-of-band signaling for @c I2C_RESTART and 
 * @c I2C_READ operations while maintaining 8-bit data compatibility.
 * 
 * @param channel_number   I2C channel number to use for the transmission
 * @param sequence         Pointer to the I2C operation sequence array
 * @param sequence_length  Number of elements in the sequence array (minimum: 2)
 * @param received_data    Buffer to store bytes from I2C_READ operations. Pass 
 * @c NULL if there are no reads in the sequence. Buffer must be large enough 
 * to hold one byte per READ operation.
 * @return true on success, false on error.
 * @see I2C_RESTART
 * @see I2C_READ
 */
bool I2C_MasterSendSequence(uint8_t channel, uint16_t *sequence, 
                            uint32_t sequence_len, uint8_t *recieve_buffer);

/**
 * @brief Returns the status of the channel selected.
 */
I2C_Status_e I2C_GetStatus(uint8_t channel);

void I2C_ServicePoll(uint8_t ch);

#endif // _I2C_H_
