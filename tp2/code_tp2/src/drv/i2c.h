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

#define I2C_NUMBER_OF_CHANNELS FSL_FEATURE_SOC_I2C_COUNT /* three I2C modules 
on the Kinetis K64 */
#define I2C_POLLING_FLAG false /* set to true to use polling instead of IRQs */

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

bool I2C_MasterInit(uint8_t channel, uint16_t baud_rate);

void I2C_MasterTx(uint8_t channel, uint8_t slave_address, uint8_t *data2send,
                  size_t len);

void I2C_MasterRx(uint8_t channel, uint8_t slave_address, uint8_t *data2read, 
                  size_t len);

void I2C_MasterTxRx(uint8_t channel, uint8_t slave_address, uint8_t *data2send, 
                    size_t send_len, uint8_t *data2read, size_t read_len);

#endif // _I2C_H_