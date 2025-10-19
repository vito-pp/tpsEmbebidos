#ifndef _FXOS8700CQ_H_
#define _FXOS8700CQ_H_

#include <stdint.h>
#include <stdbool.h>

// FXOS8700CQ I2C address
#define FXOS_SLAVE_ADDR     0x1D // with pins SA0=1, SA1=0

// FXOS8700CQ internal register addresses
#define FXOS_STATUS         0x00 
#define FXOS_SYSMOD         0x0B 
#define FXOS_WHO_AM_I       0x0D 
#define FXOS_XYZ_DATA_CFG   0x0E

#define FXOS_OUT_X_MSB      0x01 
#define FXOS_OUT_X_LSB      0x02 
#define FXOS_OUT_Y_MSB      0x03 
#define FXOS_OUT_Y_LSB      0x04 
#define FXOS_OUT_Z_MSB      0x05 
#define FXOS_OUT_Z_LSB      0x06 
#define FXOS_CTRL_REG1      0x2A 
#define FXOS_CTRL_REG2      0x2B 
#define FXOS_CTRL_REG3      0x2C 
#define FXOS_CTRL_REG4      0x2D 
#define FXOS_CTRL_REG5      0x2E 

#define FXOS_M_STATUS       0x32 
#define FXOS_M_OUT_X_MSB    0x33 
#define FXOS_M_OUT_X_LSB    0x34 
#define FXOS_M_OUT_Y_MSB    0x35 
#define FXOS_M_OUT_Y_LSB    0x36 
#define FXOS_M_OUT_Z_MSB    0x37 
#define FXOS_M_OUT_Z_LSB    0x38 
#define FXOS_M_CTRL_REG1    0x5B 
#define FXOS_M_CTRL_REG2    0x5C 
#define FXOS_M_CTRL_REG3    0x5D 

// useful macros
#define FXOS_WHOAMI_VAL 0xC7

// number of bytes to be read from the FXOS8700CQ
#define FXOS_READ_LEN 13 /* status (1 byte) plus 6 channels (6 * 16-bit) 
= 13 bytes */

#define FXOS_ADDR_W      ((FXOS_SLAVE_ADDR << 1) | 0)  // 0x3A
#define FXOS_ADDR_R      ((FXOS_SLAVE_ADDR << 1) | 1)  // 0x3B

typedef struct
{
float x;
float y;
float z;
} Vec3_t;

bool FXOS_Init(uint8_t i2c_ch);

bool FXOS_ReadAccelerometer(Vec3_t *mg);

bool FXOS_ReadMagnetometer(Vec3_t *uT);

#endif // _FXOS8700CQ_H_