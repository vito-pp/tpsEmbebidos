/*
 * DECODE_V2.h
 *
 *  Created on: 4 nov. 2025
 *      Author: Usuario
 */

#ifndef DRV_MCAL_DECODE_V2_H_
#define DRV_MCAL_DECODE_V2_H_

int bitStartDetected(void);
int activateTimer(void);

/**
 * @brief   Mapea valores de 12 bits a [0;99].
 * @param   lut   Valor de senoidal [0;4095].
 * @return  Valor de duty cycle entre [0;99].
 */
uint8_t NCO2PWM(uint16_t lut);


#endif /* DRV_MCAL_DECODE_V2_H_ */
