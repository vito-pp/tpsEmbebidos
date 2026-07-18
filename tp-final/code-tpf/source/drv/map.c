#include "drv/map.h"

#include "drv/matrix.h"

#define FLOORS 3u

static uint8_t getOccupation(uint8_t floor)
{
    /*
     * Test version:
     * all floors full, 4 people per floor.
     *
     * Later this should read the real occupancy state.
     */
    switch (floor) {
    case 1:
        return 4;

    case 2:
        return 4;

    case 3:
        return 4;

    default:
        return 0;
    }
}

void loadMap(void)
{
    uint8_t i;
    uint8_t n;
    char id;
    char colour[64];

    for (i = 0; i < 64u; i++) {
        colour[i] = 0;
    }

    for (i = 0; i < FLOORS; i++) {
        n = getOccupation(i + 1u);

        while (n > 0u) {
            switch (n) {
            case 1:
                id = 'r';
                break;

            case 2:
                id = 'b';
                break;

            case 3:
                id = 'g';
                break;

            case 4:
                id = 'p';
                break;

            default:
                id = 0;
                break;
            }

            /*
             * Each person is a 2x2 block.
             *
             * Floor 1: columns 0-1
             * Floor 2: columns 2-3
             * Floor 3: columns 4-5
             */
            colour[(2u * (4u - n)) * 8u + i * 2u] = id;
            colour[(2u * (4u - n)) * 8u + i * 2u + 1u] = id;
            colour[(2u * (4u - n) + 1u) * 8u + i * 2u] = id;
            colour[(2u * (4u - n) + 1u) * 8u + i * 2u + 1u] = id;

            n--;
        }
    }

    displayMatrix(colour, 7u, 64u);
}
