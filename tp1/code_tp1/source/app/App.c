/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "../drv/board.h"
#include "../drv/gpio.h"
#include "../drv/SysTick.h"
#include "../drv/mag_strip.h"
#include "../drv/shift_registers.h"
#include "../display.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);
static void foo(void);
int pan2Id(uint64_t pan);


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{

	int i = magStrip_Init();
//	int j = serialData_init();
	display_init();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	uint64_t pan, id;
	uint32_t add_data, disc_data, valid;
	if(getIsDataReady())
	{
		valid = processStripData(&pan, &add_data, &disc_data);
		pan = pan;
		add_data = add_data;
		disc_data = disc_data;
		id = pan2Id(pan);
	}


	setPWM(0);
	// With -
	int n = 100000000000;
	while(n--)
	{
		display(123,0);
	}
}

int pan2Id(uint64_t pan)
{
	int id = pan % 100000000;
	return id;
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

static void foo(void)
{
    gpioToggle(PIN_LED_BLUE);
    gpioWrite(PORTNUM2PIN(PB, 2), HIGH);
}

/*******************************************************************************
 ******************************************************************************/
