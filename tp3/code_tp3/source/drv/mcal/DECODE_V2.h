/*
 * DECODE_V2.h
 *
 *  Created on: 4 nov. 2025
 *      Author: Usuario
 */
#include<stdint.h>
#include<stdbool.h>

#ifndef DRV_MCAL_DECODE_V2_H_
#define DRV_MCAL_DECODE_V2_H_

void setReadingFlag(void);
void clearReadingFlag(void);
bool getReadingFlag(void);
uint8_t processBit(void);
bool bitStartDetected(void);


#endif /* DRV_MCAL_DECODE_V2_H_ */
