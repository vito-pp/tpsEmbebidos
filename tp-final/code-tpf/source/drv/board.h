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

// On Board User LEDs, custom board, active LOW
#define PIN_LED_RED    PORTNUM2PIN(PB, 17) // PTB17 / LED_R
#define PIN_LED_GREEN  PORTNUM2PIN(PB, 19) // PTB19 / LED_G
#define PIN_LED_BLUE   PORTNUM2PIN(PB, 21) // PTB21 / LED_A

#define LED_ACTIVE LOW


// On Board User Switches
#define PIN_SW2         // ???
#define PIN_SW3         PORTNUM2PIN(PA, 4)    // PTA4

#define SW_ACTIVE       LOW 
#define SW_INPUT_TYPE   // ???

// Magnetic Strip Pins
#define STRIP_ENABLE   PORTNUM2PIN(PB, 0)
#define STRIP_CLK      PORTNUM2PIN(PB, 2)
#define STRIP_DATA     PORTNUM2PIN(PB, 9)

// Shift Register pins, custom board
#define SR_DATA        PORTNUM2PIN(PC, 16) // SERIAL_DATA
#define SR_CLK         PORTNUM2PIN(PC, 1)  // SERIAL_CLOCK
#define SR_LCLK        PORTNUM2PIN(PC, 18) // SERIAL_LATCH

// Encoder
#define PIN_ENC_A      PORTNUM2PIN(PC,4) // ENCODER_A
#define PIN_ENC_B      PORTNUM2PIN(PC,5) // ENCODER_B
#define PIN_ENC_C      PORTNUM2PIN(PC,3) // ENCODER_SW

// PIN BUS
#define PIN_BUS_0	PORTNUM2PIN(PB,2)
#define PIN_BUS_1	PORTNUM2PIN(PC,3)
#define PIN_BUS_2	PORTNUM2PIN(PC,2)
#define PIN_BUS_3	PORTNUM2PIN(PA,2)
#define PIN_BUS_4	PORTNUM2PIN(PB,23)
#define PIN_BUS_5	PORTNUM2PIN(PA,1)
#define PIN_BUS_6	PORTNUM2PIN(PB,9)
#define PIN_BUS_7	PORTNUM2PIN(PC,17)


// PIN STATE
#define PIN_STATUS_0	PORTNUM2PIN(PB,20)
#define PIN_STATUS_1	PORTNUM2PIN(PC,5)

/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_
