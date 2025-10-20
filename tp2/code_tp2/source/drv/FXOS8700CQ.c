#include "FXOS8700CQ.h"
#include "i2c.h"

#include <math.h>

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} RawData_t;

static uint8_t i2c_channel_id;
static uint8_t read_raw_buff[6]; // to read the acc or the mag
static uint8_t read_raw_buff_double[12]; // to read the acc and the mag
static RawData_t out14;
static RawData_t out16;

/*******************************************************************************
 * FILE SCOPE FUNCTIONS DECLARATIONS
 ******************************************************************************/

static bool FXOS_ReadWhoAmI(uint8_t i2c_ch, uint8_t *who);

static bool FXOS_Acc_ReadRaw(uint8_t i2c_ch, uint8_t rx[6]);
static void FXOS_Acc_Unpack14b(const uint8_t rx[6], RawData_t *out14);
static void FXOS_Acc2mg(const RawData_t *raw14, Vec3_t *mg);

static bool FXOS_Mag_ReadRaw(uint8_t i2c_ch, uint8_t rx[6]);
static void FXOS_Mag_Unpack16b(const uint8_t rx[6], RawData_t *out16);
static void FXOS_Mag2uT(const RawData_t *raw16, Vec3_t *uT);

static bool FXOS_Both_ReadRaw(uint8_t i2c_ch, uint8_t rx[12]);
static void FXOS_Both_Unpack(const uint8_t rx[12], RawData_t *out14, 
                            RawData_t *out16);

/*******************************************************************************
 * GLOBAL SCOPE FUNCTIONS DEFINITIONS
 ******************************************************************************/

bool FXOS_Init(uint8_t i2c_ch, uint32_t baud)
{
	if (!I2C_MasterInit(i2c_ch, baud)) return false;

    // sanity check
    uint8_t who = 0;
    if (!FXOS_ReadWhoAmI(i2c_ch, &who)) return false;
    while(I2C_GetStatus(i2c_ch) != I2C_AVAILABLE)
    {
        if (I2C_GetStatus(i2c_ch) == I2C_ERROR) return false;
#if I2C_POLLING_FLAG
    	I2C_ServicePoll(i2c_ch); // wait for the tx to end
#endif
    }
    if (who != FXOS_WHOAMI_VAL) return false;

    // CTRL_REG1 standby: clear ACTIVE (bit 0)
    uint16_t seq1[] = { FXOS_ADDR_W, FXOS_CTRL_REG1, 0x00 };
    if (!I2C_MasterSendSequence(i2c_ch, seq1, 3, NULL)) return false;
    while(I2C_GetStatus(i2c_ch) != I2C_AVAILABLE)
    {
        if (I2C_GetStatus(i2c_ch) == I2C_ERROR) return false;

#if I2C_POLLING_FLAG
    	I2C_ServicePoll(i2c_ch); // wait for the tx to end
#endif
    }

    // M_CTRL_REG1: Hybrid mode (ACC+MAG). M_HMS bits [1:0] = 0b11.
    // Also oversampling.
    uint16_t seq2[] = { FXOS_ADDR_W, FXOS_M_CTRL_REG1, 0x1F };
    if (!I2C_MasterSendSequence(i2c_ch, seq2, 3, NULL)) return false;
    while(I2C_GetStatus(i2c_ch) != I2C_AVAILABLE)
    {
        if (I2C_GetStatus(i2c_ch) == I2C_ERROR) return false;
#if I2C_POLLING_FLAG
    	I2C_ServicePoll(i2c_ch); // wait for the tx to end
#endif
    }

    // M_CTRL_REG2: auto-increment for MAG
    uint16_t seq3[] = { FXOS_ADDR_W, FXOS_M_CTRL_REG2, 0x20 };
    if (!I2C_MasterSendSequence(i2c_ch, seq3, 3, NULL)) return false;
    while(I2C_GetStatus(i2c_ch) != I2C_AVAILABLE)
    {
        if (I2C_GetStatus(i2c_ch) == I2C_ERROR) return false;

#if I2C_POLLING_FLAG
    	I2C_ServicePoll(i2c_ch); // wait for the tx to end
#endif
    }

    // CTRL_REG1: set ODR and ACTIVE=1. For 100 Hz ODR: DR=010, 
    // ACTIVE=1 to 0x19
    uint16_t seq4[] = { FXOS_ADDR_W, FXOS_CTRL_REG1, 0x19 };
    if (!I2C_MasterSendSequence(i2c_ch, seq4, 3, NULL)) return false;
    while(I2C_GetStatus(i2c_ch) != I2C_AVAILABLE)
    {
        if (I2C_GetStatus(i2c_ch) == I2C_ERROR) return false;

#if I2C_POLLING_FLAG
    	I2C_ServicePoll(i2c_ch); // wait for the tx to end
#endif
    }

    i2c_channel_id = i2c_ch;

    return true;
}

bool FXOS_ReadAccelerometer(Vec3_t* mg)
{
    if (I2C_GetStatus(i2c_channel_id) == I2C_AVAILABLE)
    {
    	FXOS_Acc_ReadRaw(i2c_channel_id, read_raw_buff);
        FXOS_Acc_Unpack14b(read_raw_buff, &out14);
        FXOS_Acc2mg(&out14, mg);
        return true;
    }
    else
    {
        return false;
    }
}

bool FXOS_ReadMagnetometer(Vec3_t* uT)
{
    if (I2C_GetStatus(i2c_channel_id) == I2C_AVAILABLE)
    {
    	FXOS_Mag_ReadRaw(i2c_channel_id, read_raw_buff);
        FXOS_Mag_Unpack16b(read_raw_buff, &out16);
        FXOS_Mag2uT(&out16, uT);
        return true;
    }
    else
    {
        return false;
    }
}

bool FXOS_ReadBoth(Vec3_t *mg, Vec3_t *uT)
{
    if (I2C_GetStatus(i2c_channel_id) == I2C_AVAILABLE)
    {
        FXOS_Both_ReadRaw(i2c_channel_id, read_raw_buff_double);

        FXOS_Both_Unpack(read_raw_buff_double, &out14, &out16);

        FXOS_Acc2mg(&out14, mg);
        FXOS_Mag2uT(&out16, uT);
        return true;
    }
    else
    {
        return false;
    }
}

void vec2rot(Vec3_t *mg, Vec3_t *uT, Rotation_t *rot)
{
    float ax = mg->x, ay = mg->y, az = mg->z;
    float a_norm = sqrtf(ax*ax + ay*ay + az*az);
    if (a_norm == 0) return;

    float pitch = atan2f(ay, az);
    float roll = atan2f(-ax, sqrtf(ay*ay + az*az));

    float sr = sinf(pitch), cr = cosf(pitch);
    float sp = sinf(roll), cp = cosf(roll);

    float mxh = uT->x*cp + uT->z*sp;
    float myh = uT->x*sr*sp + uT->y*cr - uT->z*sr*cp;

    float yaw = atan2f(-myh, mxh);

    const float k = 47.2957795f;
    rot->roll = roll*k;
    rot->pitch = pitch*k;
    rot->yaw = yaw*k;
//    if(rot->yaw < 0) rot->yaw += 360.0f;
}

/*******************************************************************************
 * FILE SCOPE HELPER FUNCTIONS DEFINITIONS
 ******************************************************************************/

static bool FXOS_ReadWhoAmI(uint8_t i2c_ch, uint8_t *who)
{
    uint16_t seq[] =
    {
        FXOS_ADDR_W, FXOS_WHO_AM_I,
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
        FXOS_ADDR_W, FXOS_OUT_X_MSB,           
        I2C_RESTART, FXOS_ADDR_R,     // restart + read address
        I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ   // 6 bytes
    };
    return I2C_MasterSendSequence(i2c_ch, seq, sizeof(seq)/sizeof(seq[0]), rx);
}

static void FXOS_Acc_Unpack14b(const uint8_t rx[6], RawData_t *out14)
{
    int16_t x = (int16_t)((rx[0] << 8) | rx[1]);
    int16_t y = (int16_t)((rx[2] << 8) | rx[3]);
    int16_t z = (int16_t)((rx[4] << 8) | rx[5]);
    out14->x = x >> 2;
    out14->y = y >> 2;
    out14->z = z >> 2;
}

// mg conversion (+-2g then 4096 counts/g)
static void FXOS_Acc2mg(const RawData_t *raw14, Vec3_t *mg)
{
    mg->x = (raw14->x * 1000.0f) / 4096.0f;
    mg->y = (raw14->y * 1000.0f) / 4096.0f;
    mg->z = (raw14->z * 1000.0f) / 4096.0f;
}

static bool FXOS_Mag_ReadRaw(uint8_t i2c_ch, uint8_t rx[6])
{
    uint16_t seq[] = 
    {
        FXOS_ADDR_W, FXOS_M_OUT_X_MSB,
        I2C_RESTART, FXOS_ADDR_R,
        I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ
    };
    return I2C_MasterSendSequence(i2c_ch, seq, sizeof(seq)/sizeof(seq[0]), rx);
}

static void FXOS_Mag_Unpack16b(const uint8_t rx[6], RawData_t *out16)
{
    out16->x = (int16_t)((rx[0] << 8) | rx[1]);
    out16->y = (int16_t)((rx[2] << 8) | rx[3]);
    out16->z = (int16_t)((rx[4] << 8) | rx[5]);
}

static bool FXOS_Both_ReadRaw(uint8_t i2c_ch, uint8_t rx[12])
{
    uint16_t seq[] =
    {
        FXOS_ADDR_W, FXOS_OUT_X_MSB,           
        I2C_RESTART, FXOS_ADDR_R,
        I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ,

        I2C_RESTART, FXOS_ADDR_W, FXOS_M_OUT_X_MSB,
        I2C_RESTART, FXOS_ADDR_R,
        I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ, I2C_READ
    };
    return I2C_MasterSendSequence(i2c_ch, seq, sizeof(seq)/sizeof(seq[0]), rx);
}

static void FXOS_Both_Unpack(const uint8_t rx[12], RawData_t *out14, 
                            RawData_t *out16)
{
    int16_t x = (int16_t)((rx[0] << 8) | rx[1]);
    int16_t y = (int16_t)((rx[2] << 8) | rx[3]);
    int16_t z = (int16_t)((rx[4] << 8) | rx[5]);
    out14->x = x >> 2;
    out14->y = y >> 2;
    out14->z = z >> 2;

    out16->x = (int16_t)((rx[6] << 8) | rx[7]);
    out16->y = (int16_t)((rx[8] << 8) | rx[9]);
    out16->z = (int16_t)((rx[10] << 8) | rx[11]);
}

// 0.1 uT/LSB
static void FXOS_Mag2uT(const RawData_t *raw16, Vec3_t *uT)
{
    uT->x = raw16->x * 0.1f;
    uT->y = raw16->y * 0.1f;
    uT->z = raw16->z * 0.1f;
}
