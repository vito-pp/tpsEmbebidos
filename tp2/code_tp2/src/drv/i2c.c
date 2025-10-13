#include "i2c.h"
#include "../../SDK/CMSIS/MK64F12.h" 
#include "gpio.h"

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

static I2C_Type *const I2C_channels[] = I2C_BASE_PTRS;
static IRQn_Type const I2C_IRQn[] = I2C_IRQS;

// SCL divider table for MK64F12 (from Reference Manual, 0x00–0x3F)
static const uint16_t scl_div[] = {
    20,   22,   24,   26,   28,   30,   34,   40,   28,   32,   36,   40,   44,   48,   56,   68,
    48,   56,   64,   72,   80,   88,  104,  128,   80,   96,  112,  128,  144,  160,  192,  240,
    160,  192,  224,  256,  288,  320,  384,  480,  320,  384,  448,  512,  576,  640,  768,  960,
    640,  768,  896, 1024, 1152, 1280, 1536, 1920, 1280, 1536, 1792, 2048, 2304, 2560, 3072, 3840
};

static void setBaudRate(I2C_Type *i2c, uint32_t bus_clk, uint32_t baud);

bool I2C_MasterInit(uint8_t channel, uint16_t baud_rate)
{
    if (channel >= FSL_FEATURE_SOC_I2C_COUNT
        || baud_rate > FSL_FEATURE_I2C_MAX_BAUD_KBPS) 
    {
        return false;
    }

    I2C_Type *i2c = I2C_channels[channel];

    // 1) Gate clocks
    // SDA/SCL ports
    switch (channel)
    {
        case 0: SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; break;
        case 1: SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK; break;
        case 2: SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK; break;
	}

    // I2C module
	switch (channel)
    {
        case 0: SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK; break;
        case 1: SIM->SCGC4 |= SIM_SCGC4_I2C1_MASK; break;
        case 2: SIM->SCGC1 |= SIM_SCGC1_I2C2_MASK; break;
	}

	// 2) Port configuration (pin mux to I2C + open-drain)
    // External pull-ups required!!!
    switch (channel)
    {
        case 0: 
            //first set MUX & ODE bits to 0
            PORTE->PCR[PIN2NUM(I2C0_SCL_PIN)] &= ~(PORT_PCR_MUX(0b111) 
                                                | PORT_PCR_ODE(1));
            // then write
            PORTE->PCR[PIN2NUM(I2C0_SCL_PIN)] |= PORT_PCR_MUX(I2C0_ALT) 
                                                | PORT_PCR_ODE(1);
            PORTE->PCR[PIN2NUM(I2C0_SDA_PIN)] &= ~(PORT_PCR_MUX(0b111) 
                                                | PORT_PCR_ODE(1));
            PORTE->PCR[PIN2NUM(I2C0_SDA_PIN)] |= PORT_PCR_MUX(I2C0_ALT) 
                                                | PORT_PCR_ODE(1);            
            break;
        case 1: 
            PORTC->PCR[PIN2NUM(I2C1_SCL_PIN)] &= ~(PORT_PCR_MUX(0b111) 
                                                | PORT_PCR_ODE(1));
            PORTC->PCR[PIN2NUM(I2C1_SCL_PIN)] |= PORT_PCR_MUX(I2C1_ALT) 
                                                | PORT_PCR_ODE(1);
            PORTC->PCR[PIN2NUM(I2C1_SDA_PIN)] &= ~(PORT_PCR_MUX(0b111) 
                                                | PORT_PCR_ODE(1));
            PORTC->PCR[PIN2NUM(I2C1_SDA_PIN)] |= PORT_PCR_MUX(I2C1_ALT) 
                                                | PORT_PCR_ODE(1);
            break;
        case 2: 
            PORTA->PCR[PIN2NUM(I2C2_SCL_PIN)] &= ~(PORT_PCR_MUX(0b111) 
                                                | PORT_PCR_ODE(1));
            PORTA->PCR[PIN2NUM(I2C2_SCL_PIN)] |= PORT_PCR_MUX(I2C2_ALT) 
                                                | PORT_PCR_ODE(1);
            PORTA->PCR[PIN2NUM(I2C2_SDA_PIN)] &= ~(PORT_PCR_MUX(0b111) 
                                                | PORT_PCR_ODE(1));
            PORTA->PCR[PIN2NUM(I2C2_SDA_PIN)] |= PORT_PCR_MUX(I2C2_ALT) 
                                                | PORT_PCR_ODE(1);
            break;
    }

    // 3) Module configuration
	setBaudRate(i2c, 25000000, baud_rate); // falta ver cuanto vale el bus clock
	i2c->C1 = 0;
	i2c->C1 |= I2C_C1_IICEN_MASK; // enable the module (enable after tweaking 
    // other registers)
    // ToDo: enable interrupts (IICEI)
	// NVIC_EnableIRQ(I2C_IRQn[channel]);
    // ToDo: add glitch-filters, high-drive, timeouts, etc.

    return true;
}

void I2C_MasterTx(uint8_t channel, uint8_t slave_address, uint8_t *data2send,
                    size_t len);

void I2C_MasterRx(uint8_t channel, uint8_t slave_address, uint8_t *data2read, 
                    size_t len);

void I2C_MasterTxRx(uint8_t channel, uint8_t slave_address, uint8_t *data2send, 
                    size_t send_len, uint8_t *data2read, size_t read_len);


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