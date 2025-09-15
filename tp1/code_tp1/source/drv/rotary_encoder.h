/**
 * @file rotary_encoder.h
 * @brief Archivo header exlusivo al manejo y logica del encoder rotativo y su boton.
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

#define LONG_PRESS_CYCLES ((INTERRUPT_FREQUENCY * LONG_PRESS_TIME) * ENCODER_BUFFER_SIZE) // Ciclos
#define MAX_CYCLES_BETWEEN_DOUBLE_PRESS 1000                                              // Ciclos

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
 * @brief Lectura y logica del estado del encoder rotativo y su boton. 
 * @return Estado actual del encoder. Priorizacion de eventos: Double Press > Long Press > Press > CW = CCW
 */
enc_input_t encoder_update(void);

/**
 * @brief Callback de interrupcion para analisis de encoder. "Flag up"
 */
void encoder_callback(void);

/**
 * @brief Seteo de pines de lectura de Encoder (rotativo y boton)
 */
void encoderInit(void);

/*******************************************************************************
 ******************************************************************************/


#endif // ROTARY_ENCODER_H
