// Analisis de Estados General del Encoder.
#include "rotary_encoder.h"
#include "gpio.h"
#include "board.h"

#define PIN_ENC_A PORTNUM2PIN(PC,5)
#define PIN_ENC_B PORTNUM2PIN(PC,7)
#define PIN_ENC_C PORTNUM2PIN(PC,0)

// Si RSwitch vale 1 en cualquier instante, el encoder fue apretado.

// Estado de inicializacion, el valor del encoder se encuentra en cero.
// Necesito una variable global, o general, o objecto de encoder que pueda llevar a cuenta el
// valor del encoder.

// Flag de Interrupciones
static volatile bool encoder_flag = false;
static bool long_holding = false;

static bool checking_double_press = 0;
static uint16_t cycles_since_last_pressed = 0;

// Buffer for last 5 states (each state: 2 bits, A and B)
static uint8_t encoder_state_buffer[ENCODER_BUFFER_SIZE] = {0};
static uint8_t encoder_state_button_buffer[ENCODER_BUFFER_SIZE] = {0};

static uint8_t buffer_index_rotation = 0;
static uint8_t buffer_index_button = 0;

// Patterns for 5-step rotation (typical quadrature encoder)
const uint8_t CCW_PATTERN[5] = {0b11, 0b01, 0b00, 0b10, 0b11};  // Clockwise
const uint8_t CW_PATTERN[5] = {0b11, 0b10, 0b00, 0b01, 0b11}; // Counter-clockwise

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

void encoderInit()
{
    gpioMode(PIN_ENC_A, INPUT);
    gpioMode(PIN_ENC_B, INPUT);
    gpioMode(PIN_ENC_C, INPUT);
    gpioMode(PIN_LED_BLUE, OUTPUT);
}

uint8_t encoder_update(void)
{

    if (!encoder_flag)
    {
        return ENC_NONE;
    }

    encoder_flag = false;

    stateA = gpioRead(PIN_ENC_A);
    stateB = gpioRead(PIN_ENC_B);
    stateC = gpioRead(PIN_ENC_C);

    // Pack A and B into 2 bits
    uint8_t state = ((stateA & 0x1) << 1) | (stateB & 0x1);
    uint8_t state_button = (stateC & 0x1);
    encoder_state_button_buffer[buffer_index_button] = state_button;
    buffer_index_button = (buffer_index_button + 1) % ENCODER_BUFFER_SIZE;



    // Analisis de Boton ()
        for (int i = 0; i < ENCODER_BUFFER_SIZE; i++)
        {
            uint8_t idx = (buffer_index_button + i) % ENCODER_BUFFER_SIZE;

            // Itero a lo largo de todo el buffer del boton. Si esta lleno de unos, entonces es un ciclo mas apretado.
            // Si se presiona durante dos segundos, entonces es un long press.

            // Aca hay una leve consideracion. Cuando yo aprieto una vez, si aprieto despues y mantengo
            // lo suficientemente largo, el primer click no se analiza como un click, sino que se pierde.
            // Y unicamente se prioriza el Long Press.

            if (cycles_pressed > LONG_PRESS_CYCLES && !long_holding)
            {
            	cycles_pressed = 0;
            	long_holding = true;
                return ENC_BUTTON_LONG_PRESS;
            }
            
            
            if (checking_double_press){
                cycles_since_last_pressed++;
                if (cycles_since_last_pressed > MAX_CYCLES_BETWEEN_DOUBLE_PRESS){
                    checking_double_press = false;
                    cycles_since_last_pressed = 0;

                    // Recien devuelvo un Button Press cuando verifique que el usuario no quiso hacer en realidad un double press.
                    // Efectivamente, esto genera un delay en la respuesta del boton, pero se busca hacer lo mas baja posible.
                    return ENC_BUTTON_PRESS;
                }
            }

            // Se solto el boton
            if (encoder_state_button_buffer[idx] != 0)
            {

                long_holding = false;

                if (cycles_pressed >= MIN_PRESS_CYCLES)
                {
                        cycles_pressed = 0;
                        if (checking_double_press){
                            checking_double_press = false;
                            return ENC_DOUBLE_PRESS;
                        } else {
                            checking_double_press = true;
                            cycles_since_last_pressed = 0;
                        }


                }
                cycles_pressed = 0;
                // Sali del For. No quiero que se sume ningun ciclo.
                break;
            }
            else
            {
            	if (!long_holding){
                    cycles_pressed++;
            	}
            }
        }

    // Si no hay cambios de estado de tipo de rotacion, o de presionar el boton, no hacer nada.
    if (encoder_state_buffer[(buffer_index_rotation + ENCODER_BUFFER_SIZE - 1) % ENCODER_BUFFER_SIZE] == state)
    {
    	return ENC_NONE;
    }

    // Store in buffer
    encoder_state_buffer[buffer_index_rotation] = state;
    buffer_index_rotation = (buffer_index_rotation + 1) % ENCODER_BUFFER_SIZE;

    // Analisis de Rotacion (CW o CCW)
    bool match_cw = 1, match_ccw = 1;
    for (int i = 0; i < ENCODER_BUFFER_SIZE; i++)
    {
        uint8_t idx = (buffer_index_rotation + i) % ENCODER_BUFFER_SIZE;
        if (encoder_state_buffer[idx] != CW_PATTERN[i])
            match_cw = 0;
        if (encoder_state_buffer[idx] != CCW_PATTERN[i])
            match_ccw = 0;
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
