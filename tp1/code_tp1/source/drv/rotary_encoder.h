#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdbool.h>
#include <stdint.h>

#define ENCODER_BUFFER_SIZE 5
#define MIN_PRESS_CYCLES 50

#define LONG_PRESS_TIME 2 //Seconds
#define INTERRUPT_FREQUENCY 2000 //Hz
#define LONG_PRESS_CYCLES ((INTERRUPT_FREQUENCY * LONG_PRESS_TIME)*ENCODER_BUFFER_SIZE) //Ciclos

uint8_t encoder_update(void);
void encoder_callback(void);

enum {
    ENC_NONE = 0,
    ENC_CW,
    ENC_CCW,
    ENC_BUTTON_PRESS,
    ENC_BUTTON_LONG_PRESS
};

#endif // ROTARY_ENCODER_H
