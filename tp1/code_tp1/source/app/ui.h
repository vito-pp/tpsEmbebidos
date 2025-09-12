#include <stdint.h>

#include "fsm.h"
#include "../drv/mag_strip.h"
#include "../drv/rotary_encoder.h"

// returns id read from the magnetic strip
uint32_t readIdMagStrip(void);

// returns event read from the rotary encoder
FSM_event_t encoderInput(void);