#include <stdint.h>
#include <stdbool.h>

// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR   0x1D // with pins SA0=1, SA1=0

// FXOS8700CQ internal register addresses
#define FXOS8700CQ_STATUS       0x00
#define FXOS8700CQ_WHOAMI       0x0D
#define FXOS8700CQ_XYZ_DATA_CFG 0x0E
#define FXOS8700CQ_CTRL_REG1    0x2A
#define FXOS8700CQ_M_CTRL_REG1  0x5B
#define FXOS8700CQ_M_CTRL_REG2  0x5C
#define FXOS8700CQ_WHOAMI_VAL   0xC7

// number of bytes to be read from the FXOS8700CQ
#define FXOS8700CQ_READ_LEN 13 /* status (1 byte) plus 6 channels (6 * 16-bit) 
= 13 bytes */

#define FXOS_ADDR_W      ((FXOS8700CQ_SLAVE_ADDR << 1) | 0)  // 0x3C
#define FXOS_ADDR_R      ((FXOS8700CQ_SLAVE_ADDR << 1) | 1)  // 0x3D

typedef struct
{
int16_t x;
int16_t y;
int16_t z;
} Vec3_t;

bool FXOS_Init(uint8_t i2c_ch);
