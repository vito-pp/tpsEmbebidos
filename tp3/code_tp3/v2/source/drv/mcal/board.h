/***************************************************************************//**
  @file     board.h
  @brief    Board management for the FRDM-K64F
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

#define PIN_I2C_SCL     PORTNUM2PIN(PE, 24)
#define PIN_I2C_SDA     PORTNUM2PIN(PE, 25)

#define FTM0_CH0 		PORTNUM2PIN(PC,1) //PWM
#define FTM3CH5 		PORTNUM2PIN(PC,9) //IC

#define CMP0_OUT        PORTNUM2PIN(PC,5)
#define CMP0_IN1        PORTNUM2PIN(PC,7)
//#define CMP0_IN2        PORTNUM2PIN(PC,8)

/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_
