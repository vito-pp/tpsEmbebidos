/***************************************************************************//**
  @file     App.c
  @brief    Test simple: enviar el mismo mensaje continuamente por UART0
*******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stddef.h>
#include <stdio.h>

#include "drv/board.h"
#include "drv/gpio.h"
#include "misc/timer.h"
#include "drv/SysTick.h"
#include "drv/i2c.h"
#include "drv/FXOS8700CQ.h"

#include <stdbool.h>
#include "drv/UART_strings.h"
#include "drv/UART.h"

/*******************************************************************************
 * FILE SCOPE VARIABLES
 ******************************************************************************/

static Vec3_t uT, mg;
static Rotation_t rot;
static char rx_line[128];

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
static void __error_handler__(void);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioWrite(PIN_LED_RED, !LED_ACTIVE);
	UART_Init();
    if (!FXOS_Init(0, 9600))
    {
        __error_handler__();
    }
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	FXOS_ReadBoth(&mg, &uT);
    delayLoop(1000);
#if	I2C_POLLING_FLAG
    I2C_ServicePoll(0);
#endif
	vec2rot(&mg, &uT, &rot);

	UART_Poll();
	/* TX no bloqueante */
	if (UART_TxPending() == 0)
	{
		/* Intentar encolar (puede no entrar todo a la vez) */
		UART_SendString("A,0,R,-170\n");
		UART_SendString("A,0,C,-120\n");
		UART_SendString("A,0,O,-90\n");
		UART_SendString("A,0,R,-45\n");
		UART_SendString("A,0,C,0\n");
		UART_SendString("A,0,O,45\n");
		UART_SendString("A,0,R,60\n");
		UART_SendString("A,0,C,90\n");
		UART_SendString("A,0,O,150\n");
		UART_SendString("A,1,R,-170\n");
		UART_SendString("A,1,C,-120\n");
		UART_SendString("A,1,O,-90\n");
	}

	/* RX no bloqueante: copiar disponible hasta fin de línea o hasta llenar */
	int n = UART_ReceiveString(rx_line, sizeof(rx_line));
	if (n > 0)
	{
		UART_SendString("Recibido: ");
		UART_SendString(rx_line);
		UART_SendString("\r\n");
	}
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
