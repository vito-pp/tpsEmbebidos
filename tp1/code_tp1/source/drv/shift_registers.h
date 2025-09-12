/*
 * shift_registers.h
 *
 *  Created on: 9 sep. 2025
 *      Author: Usuario
 */

#ifndef DRV_SHIFT_REGISTERS_H_
#define DRV_SHIFT_REGISTERS_H_

#include<stdint.h>

int serialData_init(void);
void sendSerialData(uint16_t data);

#endif /* DRV_SHIFT_REGISTERS_H_ */
