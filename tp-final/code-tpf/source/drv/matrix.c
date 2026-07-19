#include "matrix.h"
#include "matStream.h"

#define OFF     0x000000
#define RED(x) (0x0100 << ((uint8_t)(x)))
#define BLUE(x) (0x010000 << ((uint8_t)(x)))
#define GREEN(x) (0x1 << ((uint8_t)(x)))
#define PURPLE(x) (0x010100 << ((uint8_t)(x)))
#define WHITE   0xFFF




int displayMatrix(char * colour, uint8_t intensity ,size_t n)
{
    
    static uint32_t word[64];
    int i;
    if(intensity > 7 || n > 64)
    {
        return 0;
    }
    for(i = 0; i < n; i++)
    {
        switch(colour[i])
        {
            //añadir shift para intensidad}
            case 'g': word[i] = GREEN(intensity); break;
            case 'r': word[i] = RED(intensity); break;
            case 'b': word[i] = BLUE(intensity); break;
            case 'p': word[i] = PURPLE(intensity); break;
            case 'w': word[i] = WHITE; break;
            default:  break;
        }
    }

    loadDisplay(word, n);
    WS2812_Update();

    return 1;
}
