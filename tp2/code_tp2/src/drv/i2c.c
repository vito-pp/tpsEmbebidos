/**
 * Based on nxp SDK and Jan Rychten I2C drivers.
 */

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "i2c.h"
#include "../../SDK/CMSIS/MK64F12.h" 
#include "gpio.h"
#include "../startup/hardware.h"

/*******************************************************************************
 * CONSTANTS AND DEFINES
 ******************************************************************************/

#define I2C0_SDA_PIN	PORTNUM2PIN(PE,25)
#define I2C0_SCL_PIN	PORTNUM2PIN(PE,24)
#define I2C1_SDA_PIN	PORTNUM2PIN(PC,11)
#define I2C1_SCL_PIN	PORTNUM2PIN(PC,10)
#define I2C2_SDA_PIN	PORTNUM2PIN(PA,13) // Not in FRDM-K64F 
#define I2C2_SCL_PIN	PORTNUM2PIN(PA,14) // Not in FRDM-K64F

#define I2C0_ALT	5
#define I2C1_ALT	2
#define I2C2_ALT	5

#define F_MUL_VALUE	0x2		// mul = 4

static I2C_Type *const I2C_base_ptrs[] = I2C_BASE_PTRS;
static IRQn_Type const I2C_IRQn[] = I2C_IRQS;

// SCL divider table for MK64F12 (from Reference Manual, 0x00–0x3F)
static const uint16_t scl_div[] = 
{
    20,   22,   24,   26,   28,   30,   34,   40,   28,   32,   36,   40,   44,   48,   56,   68,
    48,   56,   64,   72,   80,   88,  104,  128,   80,   96,  112,  128,  144,  160,  192,  240,
    160,  192,  224,  256,  288,  320,  384,  480,  320,  384,  448,  512,  576,  640,  768,  960,
    640,  768,  896, 1024, 1152, 1280, 1536, 1920, 1280, 1536, 1792, 2048, 2304, 2560, 3072, 3840
};

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum
{
    I2C_TX,
    I2C_RX
} I2C_Mode_e;

typedef struct I2C_Buffer_t
{
    uint16_t *sequence;
    uint32_t sequence_end;
    uint8_t *recieve_buffer;
    uint8_t reads_ahead;
    I2C_Status_e status;
    I2C_Mode_e mode;
} I2C_Buffer_t;

/*******************************************************************************
 * LOCAL FUNCTIONS PROTOTYPES
 ******************************************************************************/

static void setBaudRate(I2C_Type *i2c, uint32_t bus_clk, uint32_t baud);
static void I2C_IRQHandler(uint8_t channel_id);

/*******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************/

volatile static I2C_Buffer_t I2C_channel[I2C_NUMBER_OF_CHANNELS];

/*******************************************************************************
 * FUNCTION DEFINITIONS WITH GLOBAL SCOPE
 ******************************************************************************/

bool I2C_MasterInit(uint8_t channel_id, uint16_t baud_rate)
{
    if (channel_id >= I2C_NUMBER_OF_CHANNELS
        || baud_rate > FSL_FEATURE_I2C_MAX_BAUD_KBPS * 1000) 
    {
        return false;
    }

    I2C_Type *i2c = I2C_base_ptrs[channel_id];

    // 1) Gate clocks
    // SDA/SCL ports
    switch (channel_id)
    {
        case 0: SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; break;
        case 1: SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK; break;
        case 2: SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK; break;
	}

    // I2C module
	switch (channel_id)
    {
        case 0: SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK; break;
        case 1: SIM->SCGC4 |= SIM_SCGC4_I2C1_MASK; break;
        case 2: SIM->SCGC1 |= SIM_SCGC1_I2C2_MASK; break;
	}

	// 2) Port configuration (pin mux to I2C + open-drain)
    // External pull-ups required!!!
    switch (channel_id)
    {
        case 0: 
            PORTE->PCR[PIN2NUM(I2C0_SCL_PIN)] |= PORT_PCR_MUX(I2C0_ALT) 
                                                | PORT_PCR_ODE_MASK;

            PORTE->PCR[PIN2NUM(I2C0_SDA_PIN)] |= PORT_PCR_MUX(I2C0_ALT) 
                                                | PORT_PCR_ODE_MASK;            
            break;
        case 1: 
            PORTC->PCR[PIN2NUM(I2C1_SCL_PIN)] |= PORT_PCR_MUX(I2C1_ALT) 
                                                | PORT_PCR_ODE_MASK;

            PORTC->PCR[PIN2NUM(I2C1_SDA_PIN)] |= PORT_PCR_MUX(I2C1_ALT) 
                                                | PORT_PCR_ODE_MASK;
            break;
        case 2: 
            PORTA->PCR[PIN2NUM(I2C2_SCL_PIN)] |= PORT_PCR_MUX(I2C2_ALT) 
                                                | PORT_PCR_ODE_MASK;

            PORTA->PCR[PIN2NUM(I2C2_SDA_PIN)] |= PORT_PCR_MUX(I2C2_ALT) 
                                                | PORT_PCR_ODE_MASK;
            break;
    }

    // 3) Module configuration
	setBaudRate(i2c, (__CORE_CLOCK__)/2, baud_rate); // Bus' clk is half core's
    // ToDo (more coplex config): add glitch-filters, high-drive, timeouts, etc.
	i2c->C1 = 0;
	i2c->C1 |= I2C_C1_IICEN_MASK; // enable the module (enable last)
#if !I2C_POLLING_FLAG
    i2c->C1 |= I2C_C1_IICIE_MASK; // enable interrupts
	NVIC_EnableIRQ(I2C_IRQn[channel_id]); // enable interrputs in NVIC
#endif
    return true;
}

bool I2C_MasterSendSequence(uint8_t channel_id, uint16_t *sequence, 
                            uint32_t len, uint8_t *recieve_buffer)
{
    if (channel_id >= I2C_NUMBER_OF_CHANNELS) 
    {
        return false;
    }

    I2C_Type *i2c = I2C_base_ptrs[channel_id];
    volatile I2C_Buffer_t *channel =  &(I2C_channel[channel_id]);

    if (channel->status == I2C_BUSY)
    {
        return false;
    }

    channel->sequence;
    channel->sequence_end = sequence + len;
    channel->recieve_buffer = recieve_buffer;
    channel->status = I2C_BUSY;
    channel->mode = I2C_TX;

    // Generate a start condition and prepare for transmitting
    i2c->C1 |= (I2C_C1_MST_MASK | I2C_C1_TX_MASK);

    if (i2c->S & I2C_S_ARBL_MASK) // Lost of arbitration
    {
        // Clean everything and leave
        i2c->C1 &= ~(I2C_C1_MST_MASK | I2C_C1_TX_MASK);
        channel->status = I2C_ERROR;
        return false;
    }

    i2c->D = *(channel->sequence)++; // Writes the first byte
    // The ISR handler will take care of the rest. If I2C_POLLING_FLAG is set to true ... (ToDo add I2C_Update() func for polling)
    return true;
}

I2C_Status_e I2C_GetStatus(uint8_t channel_id)
{
    return I2C_channel[channel_id].status;
}

/*******************************************************************************
 * FUNCTION DEFINITIONS WITH FILE SCOPE
 ******************************************************************************/

static void setBaudRate(I2C_Type *i2c, uint32_t bus_clk, uint32_t baud)
{
    uint8_t best_icr = 0;
    uint8_t best_mult = 0;
    uint32_t best_diff = 0xFFFFFFFF;

    for (uint8_t mult = 0; mult < 3; mult++) 
    {
        uint32_t mul_val = 1 << mult; // 1, 2, 4
        for (uint8_t icr = 0; icr < 0x40; icr++) 
        {
            uint32_t calc_baud = bus_clk / (mul_val * scl_div[icr]);
            uint32_t diff = (baud > calc_baud) ? baud - calc_baud : calc_baud - baud;
            if (diff < best_diff) 
            {
                best_diff = diff;
                best_icr = icr;
                best_mult = mult;
            }
        }
    }
    i2c->F = I2C_F_MULT(best_mult) | I2C_F_ICR(best_icr);
}       

static void I2C_IRQHandler(uint8_t channel_id)
{
    volatile I2C_Buffer_t *channel = &(I2C_channel[channel_id]);
    I2C_Type* i2c = I2C_base_ptrs[channel_id];
    uint16_t element;

    uint8_t status = i2c->S;

    if (!(status & I2C_S_IICIF_MASK))
    {   // the IICF was trigg'd by another module
        return;
    }

    i2c->S |= I2C_S_IICIF_MASK; // Acknowledge the IRQ

    if (status & I2C_S_ARBL_MASK)
    {
        i2c->S |= I2C_S_ARBL_MASK;
        i2c->C1 &= ~(I2C_C1_MST_MASK | I2C_C1_IICIE_MASK); /* Generate STOP and
        disable further interrupts. */
        channel->status = I2C_ERROR;
    }

    if (channel->mode == I2C_TX)
    {
        if (channel->sequence == channel->sequence_end) // End of transmit
        {
            /* Generate STOP (set MST=0), switch to RX mode, and disable 
            further interrupts. */
            i2c->C1 &= ~(I2C_C1_MST_MASK | I2C_C1_IICIE_MASK 
                        |I2C_C1_TXAK_MASK);
            channel->status = I2C_AVAILABLE;
            return; // Success
        }

        if (status & I2C_S_RXAK_MASK) // NACK recieved
        {
            i2c->C1 &= ~(I2C_C1_MST_MASK | I2C_C1_IICIE_MASK); /* Generate STOP 
            and disable further interrupts. */
            channel->status = I2C_ERROR;
            return; // Error
        }

        element = *channel->sequence;

        if (element == I2C_RESTART) // Do we have a reapeted start?
        {
            i2c->C1 |= (I2C_C1_RSTA_MASK | I2C_C1_TX_MASK); /* Generate a 
            repeated start condition and switch to TX */
            channel->sequence++;
            element = *channel->sequence;
            i2c->D = element;
        }
        else
        {
            if (element == I2C_READ)
            {
                channel->mode = I2C_RX;
                // We need to count how many bytes to read
                channel->reads_ahead = 1;
                while(((channel->sequence + channel->reads_ahead) <
                        channel->sequence_end) && (*(channel->sequence + 
                        channel->reads_ahead) == I2C_READ))
                {
                    channel->reads_ahead++;
                }
                i2c->C1 &= ~I2C_C1_TX_MASK; // Switch to RX mode

                if (channel->reads_ahead == 1)
                {
                    i2c->C1 |= I2C_C1_TXAK_MASK; // do not ACK the final read
                }
                else
                {
                    i2c->C1 &= ~I2C_C1_TX_MASK; // ACK but the final read
                }
                /* Now comes a dummy read. Thats why we dont increment the 
                recieve_buffer pointer*/
                *channel->recieve_buffer = i2c->D;
                channel->reads_ahead--;
            }
            else // Not a restart, not a read, must be a write
            {
                i2c->D = element;
            }
        }
    }

    else    // RX mode
    {
        switch (channel->reads_ahead)
        {
            case 0: /* Last byte to be read. Need to switch to TX to send a Sr 
            or to avoid to get another I2C read */
                i2c->C1 |= I2C_C1_TX_MASK;
                *channel->recieve_buffer++ = i2c->D; // Last read

                if ((channel->sequence < channel->sequence_end) &&
                    (*channel->sequence == I2C_RESTART)) // Sr?
                {
                    i2c->C1 |= I2C_C1_RSTA_MASK; // Generate a Sr
                    channel->mode = I2C_TX;
                    channel->sequence++;
                    element = *channel->sequence;
                    i2c->D = element;
                }
                else
                {
                    /* Generate STOP (set MST=0), switch to RX mode, and 
                    disable further interrupts */
                    i2c->C1 &= ~(I2C_C1_MST_MASK | I2C_C1_IICIE_MASK 
                                |I2C_C1_TXAK_MASK);
                    channel->status = I2C_AVAILABLE;
                    return;
                }
                break;

            case 1:
                i2c->C1 |= I2C_C1_TXAK_MASK; // Do not ACK the final read
                *channel->recieve_buffer++ = i2c->D;
                break;

            default:
                *channel->recieve_buffer++ = i2c->D;
                break;
        }

        channel->reads_ahead--;
    }
    channel->sequence++;
    return;
}

/*******************************************************************************
 * ISR Handlers
 ******************************************************************************/

__ISR__ I2C0_IRQHandler(void)
{
    I2C_IRQHandler(0);
}

__ISR__ I2C1_IRQHandler(void)
{
    I2C_IRQHandler(1);
}

__ISR__ I2C2_IRQHandler(void)
{
    I2C_IRQHandler(2);
}