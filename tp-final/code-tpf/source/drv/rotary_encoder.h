/**
 * @file rotary_encoder.h
 * @brief Header file for handling and logic of the rotary encoder and its button.
 */

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/


#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define ENCODER_BUFFER_SIZE 5
#define MIN_PRESS_CYCLES 50

#define LONG_PRESS_TIME 2        // Seconds
#define INTERRUPT_FREQUENCY 2000 // Hz

#define MAX_DOUBLE_PRESS_COOLDOWN 1 // Seconds

#define LONG_PRESS_CYCLES ((INTERRUPT_FREQUENCY * LONG_PRESS_TIME) \
                            *ENCODER_BUFFER_SIZE / 2) //Ciclos
#define MAX_CYCLES_BETWEEN_DOUBLE_PRESS 750 //Ciclos

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum
{
    ENC_NONE = 0,
    ENC_CW,
    ENC_CCW,
    ENC_BUTTON_PRESS,
    ENC_DOUBLE_PRESS,
    ENC_BUTTON_LONG_PRESS
} enc_input_t;



/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Reads and handles the state of the rotary encoder and its button.
 * @return Current state of the encoder. Event priority: Double Press > Long Press > Press > CW = CCW
 */
enc_input_t encoder_update(void);

/**
 * @brief Interrupt callback for encoder analysis. "Flag up"
 */
void encoder_callback(void);

/**
 * @brief Sets up the pins for reading the rotary encoder and its button
 */
void encoderInit(void);

/*******************************************************************************
 ******************************************************************************/


#endif // ROTARY_ENCODER_H
