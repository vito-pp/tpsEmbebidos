#include "drv/matrix.h"

#include "drv/matStream.h"

#define OFF         0x000000u

#define RED(x)     (0x000100u << ((uint8_t)(x)))
#define GREEN(x)   (0x000001u << ((uint8_t)(x)))
#define BLUE(x)    (0x010000u << ((uint8_t)(x)))
#define PURPLE(x)  (0x010100u << ((uint8_t)(x)))

int displayMatrix(char *colour, uint8_t intensity, size_t n)
{
    static uint32_t word[64];
    size_t i;

    if (colour == 0) {
        return 0;
    }

    if (intensity > 7u || n > 64u) {
        return 0;
    }

    for (i = 0; i < 64u; i++) {
        word[i] = OFF;
    }

    for (i = 0; i < n; i++) {
        switch (colour[i]) {
        case 'g':
            word[i] = GREEN(intensity);
            break;

        case 'r':
            word[i] = RED(intensity);
            break;

        case 'b':
            word[i] = BLUE(intensity);
            break;

        case 'p':
            word[i] = PURPLE(intensity);
            break;

        default:
            word[i] = OFF;
            break;
        }
    }

    loadDisplay(word, n);
    WS2812_Update();

    return 1;
}
