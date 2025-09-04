// Analisis de Estados General del Encoder.
#include "rotary_encoder.h"

// Si RSwitch vale 1 en cualquier instante, el encoder fue apretado.

// Estado de inicializacion, el valor del encoder se encuentra en cero.
// Necesito una variable global, o general, o objecto de encoder que pueda llevar a cuenta el 
// valor del encoder.

// Variable Global del valor que representa el encoder [0,9].
int encoder_value = 0;

// Buffer for last 5 states (each state: 2 bits, A and B)
#define ENCODER_BUFFER_SIZE 5
uint8_t encoder_state_buffer[ENCODER_BUFFER_SIZE] = {0};
uint8_t buffer_index = 0;

// Patterns for 5-step rotation (typical quadrature encoder)
const uint8_t CW_PATTERN[5]  = {0b00, 0b01, 0b11, 0b10, 0b00}; // Clockwise
const uint8_t CCW_PATTERN[5] = {0b00, 0b10, 0b11, 0b01, 0b00}; // Counter-clockwise

// Author: @SantinoAgosti
// Date: 2024-01-09
// Return: int: El valor numerico del encoder, entre 0 y 9.
// info: Funcion llamada en interrupcion para actualizar el valor del encoder.
// La funcion debe ser llamada periodicamente mediante una interrupcion periodica asociada, 
// por ejemplo mediante Systick. La frecuencia de sampleo debe ser mayor a 1500Hz.

// Version Basica 1.0
// Se actualiza el valor del encoder cuando se detecta un cambio en cualquiera de los pines A o B

int encoder_update(bool stateA, bool stateB) {
    // Pack A and B into 2 bits
    uint8_t state = ((stateA & 0x1) << 1) | (stateB & 0x1);
    // If state is same as previous, return immediately
    if (encoder_state_buffer[(buffer_index + ENCODER_BUFFER_SIZE - 1) % ENCODER_BUFFER_SIZE] == state) {
        return encoder_value;
    }
    // Store in buffer
    encoder_state_buffer[buffer_index] = state;
    buffer_index = (buffer_index + 1) % ENCODER_BUFFER_SIZE;

    // Check for CW or CCW pattern
    int match_cw = 1, match_ccw = 1;
    for (int i = 0; i < ENCODER_BUFFER_SIZE; i++) {
        uint8_t idx = (buffer_index + i) % ENCODER_BUFFER_SIZE;
        if (encoder_state_buffer[idx] != CW_PATTERN[i]) match_cw = 0;
        if (encoder_state_buffer[idx] != CCW_PATTERN[i]) match_ccw = 0;
    }
    if (match_cw) {
        // Clockwise detected
        encoder_value++;
        if (encoder_value > 9) encoder_value = 0;
    } else if (match_ccw) {
        // Counter-clockwise detected
        encoder_value--;
        if (encoder_value < 0) encoder_value = 9;
    }
    return encoder_value;
}