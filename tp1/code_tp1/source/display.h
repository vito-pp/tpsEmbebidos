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

#ifndef MCUDISPLAY_H_
#define MCUDISPLAY_H_

#include <stdint.h>

/************* DEFINICIONES GENERALES *************/
#define LEN_TEXT 50          // Máximo de caracteres a mostrar en buffer
enum { D0, D1, D2, D3 };     // Estados para multiplexar los 4 dígitos

/************* ESTRUCTURA PRINCIPAL *************/
typedef struct {
    uint8_t value_0;     // Valor en segmento del dígito 0
    uint8_t value_1;     // Valor en segmento del dígito 1
    uint8_t value_2;     // Valor en segmento del dígito 2
    uint8_t value_3;     // Valor en segmento del dígito 3
    uint8_t status;      // Qué dígito está activo (D0..D3)
    int text_len;        // Largo del texto cargado
    int brightness;       // Brillo (se controla por duty cycle)
    int mode;            // Modo de funcionamiento: MOVE (desplazamiento) o fijo
} display_t;

/************* NIVELES DE BRILLO *************/
#define MIN_BRIGHTNESS   1
#define MID_BRIGHTNESS   4
#define MAX_BRIGHTNESS   8
#define INIT_BRIGHTNESS  1

#define MOVE   -1   // Constante para indicar modo scroll automático



/************* FUNCIONES PÚBLICAS *************/
void initMcuDisplay(void);                 // Inicializa el display y timers
void updateMcuDisplay(void);               // Actualiza estado (se llama periódicamente)

void textToDisplay(int index, uint8_t letter);  // Carga un carácter en el buffer
void textLenDisplay(int len);                   // Define la longitud del texto
void brightnessDisplay(int value_brightness);     // Ajusta brillo
void modeToDisplay(int mode);                   // Cambia modo de visualización

#endif /* MCUDISPLAY_H_ */
