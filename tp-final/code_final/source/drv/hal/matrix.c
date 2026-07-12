#include "matrix.h"
#include "../mcal/matStream.h"

#define FLOORS 3

#define OFF     0x000000
#define RED(x) (0x0100 << ((uint8_t)(x)))
#define BLUE(x) (0x010000 << ((uint8_t)(x)))
#define GREEN(x) (0x1 << ((uint8_t)(x)))
#define YELLOW(x) (0x010100 << ((uint8_t)(x)))


uint8_t getOcupation(uint8_t floor);

void reloadMap(void)
{
    int i;
    char id;
    char colour[64];
    for(i= 0; i < 64; i++)
    {
        colour[i] = OFF;
    }
    for(i = 0; i < FLOORS; i++)
    {
        int n = getOcupation(i+1);
        if(n > 4)
        {
            return;
        }
        while(n)
        {
            switch(n)
            {
                case 1: id = 'r'; break;
                case 2: id = 'b'; break;
                case 3: id = 'g'; break;
                case 4: id = 'y'; break;
                default: break;
            }
            colour[(7 - 2*(4-n))*8 + i*2] = id;
            colour[(7 - 2*(4-n))*8 + i*2 + 1] = id;

            colour[(6 - 2*(4-n))*8 + i*2] = id;
            colour[(6 - 2*(4-n))*8 + i*2 + 1] = id;
            n--;
        }
    }

    displayMatrix(colour, 7, 64);
}
//capaz se puede cambiar por  estructura para mas versatilidad... char*
// la intensidad e Z ^ [0,7]
//max intensity = 7.
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
            case 'b': word[i] = YELLOW(intensity); break;
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

    return 1;
}
