#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdbool.h>
#include <stdint.h>

#define ENCODER_BUFFER_SIZE 5
#define MIN_PRESS_CYCLES 50

#define LONG_PRESS_TIME 2 //Seconds
#define INTERRUPT_FREQUENCY 2000 //Hz

#define MAX_DOUBLE_PRESS_COOLDOWN 1 // Seconds

#define LONG_PRESS_CYCLES ((INTERRUPT_FREQUENCY * LONG_PRESS_TIME)*ENCODER_BUFFER_SIZE) //Ciclos
#define MAX_CYCLES_BETWEEN_DOUBLE_PRESS 1000 //Ciclos

typedef enum {
    ENC_NONE = 0,
    ENC_CW,
    ENC_CCW,
    ENC_BUTTON_PRESS,
    ENC_DOUBLE_PRESS,
    ENC_BUTTON_LONG_PRESS
} enc_input_t;

enc_input_t encoder_update(void);
void encoder_callback(void);
void encoderInit(void);


#endif // ROTARY_ENCODER_H
