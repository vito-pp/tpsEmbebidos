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

/**
 * @brief Initialize Display (through serial data initialization)
 * @return TRUE if correctly initialized
 * @return FALSE if previously initialized
 */
int display_init(void);

/**
 * @brief Displays number on all 4 displays
 * @param number: number to be displayed
 */
void display(unsigned int number);

/**
 * @brief Clears display
 */
void dispClear(void);
/**
 * @brief Sets pwm value
 */
void setPWM(uint8_t desired_pwm);
/**
 * @brief Writes character to indexed display. (0-9 or '-')
 * @param num: character to be written
 * @param disp: index of display for number to be displayed (0,1,2,3)
 * @return TRUE: correctly displayed character
 * @return FALSE: invalid character
 */
bool displayDigit(uint8_t num, uint8_t disp);

/**
 * @brief Turns led on
 * @param led: led to be turned on
 * @return TRUE led correctly turned on
 * @return FALSE failed to turn led on
 */
bool turnOnLED(uint8_t led);

/**
 * @brief Turns led off
 * @param led: led to be turned off
 * @return TRUE led correctly turned off
 * @return FALSE failed to turn led off
 */
bool turnOffLED(uint8_t led);

#endif
