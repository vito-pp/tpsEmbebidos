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
#include "drv/mcal/board.h"
#include "drv/mcal/gpio.h"
#include "drv/mcal/SysTick.h"
#include "drv/mcal/UART_strings.h"
#include "drv/mcal/UART.h"
#include "drv/mcal/dma.h"
#include "drv/mcal/FTM.h"
#include "drv/mcal/CMP.h"
#include "drv/mcal/pit.h"
#include "drv/mcal/DECODE_V2.h"
#include "drv/mcal/bitstream.h"

  void uint16_to_bin(uint16_t value, char *out, size_t out_len);

void uint16_to_bin(uint16_t value, char *out, size_t out_len)
{
    if (!out || out_len == 0)
        return;

    const size_t total_bits = 16;
    size_t need = total_bits + 1; // +1 para '\0'

    if (out_len < need)
    {
        size_t bits = out_len - 1; // cuantos bits podemos escribir
        for (size_t i = 0; i < bits; ++i)
        {
            size_t shift = bits - 1 - i;
            out[i] = ((value >> shift) & 1U) ? '1' : '0';
        }
        out[bits] = '\0';
        return;
    }

    for (int i = 0; i < (int)total_bits; ++i)
    {
        int shift = (int)total_bits - 1 - i; // MSB first
        out[i] = ((value >> shift) & 1U) ? '1' : '0';
    }
    out[total_bits] = '\0';
}

/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/
bool bit_stream[11];
uint16_t data_stream[20];
bool finished = 0;
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
bool finishStatus(void);
bool finishStatus(void)
{
	return finished;
}
void finishedReading(void);
void finishedReading(void)
{
	finished=1;
}
void clearFinished(void);
void clearFinished(void)
{
	finished = 0;
}
static void ftm_cb(void* user);
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	UART_Init(UART_PARITY_ODD);
	CMP_Init();
	FTM_Init();
	PIT_Init();
	gpioMode(PORTNUM2PIN(PB,2), OUTPUT);
	gpioWrite(PORTNUM2PIN(PB,2), 0);
	gpioMode(PORTNUM2PIN(PB,3), OUTPUT);
	gpioWrite(PORTNUM2PIN(PB,3), 0);

	pit_cfg_t pit_cfg =
 	 {
		.ch = 0,
		.load_val = PIT_TICKS_FROM_US(833),
		.periodic = true,
		.int_en = true,
		.callback = ftm_cb,
		.user = NULL
 	 };
	PIT_Config(&pit_cfg);
	PIT_Stop(0);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
  UART_Poll();
  if (bitStartDetected())
  {

	  gpioToggle(PORTNUM2PIN(PB,2));
	  setReadingFlag();
	  clearFinished();
	  //gpioToggle(PORTNUM2PIN(PB,3));
	  PIT_Start(0);
	  //gpioToggle(PORTNUM2PIN(PB,3));
  }
  bit_stream[0] +=0;
  static char palabras[20];
  static int k;
  if(finishStatus())
  {
	  clearFinished();
	  //bit_stream[0] += 0;



    char received_data[1]; // placeholder
    received_data[0] = deformat_bitstream(bit_stream);

   // if (received_data != 0xFF){
        char bits[17];
        uint16_t frame = data_to_uart(received_data[0]);
        uint16_to_bin(frame, bits, sizeof(bits));
        //UART_SendString("Dato Recibido del ADC: ");
        UART_SendString(received_data);
        palabras[k]= received_data[0];
        k++;
        //UART_SendString("\r\n");
        //UART_SendString("Informacion recibida en bits: ");
        //UART_SendString(bits);
        //UART_SendString("\r\n");
    }//}
  if(k==20)
  {
	  k=0;
  }

  //PWM_setDuty(50);
}

/*******************************************************************************
 *******************************************************************************
    LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void delayLoop(uint32_t veces)
{
    while (veces--);
}

static void __error_handler__(void)
{
    gpioWrite(PIN_LED_RED, LED_ACTIVE);
}

static void ftm_cb(void* user)
{
	gpioToggle(PORTNUM2PIN(PB,3));
	static uint8_t cnt = 1;
	bit_stream[cnt]  = processBit();
/*/////////////////////////////////////////
	static uint8_t cnt = 0;
	static int i=0;
	data_stream[i] = data_stream[i]<< cnt | processBit() ;
*/////////////////////////////////////////
	cnt++;
	if (cnt == 11)//|| (bit_stream[i][cnt-1] == 2)) //bit[cnt-1 ]== 2 => error
	{
		PIT_Stop(0);
		//DELAY
		//int n = 100000;
		//while(n--);
		cnt = 1;
		/////////
		//i++;
		//data_stream[i]= 0;
		//cnt= 0;
		//////////
		clearReadingFlag();
		finishedReading();
		IC_clearBitStart();
		//gpioToggle(PORTNUM2PIN(PB,2));
	}
	gpioToggle(PORTNUM2PIN(PB,3));
	/*if(i == 10)
	{
		i= 0;
	}*/



}

