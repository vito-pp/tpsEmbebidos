/*
 * shift_registers.h
 *
 *  Created on: 9 sep. 2025
 *      Author: Usuario
 */

#ifndef DRV_SHIFT_REGISTERS_H_
#define DRV_SHIFT_REGISTERS_H_

#include<stdint.h>

/**
 * @brief Initialize Serial Data communication.
 * @return TRUE if correctly initialized
 * @return FALSE if previously initialized
 */
int serialData_init(void);

/**
 * @brief Sends serial data through previously initialized pin to shift
 * 		  shift register.
 *		  Manages following SR pins: LCLK, CLK, SERIAL DATA
 * @return TRUE if correctly initialized
 * @return FALSE if previously initialized
 */
void sendSerialData(uint16_t data);

#endif /* DRV_SHIFT_REGISTERS_H_ */
