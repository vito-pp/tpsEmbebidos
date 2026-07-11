#include "matrix.h"
#include "../mcal/matStream.h"

void updateDisplay(void);
//capaz se puede cambiar por  estructura para mas versatilidad... char*
// la intensidad e Z ^ [0,7]
//max intensity = 7.
int displayMatrix(char * colour, uint8_t intensity ,size_t n)
{
    uint32_t word[64];
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
            case 'g': word[i] = 0x010000 << intensity; break;
            case 'r': word[i] = 0x0100 << intensity; break;
            case 'b': word[i] = 0x1 << intensity; break;
            default: return 0; break;
        }
        /*
        colour[i] = (led[i].green << 16 ) | 
                    (led[i].red <<8 )   | 
                    led[i].blue;
        */
    }

    loadDisplay(word, n);
    WS2812_Update();

    //delay?

    return 1;
}
