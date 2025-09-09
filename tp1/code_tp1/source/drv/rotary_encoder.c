// Analisis de Estados General del Encoder.
#include "rotary_encoder.h"
#include "gpio.h"
#include "board.h"

// Si RSwitch vale 1 en cualquier instante, el encoder fue apretado.

// Estado de inicializacion, el valor del encoder se encuentra en cero.
// Necesito una variable global, o general, o objecto de encoder que pueda llevar a cuenta el
// valor del encoder.

// Flag de Interrupciones
static volatile bool encoder_flag = false;

// Buffer for last 5 states (each state: 2 bits, A and B)
static uint8_t encoder_state_buffer[ENCODER_BUFFER_SIZE] = {0};
static uint8_t encoder_state_button_buffer[ENCODER_BUFFER_SIZE] = {0};

static uint8_t buffer_index = 0;
static uint8_t encoder_value = 0;
static uint8_t last_encoder_value = 0;

// Patterns for 5-step rotation (typical quadrature encoder)
const uint8_t CW_PATTERN[5] = {0b00, 0b01, 0b11, 0b10, 0b00};  // Clockwise
const uint8_t CCW_PATTERN[5] = {0b00, 0b10, 0b11, 0b01, 0b00}; // Counter-clockwise

// Author: @SantinoAgosti
// Date: 2024-01-09
// Return: int: El valor numerico del encoder, entre 0 y 9.
// info: Funcion llamada en interrupcion para actualizar el valor del encoder.
// La funcion debe ser llamada periodicamente mediante una interrupcion periodica asociada,
// por ejemplo mediante Systick. La frecuencia de sampleo debe ser mayor a 1500Hz.

// Version Basica 1.0
// Se actualiza el valor del encoder cuando se detecta un cambio en cualquiera de los pines A o B
static bool stateA = 0;
static bool stateB = 0;
static bool stateC = 0;

static uint16_t cycles_pressed = 0;
// Si se aprieta el boton durante dos segundos, entonces se considera un long press.

void encoder_callback(void)
{
    // This function can be used if interrupts are set up on encoder pins
    encoder_flag = true; // Set flag to indicate encoder state needs updating
}

uint8_t encoder_update(void)
{

    if (!encoder_flag)
    {
        return;
    }

    encoder_flag = false;

    stateA = gpioRead(PORTNUM2PIN(PB, 2));
    stateB = gpioRead(PORTNUM2PIN(PB, 3));
    stateC = gpioRead(PORTNUM2PIN(PB, 10));

    // Pack A and B into 2 bits
    uint8_t state = ((stateA & 0x1) << 1) | (stateB & 0x1);
    uint8_t state_button = (stateC & 0x1);

    // Si no hay cambios de estado de tipo de rotacion, o de presionar el boton, no hacer nada.
    if (encoder_state_buffer[(buffer_index + ENCODER_BUFFER_SIZE - 1) % ENCODER_BUFFER_SIZE] == state)
    {
        if (encoder_state_button_buffer[(buffer_index + ENCODER_BUFFER_SIZE - 1) % ENCODER_BUFFER_SIZE] == state_button)
        {
            return;
        }
    }

    // Store in buffer
    encoder_state_buffer[buffer_index] = state;
    encoder_state_button_buffer[buffer_index] = state_button;
    buffer_index = (buffer_index + 1) % ENCODER_BUFFER_SIZE;

    // Analisis de Rotacion (CW o CCW)
    bool match_cw = 1, match_ccw = 1;
    for (int i = 0; i < ENCODER_BUFFER_SIZE; i++)
    {
        uint8_t idx = (buffer_index + i) % ENCODER_BUFFER_SIZE;
        if (encoder_state_buffer[idx] != CW_PATTERN[i])
            match_cw = 0;
        if (encoder_state_buffer[idx] != CCW_PATTERN[i])
            match_ccw = 0;
    }

    // Analisis de Boton ()
    for (int i = 0; i < ENCODER_BUFFER_SIZE; i++)
    {
        uint8_t idx = (buffer_index + i) % ENCODER_BUFFER_SIZE;

        // Itero a lo largo de todo el buffer del boton. Si esta lleno de unos, entonces es un ciclo mas apretado.

        if (encoder_state_button_buffer[idx] != 1)
        {
            if (cycles_pressed >= MIN_PRESS_CYCLES)
            {
                if (cycles_pressed < LONG_PRESS_CYCLES)
                {
                    // Deteccion de Press
                    cycles_pressed = 0;
                    return ENC_BUTTON_PRESS;
                }
                else
                {
                    // Si se presiona durante dos segundos, entonces es un long press.
                    cycles_pressed = 0;
                    return ENC_BUTTON_LONG_PRESS;
                }
            }
            else
            {
                cycles_pressed = 0;
            }
        }
        else
        {
            cycles_pressed++;
        }
    }

    if (match_cw)
    {
        // Clockwise detected
        return ENC_CW;
    }

    else if (match_ccw)
    {
        // Counter-clockwise detected
        return ENC_CCW;
    }

    // No hubo accion alguna del encoder.
    return ENC_NONE;

}