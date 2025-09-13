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

/**
 * @brief Initialize Display (through serial data initialization)
 * @return TRUE if correctly initialized
 * @return FALSE if previously initialized
 */
int display_init(void);

/**
 * @brief Writes character to indexed display. (0-9 or '-')
 * @param num: character to be written
 * @param disp: index of display for number to be displayed (0,1,2,3)
 * @return TRUE: correctly displayed character
 * @return FALSE: invalid character
 */
int display(uint8_t num, uint8_t disp);

#endif
