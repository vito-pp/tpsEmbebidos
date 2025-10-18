#include <FXOS8700CQ.h>

#include "i2c.h"

static bool FXOS_ReadWhoAmI(uint8_t i2c_ch, uint8_t *who);

static bool FXOS_Acc_ReadRaw(uint8_t i2c_ch, uint8_t rx[6]);
static void FXOS_Acc_Unpack14b(const uint8_t rx[6], Vec3_t *out14);
static void FXOS_Acc2mg(const Vec3_t *raw14, float mg[3]);

static bool FXOS_Mag_ReadRaw(uint8_t i2c_ch, uint8_t rx[6]);
static void FXOS_Mag_Unpack16b(const uint8_t rx[6], Vec3_t *out16);
static void FXOS_Mag2uT(const Vec3_t *raw16, float uT[3]);

bool FXOS_Init(uint8_t i2c_ch)
{
    // CTRL_REG1 standby: clear ACTIVE (bit 0)
    uint16_t seq1[] = { FXOS_ADDR_W, FXOS8700CQ_CTRL_REG1, 0x00 };
    if (!I2C_MasterSendSequence(i2c_ch, seq1, 3, NULL)) return false;

    // M_CTRL_REG1: Hybrid mode (ACC+MAG). M_HMS bits [1:0] = 0b11.
    // Also oversampling.
    uint16_t seq2[] = { FXOS_ADDR_W, FXOS8700CQ_M_CTRL_REG1, 0x1F };
    if (!I2C_MasterSendSequence(i2c_ch, seq2, 3, NULL)) return false;

    // M_CTRL_REG2: auto-increment for MAG
    uint16_t seq3[] = { FXOS_ADDR_W, FXOS8700CQ_M_CTRL_REG2, 0x20 };
    if (!I2C_MasterSendSequence(i2c_ch, seq3, 3, NULL)) return false;

    // CTRL_REG1: set ODR and ACTIVE=1. For 100 Hz ODR (example): DR=010, 
    // ACTIVE=1 to 0x19
    uint16_t seq4[] = { FXOS_ADDR_W, FXOS8700CQ_CTRL_REG1, 0x19 };
    if (!I2C_MasterSendSequence(i2c_ch, seq4, 3, NULL)) return false;

    return true;
}

static bool FXOS_ReadWhoAmI(uint8_t i2c_ch, uint8_t *who)
{
    uint16_t seq[] = 
    {
        FXOS_ADDR_W, FXOS8700CQ_WHOAMI,
        I2C_RESTART, FXOS_ADDR_R,     
        I2C_READ
    };
    return I2C_MasterSendSequence(i2c_ch, seq, sizeof(seq)/sizeof(seq[0]), who);
}

// read 6 bytes into rx[0..5]
static bool FXOS_Acc_ReadRaw(uint8_t i2c_ch, uint8_t rx[6])
{
    uint16_t seq[] = 
    {
        FXOS_ADDR_W, 0x01,            // sub-address: X_MSB
        I2C_RESTART, FXOS_ADDR_R,     // restart + read address
        I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ   // 6 bytes
    };
    return I2C_MasterSendSequence(i2c_ch, seq, sizeof(seq)/sizeof(seq[0]), rx);
}

static void FXOS_Acc_Unpack14b(const uint8_t rx[6], Vec3_t *out14)
{
    int16_t x = (int16_t)((rx[0] << 8) | rx[1]);
    int16_t y = (int16_t)((rx[2] << 8) | rx[3]);
    int16_t z = (int16_t)((rx[4] << 8) | rx[5]);
    out14->x = x >> 2;
    out14->y = y >> 2;
    out14->z = z >> 2;
}

// mg conversion (+-2g then 4096 counts/g)
static void FXOS_Acc2mg(const Vec3_t *raw14, float mg[3])
{
    mg[0] = (raw14->x * 1000.0f) / 4096.0f;
    mg[1] = (raw14->y * 1000.0f) / 4096.0f;
    mg[2] = (raw14->z * 1000.0f) / 4096.0f;
}

static bool FXOS_Mag_ReadRaw(uint8_t i2c_ch, uint8_t rx[6])
{
    uint16_t seq[] = 
    {
        FXOS_ADDR_W, 0x33,            // sub-address: M_X_MSB
        I2C_RESTART, FXOS_ADDR_R,
        I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ
    };
    return I2C_MasterSendSequence(i2c_ch, seq, sizeof(seq)/sizeof(seq[0]), rx);
}

static void FXOS_Mag_Unpack16b(const uint8_t rx[6], Vec3_t *out16)
{
    out16->x = (int16_t)((rx[0] << 8) | rx[1]);
    out16->y = (int16_t)((rx[2] << 8) | rx[3]);
    out16->z = (int16_t)((rx[4] << 8) | rx[5]);
}

// 0.1 uT/LSB
static void FXOS_Mag2uT(const Vec3_t *raw16, float uT[3])
{
    uT[0] = raw16->x * 0.1f;
    uT[1] = raw16->y * 0.1f;
    uT[2] = raw16->z * 0.1f;
}
