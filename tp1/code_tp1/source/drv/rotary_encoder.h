#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdbool.h>
#include <stdint.h>

#define ENCODER_BUFFER_SIZE 5
#define MIN_PRESS_SAMPLES 5

int encoder_state(bool stateA, bool stateB, int encoder_value);

#endif // ROTARY_ENCODER_H
