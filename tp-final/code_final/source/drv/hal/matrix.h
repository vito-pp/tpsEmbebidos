#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct{
    uint8_t red;
    uint8_t blue;
    uint8_t green;
} led_t;


int displayMatrix(char * colour, uint8_t intensity ,size_t n);

#endif
