/*
 * mcuDisplay.h
 *
 * Driver para manejar un display de 7 segmentos multiplexado
 * con la placa Kinetis 64 Freedom.
 *
 * El driver permite mostrar texto, controlar el brillo,
 * y elegir el modo de desplazamiento automático.
 *
 *  Created on: 1 sep. 2025
 *  Author: Gonzalo Louzao
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

#define BRIGHTNESS_LEVELS 10
#define HYPHEN 0xA

/**
 * @brief Initialize Display (through serial data initialization)
 * @return TRUE if correctly initialized
 * @return FALSE if previously initialized
 */
int display_init(void);

/**
 * @brief Displays number on all 4 displays
 * @param number: number to be displayed
 * @param hide: if TRUE, hides all digits except the corresponding to unit
 * @param lenght: length of the number displayed. if exceeds the degits of 
 * number, will fill with zeros
 */
void display(unsigned int number, bool hide, uint8_t lenght);

void displayHyphens(void);

/**
 * @brief Clears display
 */
void dispClear(void);
/**
 * @brief Sets pwm value, mod BRIGHTNESS_LEVELS
 */
void setPWM(uint8_t desired_pwm);

/**
 * @brief Turns led on
 * @param led: led to be turned on (1-3)
 * @return TRUE led correctly turned on
 * @return FALSE failed to turn led on
 */
bool turnOnLED(uint8_t led);

/**
 * @brief Turns led off
 * @return TRUE led correctly turned off
 * @return FALSE failed to turn led off
 */
void turnOffLEDs(void);

#endif // DISPLAY_H_
