#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>
#include <stdbool.h>

typedef struct{
    uint8_t red;
    uint8_t blue;
    uint8_t green;
} led_t;


int displayMatrix(led_t * leds, size_t n);

#endif