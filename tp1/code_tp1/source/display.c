/*
 * mcuDisplay.c
 *
 * Implementación del driver para display 7 segmentos multiplexado.
 *
 * Permite mostrar hasta 4 caracteres a la vez en los dígitos D0..D3.
 * Soporta desplazamiento de texto (modo MOVE) o selección fija de posición.
 *
 *  Created on: 1 sep. 2025
 *  Author: Gonzalo Louzao
 */

#include "display.h"
#include "hardware.h"
//#include "timer.h"
#include "board.h"
#include "gpio.h"

/************* VARIABLES INTERNAS *************/
static display_t display;            // Estructura principal
static uint8_t arr_text[LEN_TEXT];   // Buffer con el texto a mostrar
static int char_counter = 0;         // Índice para scroll

// Macro para obtener el bit n de un número
#define GET_BIT(num, n) (((num) >> (n)) & 1)

// Tiempos de refresco
#define TIME_DISPLAY    1    // 1 ms entre dígitos (multiplexado)
#define TIME_CHARACTER  250  // 250 ms para avanzar al siguiente carácter

/************* FUNCIONES INTERNAS *************/
static void next_character(void);    // Cambia el texto en pantalla
static void next_status(void);       // Cambia de dígito activo (D0..D3)
static void printMcuD(void);         // Envía valor de un dígito al bus

/************* FUNCIONES PÚBLICAS *************/

/* Inicializa el driver del display */
void initMcuDisplay(void) {
    // Estado inicial del display
    display.value_0 = 0;
    display.value_1 = 0;
    display.value_2 = 0;
    display.value_3 = 0;
    display.status = D0;
    display.text_len = 0;
    display.brightness = INIT_BRIGHTNESS;
    display.mode = MOVE;

    // Configuración de pines de datos (BUS) y control
    gpioMode(PIN_BUS_0, OUTPUT);
    gpioMode(PIN_BUS_1, OUTPUT);
    gpioMode(PIN_BUS_2, OUTPUT);
    gpioMode(PIN_BUS_3, OUTPUT);
    gpioMode(PIN_BUS_4, OUTPUT);
    gpioMode(PIN_BUS_5, OUTPUT);
    gpioMode(PIN_BUS_6, OUTPUT);
    gpioMode(PIN_BUS_7, OUTPUT);

    gpioMode(PIN_STATUS_0, OUTPUT);
    gpioMode(PIN_STATUS_1, OUTPUT);

    // Timer para multiplexado
    tim_id_t displayTimer = timerGetId();
    timerStart(displayTimer, TIMER_MS2TICKS(TIME_DISPLAY),
               TIM_MODE_PERIODIC, updateMcuDisplay);

    // Timer para avanzar caracteres en scroll
    tim_id_t nextCharacterTimer = timerGetId();
    timerStart(nextCharacterTimer, TIMER_MS2TICKS(TIME_CHARACTER),
               TIM_MODE_PERIODIC, next_character);
}

/* Actualiza el display (cambia dígito activo según multiplexado) */
void updateMcuDisplay(void) {
    static int brightness_counter = 0;

    if (brightness_counter == 0) {
        printMcuD();        // Muestra el valor actual del dígito
        next_status();      // Avanza al siguiente dígito
        brightness_counter = display.brightness;
    }
    --brightness_counter;
}

/* Carga un carácter en el buffer del texto */
void textToDisplay(int index, uint8_t letter) {
    if (index < LEN_TEXT) {
        arr_text[index] = letter;
    }
}

/* Define la longitud del texto cargado */
void textLenDisplay(int len_new) {
    if (len_new < LEN_TEXT) {
        display.text_len = len_new;
        char_counter = 0;
    }
}

/* Ajusta el brillo (duty cycle del multiplexado) */
void brightnessDisplay(int value_brightness) {
    display.brightness = value_brightness;
}

/* Cambia el modo de visualización */
void modeToDisplay(int mode) {
    if (mode >= 0 && mode <= display.text_len) {
        display.mode = mode;
    } else {
        display.mode = MOVE; // Si no es válido, vuelve a scroll
    }
}

/************* FUNCIONES INTERNAS *************/

/* Avanza al siguiente bloque de caracteres en el buffer */
static void next_character(void) {
    if (display.text_len < 5) {
        // Si el texto entra completo en 4 dígitos
        display.value_0 = arr_text[0];
        display.value_1 = arr_text[1];
        display.value_2 = arr_text[2];
        display.value_3 = arr_text[3];
    } else if (display.mode != MOVE) {
        // Modo fijo: muestra caracteres desde posición "mode"
        display.value_0 = arr_text[display.mode];
        display.value_1 = arr_text[display.mode + 1];
        display.value_2 = arr_text[display.mode + 2];
        display.value_3 = arr_text[display.mode + 3];
    } else if (char_counter <= (display.text_len - 4)) {
        // Modo scroll: avanza de a 1 carácter
        display.value_0 = arr_text[char_counter + 0];
        display.value_1 = arr_text[char_counter + 1];
        display.value_2 = arr_text[char_counter + 2];
        display.value_3 = arr_text[char_counter + 3];
        ++char_counter;
    } else {
        char_counter = 0; // Reinicia scroll
    }
}

/* Cambia el dígito activo (D0 → D1 → D2 → D3) */
static void next_status(void) {
    switch (display.status) {
        case D0: display.status = D1; break;
        case D1: display.status = D2; break;
        case D2: display.status = D3; break;
        case D3: display.status = D0; break;
        default: display.status = D0;
    }
}

/* Escribe en el bus los segmentos del dígito actual */
static void printMcuD(void) {
    // Apaga todos los segmentos antes de cargar
    for (int i = 0; i < 8; i++) {
        gpioWrite(PIN_BUS_0 + i, 0);
    }

    // Selecciona el dígito activo
    gpioWrite(PIN_STATUS_0, GET_BIT(display.status, 0));
    gpioWrite(PIN_STATUS_1, GET_BIT(display.status, 1));

    // Carga el valor correspondiente
    uint8_t value;
    switch (display.status) {
        case D0: value = display.value_0; break;
        case D1: value = display.value_1; break;
        case D2: value = display.value_2; break;
        case D3: value = display.value_3; break;
        default: value = 0; break;
    }

    for (int i = 0; i < 8; i++) {
        gpioWrite(PIN_BUS_0 + i, GET_BIT(value, i));
    }
}
