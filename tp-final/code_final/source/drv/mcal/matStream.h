#ifndef MATSTREAM_H
#define MATSTREAM_H

#include <stdint.h>
#include <stdlib.h>

void WS2812_Update(void);
//n = # of leds to be controlled
//load matrix tu show display
void loadDisplay(uint32_t * colour, size_t n);
void dispBus_init(void);

#endif
