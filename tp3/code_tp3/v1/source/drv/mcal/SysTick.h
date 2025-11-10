/**
 * @file SysTick.h
 * @brief Archivo de cabecera para el driver del SysTick.
 *
 * Este driver proporciona funcionalidad para inicializar y manejar el temporizador SysTick
 * en el microcontrolador, permitiendo configurar una función de callback que se ejecuta
 * periódicamente según un conteo especificado.
 */

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/**
 * @def SYSTICK_ISR_FREQUENCY_HZ
 * @brief Frecuencia de interrupción del SysTick en Hz (2000 Hz por defecto).
 */
#define SYSTICK_ISR_FREQUENCY_HZ 2000U

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Inicializa el driver del SysTick.
 *
 * Configura el SysTick con un conteo específico y registra una función de callback
 * que se llamará en cada interrupción del SysTick.
 *
 * @param funcallback Función a llamar en cada interrupción del SysTick.
 * @param count Valor de conteo para el temporizador (debe ser positivo y no exceder 2^25 - 1).
 *              La frecuencia del SysTick es 100 MHz / count.
 * @return True si la inicialización y registro son exitosos, false si funcallback es NULL o count es inválido.
 */
bool SysTick_Init(void (*funcallback)(void), uint32_t count);

/**
 * @brief Obtiene el valor actual del registro de conteo del SysTick.
 *
 * @return Valor actual del contador del SysTick.
 */
uint32_t getValue_SysTick(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _SYSTICK_H_