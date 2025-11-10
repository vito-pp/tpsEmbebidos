/***************************************************************************//**
  @file     App.c
  @brief    Test simple: enviar el mismo mensaje continuamente por UART0
*******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "drv/hal/timer.h"
#include "drv/hal/NCO.h"
#include "drv/mcal/board.h"
#include "drv/mcal/gpio.h"
#include "drv/mcal/SysTick.h"
#include "drv/mcal/UART_strings.h"
#include "drv/mcal/UART.h"
#include "drv/mcal/dma.h"
#include "drv/mcal/FTM.h"
#include "drv/mcal/CMP.h"
#include "drv/mcal/pit.h"
#include "drv/mcal/DAC.h"
#include "drv/mcal/DECODE_V2.h"
#include "drv/mcal/bitstream.h"

#define PARITY UART_PARITY_ODD

#define RX_BUFFER_SIZE 2048
#define TX_BUFFER_SIZE 2048
/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/

 static unsigned char idle_counter = 0;
		static bool last_byte_idle = false;

		
static bool bit_stream[11];
static uint16_t data_stream[20];
static bool finished = 0;

// Circular buffer for transmission data
static char tx_buffer[TX_BUFFER_SIZE];
static volatile size_t tx_head = 0; 
static volatile size_t tx_tail = 0;

static char rx_line[2048];

static NCO_Handle nco_handle;

static char received_data[2048];


// Bitstream de datos enviados por el DAC
static bool idle_sending_bitstream[11];
static bool idle_reciving_bitstream[11];
static bool sending_bitstream[11]; // 11?

// Bitstream de datos recibidos por el ADC
static bool reciving_bitstream[11]; // 11?

static uint16_t lut_value;
static size_t bit_cnt;
static size_t cnt;
static bool initiate_send = false;
static bool sending_data = false;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
bool validateBitStream(bool* in);
static void NCO_ISRLut(void* user);
static void NCO_ISRBit(void *user);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
bool finishStatus(void);
void finishedReading(void);
void clearFinished(void);
void uint16_to_bin(uint16_t value, char *out, size_t out_len);

static void ftm_cb(void* user);
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	UART_Init(UART_PARITY_ODD);
	
	CMP_Init();
	FTM_Init();

	PIT_Init();

	gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
	gpioMode(PORTNUM2PIN(PB,2), OUTPUT);
	gpioWrite(PORTNUM2PIN(PB,2), 0);
	gpioMode(PORTNUM2PIN(PB,3), OUTPUT);
	gpioWrite(PORTNUM2PIN(PB,3), 0);

	pit_cfg_t pit_cfg =
 	 {
		.ch = 2,
		.load_val = PIT_TICKS_FROM_US(833),
		.periodic = true,
		.int_en = true,
		.callback = ftm_cb,
		.user = NULL
 	 };
	PIT_Config(&pit_cfg);
	PIT_Stop(2);

	pit_cfg_t pit_cfg_lut =
    {
        .ch = 0,
        .load_val = PIT_TICKS_FROM_US(20),
        .periodic = true,
        .int_en = true,
        .dma_req = false,
        .callback = NCO_ISRLut,
        .user = NULL
    };
	PIT_Config(&pit_cfg_lut);

	pit_cfg_t pit_cfg_bit =
    {
        .ch = 1,
        .load_val = PIT_TICKS_FROM_US(833),
        .periodic = true,
        .int_en = true,
        .dma_req = false,
        .callback = NCO_ISRBit,
        .user = NULL
    };

    PIT_Config(&pit_cfg_bit);
	
	NCO_InitFixed(&nco_handle, K_MARK, K_SPACE, true);

    // DAC
    DAC_Init();

	// Inicializo los bitstreams en idle
    for (int i = 0; i < 11; i++)
    {
        idle_sending_bitstream[i] = true;
        idle_reciving_bitstream[i] = 0;
    }

}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{

    UART_Poll();
    int r = UART_ReceiveString(rx_line, sizeof(rx_line));

		if (r > 0)
		{
			// Queue all the recieved characters on the ring buffer
			for (int i = 0; i < r; i++)
			{
				size_t next_head = (tx_head + 1) % TX_BUFFER_SIZE;

				if (next_head != tx_tail)
				{
					tx_buffer[tx_head] = rx_line[i];
					tx_head = next_head;
				}
			}

		}

		// If we aren't recieving data, send whats in the buffer
		if (!sending_data && tx_head != tx_tail)
    {
        if (idle_counter == 4){
            last_byte_idle = true;
            idle_counter = 0;
        }
		
        if (last_byte_idle)
        {
            format_bitstream(tx_buffer[tx_tail], sending_bitstream);
            tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;

            initiate_send = true;
            sending_data = true;

        }


    }

    UART_Poll();


//--------------------------------------------------------------------
//--------------------------------------------------------------------
	if (bitStartDetected())
	{


		setReadingFlag();
		clearFinished();
		//gpioToggle(PORTNUM2PIN(PB,3));
		PIT_Start(2);
		//gpioToggle(PORTNUM2PIN(PB,3));
	}
	//bit_stream[0] +=0;
	static char palabras[100];
	static int k;

	if(finishStatus())
	{
		clearFinished();
		//bit_stream[0] += 0;


		//bool fake[11] = {0,1,1,0,0,1,1,1,0,0,1};
		 // placeholder
		//received_data[1] = 0;
		//bool bit_stream1[] = {0,1,1,0,1,0,1,1,0,0,1};
		received_data[0] = deformat_bitstream(bit_stream);
		if(validateBitStream(bit_stream))
		{
			UART_SendString(received_data);
		}


		}
	if(k==100)
	{
		k=0;
	}

}

/*******************************************************************************
 *******************************************************************************
    LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
bool validateBitStream(bool* in)
{
	int i;
	int count = 0;
	if(in[10] != 1)
	{
		return 0;
	}
	for(i=1; i < 10;i++)
	{
		count+= in[i];
	}
	return (count%2);
}
static void delayLoop(uint32_t veces)
{
    while (veces--);
}

static void __error_handler__(void)
{
    gpioWrite(PIN_LED_RED, LED_ACTIVE);
}

void uint16_to_bin(uint16_t value, char *out, size_t out_len){
    if (!out || out_len == 0)
        return;

    const size_t total_bits = 16;
    size_t need = total_bits + 1; // +1 para '\0'

    if (out_len < need) {
        size_t bits = out_len - 1; // cuantos bits podemos escribir
        for (size_t i = 0; i < bits; ++i) {
            size_t shift = bits - 1 - i;
            out[i] = ((value >> shift) & 1U) ? '1' : '0';
        }
        out[bits] = '\0';
        return;
    }

    for (int i = 0; i < (int)total_bits; ++i) {
        int shift = (int)total_bits - 1 - i; // MSB first
        out[i] = ((value >> shift) & 1U) ? '1' : '0';
    }
    out[total_bits] = '\0';
}

static void NCO_ISRBit(void* user)
{
    // Flag de reset de Contador de bits enviados (Redundancia para seguridad)
    if (initiate_send)
    {
        bit_cnt = 0;
        initiate_send = false;
    }

    // Decidir si envio en idle o si envio datos
    if (sending_data)
    {
        NCO_FskBit(&nco_handle, sending_bitstream[bit_cnt]);
    }
    else
    {

        NCO_FskBit(&nco_handle, idle_sending_bitstream[bit_cnt]);
        if (!last_byte_idle){
            idle_counter++;
        }
    }

    bit_cnt++;
    // Se setearon bit_cnt bits en el NCO.
    if (bit_cnt == 11)
    {
        if (sending_data){
            last_byte_idle = false;
        }
        sending_data = false;
        bit_cnt = 0;
    }
}

static void NCO_ISRLut(void* user)
{

    lut_value = NCO_TickQ15(&nco_handle);
    PWM_setDuty(NCO2PWM(lut_value));

}


static void ftm_cb(void* user)
{
	//gpioToggle(PORTNUM2PIN(PB,3));
	static uint8_t cnt = 1;
	bit_stream[cnt]  = processBit();

	cnt++;
	if (cnt == 11)
	{
		PIT_Stop(2);
		cnt = 1;
		clearReadingFlag();
		finishedReading();
		IC_clearBitStart();
		//gpioToggle(PORTNUM2PIN(PB,2));
	}
	//gpioToggle(PORTNUM2PIN(PB,3));
}

bool finishStatus(void)
{
	return finished;
}
void finishedReading(void)
{
	finished=1;
}
void clearFinished(void)
{
	finished = 0;
}
