#ifndef _CODIFY_CAN_BUS_
#define _CODIFY_CAN_BUS_

#include <stdint.h>

#include "../drv/FXOS8700CQ.h"

#define NUM_STATIONS 4

typedef struct
{
    uint8_t id;
    Rotation_t rot;
} Station_t;

Station_t stations[NUM_STATIONS];

decodeDataForCan(uint8_t *rx_buff, size_t len, Station_t *stations);

codeDataForCan(uint8_t *tx, Rotation_t *rot);

#endif