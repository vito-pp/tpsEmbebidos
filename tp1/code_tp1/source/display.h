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
// Máscaras de segmentos para display 7 segmentos ánodo común
static const uint8_t DIGITS_CA[16] = {
    0b0000001, // 0
    0b1001111, // 1
    0b0010010, // 2
    0b0000110, // 3
    0b1001100, // 4
    0b0100100, // 5
    0b0100000, // 6
    0b0001111, // 7
    0b0000000, // 8
    0b0000100, // 9
    0b0001000, // A
    0b1100000, // b
    0b0110001, // C
    0b1000010, // d
    0b0110000, // E
    0b0111000  // F
};


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
