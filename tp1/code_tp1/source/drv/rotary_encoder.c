// Analisis de Estados General del Encoder.
#include <stdbool.h>

// Si RSwitch vale 1 en cualquier instante, el encoder fue apretado.

// Estado de inicializacion, el valor del encoder se encuentra en cero.
// Necesito una variable global, o general, o objecto de encoder que pueda llevar a cuenta el 
// valor del encoder.

// Variable Global del valor que representa el encoder [0,9].
int encoder_value = 0;

int last_encoder_state = 0b11;
int variable_encoder_state = 0b11;
int stabilizing_encoder_state = 0b11;

int current_controler_state = 0b11;

int control_state = WAITING_NEW_STATE;

enum control_states {
    WAITING_NEW_STATE = 0,
    STABILIZING_STATE = 1,
} encoder_states;


// Variables de estado
bool stateA;
bool stateB;

#define FREQUENCY_SAMPLE 2000 // Hz
#define STABILIZATION_TIME 350 // us

int cycles_stabilizing = 0;
int number_of_states_transitioned = 0;
int adder = 0;

// Author: @SantinoAgosti
// Date: 2024-01-09
// Return: int: El valor numerico del encoder, entre 0 y 9.
// info: Funcion llamada en interrupcion para actualizar el valor del encoder.
// La funcion debe ser llamada periodicamente mediante una interrupcion periodica asociada, 
// por ejemplo mediante Systick. La frecuencia de sampleo debe ser mayor a 1500Hz.
// Se confirma la transicion de estado luego de aproximadamente 350us.

// Version Basica 1.0
// Se actualiza el valor del encoder cuando se detecta un cambio en cualquiera de los pines A o B.
int encoder_numerical_state(bool clockwise){

    // Estados Anteriores de pines de Encoders
    int lastA_state = (last_encoder_state>1) & 0b1;
    int lastB_state = last_encoder_state & 0b1;

    // Medicion de estados. Cada estado es un bit
    // stateA = GPIO_read(pinA);
    // stateB = GPIO_read(pinB);

    // Actualizacion de estado de encoder
    current_encoder_state = (stateA < 1) | stateB;

    // Cambio de estados
    if (current_encoder_state != last_encoder_state){
        
        // Transicion de A de 0 a 1.
        if ((stateA != lastA_state) && ){
            encoder_value++;
            }

        // AntiClockwise se disminuye el contador.
        if (stateB != lastB_state){
            encoder_value--;
        }
    }
    last_encoder_state = current_encoder_state;
    
}

void encoder_button(){
    // Logica anti rebote de boton?
}