/***************************************************************************//**
  @file     board.h
  @brief    Board management
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** BOARD defines **********************************************************/

// On Board User LEDs
#define PIN_LED_RED     PORTNUM2PIN(PB, 22)   // PTB22
#define PIN_LED_GREEN   PORTNUM2PIN(PE, 26)   // PTE26
#define PIN_LED_BLUE    PORTNUM2PIN(PB, 21)   // PTB21

#define LED_ACTIVE      LOW


// On Board User Switches
#define PIN_SW2         // ???
#define PIN_SW3         PORTNUM2PIN(PA, 4)    // PTA4

#define SW_ACTIVE       LOW 
#define SW_INPUT_TYPE   // ???

//Magnetic Strip Pins
#define STRIP_ENABLE PORTNUM2PIN(PB, 2)
#define STRIP_CLK PORTNUM2PIN(PB, 3)
#define STRIP_DATA PORTNUM2PIN(PB, 10)

//Shift Register pins
#define SR_DATA PORTNUM2PIN(PB,2)
#define SR_CLK PORTNUM2PIN(PB,3)
#define SR_ENABLE PORTNUM2PIN(PB,10)

// PIN BUS
#define PIN_BUS_0		PORTNUM2PIN(PB,2)
#define PIN_BUS_1		PORTNUM2PIN(PC,3)
#define PIN_BUS_2		PORTNUM2PIN(PC,2)
#define PIN_BUS_3		PORTNUM2PIN(PA,2)
#define PIN_BUS_4		PORTNUM2PIN(PB,23)
#define PIN_BUS_5		PORTNUM2PIN(PA,1)
#define PIN_BUS_6		PORTNUM2PIN(PB,9)
#define PIN_BUS_7		PORTNUM2PIN(PC,17)


// PIN STATE
#define PIN_STATUS_0		PORTNUM2PIN(PB,20)
#define PIN_STATUS_1		PORTNUM2PIN(PC,5)


/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_
