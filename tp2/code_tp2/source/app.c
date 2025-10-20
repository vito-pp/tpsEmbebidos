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
static void UART_SendRotation0(const Rotation_t *r);
static inline int clamp_deg_179(float x);
static void append_int(char **p, int v);
static void append_str(char **p, const char *s);
static void int_to_ascii(int v, char *out);

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
	//FXOS_ReadBoth(&mg, &uT);
    FXOS_ReadMagnetometer(&uT);
    while (I2C_GetStatus(0) != I2C_AVAILABLE)
    {
        I2C_ServicePoll(0);
    }
    FXOS_ReadAccelerometer(&mg);

    vec2rot(&mg, &uT, &rot);

	UART_Poll();
	/* TX no bloqueante */
	if (UART_TxPending() == 0)
	{
		/* Intentar encolar (puede no entrar todo a la vez) */
	    UART_SendRotation0(&rot);
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
static inline int clamp_deg_179(float x)
{
    /* redondeo a entero y saturación al rango [-179..179] */
    int v = (int)(x >= 0.f ? x + 0.5f : x - 0.5f);
    if (v > 179)  v = 179;
    if (v < -179) v = -179;
    return v;
}

/* int -> ASCII (C89), soporta negativos, out debe tener >=12 bytes */
static void int_to_ascii(int v, char *out)
{
    char tmp[12];
    int n = 0;
    unsigned int u;

    if (v < 0) { *out++ = '-'; u = (unsigned int)(-v); }
    else       { u = (unsigned int)v; }

    if (u == 0) { *out++ = '0'; *out = '\0'; return; }

    while (u) { tmp[n++] = (char)('0' + (u % 10)); u /= 10; }

    while (n--) *out++ = tmp[n];
    *out = '\0';
}

static void append_str(char **p, const char *s)
{
    while (*s) *(*p)++ = *s++;
}

static void append_int(char **p, int v)
{
    char nb[12];
    int_to_ascii(v, nb);
    append_str(p, nb);
}

/* Envía: "A,0,R,<roll>\nA,0,C,<pitch>\nA,0,O,<yaw>\n" */
static void UART_SendRotation0(const Rotation_t *r)
{
    if (!r) return;

    const int roll  = clamp_deg_179(r->roll);
    const int pitch = clamp_deg_179(r->pitch);
    const int yaw   = r->yaw;

    /* 3 líneas en un solo buffer */
    char buf[256];
    char *p = buf;

    append_str(&p, "A,0,O,"); append_int(&p,yaw);   append_str(&p, "\r\n");
    append_str(&p, "A,0,R,"); append_int(&p, roll);  append_str(&p, "\r\n");
    append_str(&p, "A,0,C,"); append_int(&p, pitch); append_str(&p, "\r\n");
    append_str(&p, "A,0,O,"); append_int(&p, yaw);   append_str(&p, "\r\n");
    *p = '\0';

    UART_SendString(buf);
}
