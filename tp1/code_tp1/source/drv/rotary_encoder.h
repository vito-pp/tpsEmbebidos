#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdbool.h>

#define ENCODER_BUFFER_SIZE 5
#define MIN_PRESS_SAMPLES 5

void encoder_update(void);
static bool encoder_button_update(bool button_state);


#endif // ROTARY_ENCODER_H
