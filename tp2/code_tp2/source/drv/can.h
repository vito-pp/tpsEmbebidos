/*
 * can.h
 *
 *  Created on: 20 oct. 2025
 *      Author: Usuario
 */

#ifndef DRV_CAN_H_
#define DRV_CAN_H_

#include <stdint.h>

#define RESET_INSTR 0b11000000
#define	WRITE_INSTR 0b00000010
#define	BIT_MODIFY_INSTRUCTION 	0b00000101
#define	READ_INSTRUCTION 0b00000011

//In order to initiate message transmission, the TXREQ bit in TXBxCTRL
// (Sending the SPI RTS command)

void CAN_sendData(uint8_t* data, size_t n_bytes);
uint8_t CAN_readData(uint8_t* data);

uint8_t CAN_rxStatus(void);


void CAN_Init(void);

#endif /* DRV_CAN_H_ */
