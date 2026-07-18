#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


/**
 * @brief Displays a color pattern on the WS2812 LED matrix.
 *
 * Converts a character-based color map into 24-bit RGB values, loads the
 * display buffer, and starts the DMA transmission to the LED matrix.
 *
 * Supported color codes:
 *   - 'r' : Red
 *   - 'g' : Green
 *   - 'b' : Blue
 *   - 'p' : Purple
 *   - Any other value leaves the LED off.
 *
 * @param colour     Pointer to an array containing the color codes.
 * @param intensity  Brightness level (0-7).
 * @param n          Number of LEDs to update (maximum 64).
 *
 * @return 1 if the display update was successfully started.
 * @return 0 if the input parameters are invalid.
 */
int displayMatrix(char * colour, uint8_t intensity ,size_t n);


#endif
