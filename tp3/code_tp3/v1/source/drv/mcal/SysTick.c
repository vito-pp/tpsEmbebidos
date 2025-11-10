/**
 * @file SysTick.c
 * @brief Driver del SysTick.
 *
 * Implementa la inicialización y el manejo de interrupciones del SysTick,
 * configurando el temporizador y registrando una función de callback.
 */

#include <stdbool.h>
#include <stddef.h>
#include "hardware.h"

#include "gpio.h"
#include "board.h"

#define NUM_BITS_SYSTICK_LOAD 24

/**
 * @var cb
 * @brief Puntero a la función de callback que se ejecuta en la interrupción del SysTick.
 */
static void (*cb)(void);

/**
 * @brief Inicializa el SysTick.
 *
 * @param funcallback Función de callback.
 * @param count Valor de carga para el SysTick.
 * @return True si éxito, false si error.
 */
bool SysTick_Init (void (*funcallback)(void), uint32_t count)
{
    if(funcallback == NULL || count >= (1U << (NUM_BITS_SYSTICK_LOAD + 1))
    || count == 0) 
        return false;

    SysTick->CTRL = 0x00;
    SysTick->LOAD = count - 1; // core CLK @ 100 MHz
    SysTick->VAL = 0x00;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;

    cb = funcallback;
    return true;
}

/**
 * @brief Manejador de interrupción del SysTick.
 *
 * Llama a la función de callback registrada.
 */
void SysTick_Handler (void)
{
    cb();
}

/**
 * @brief Obtiene el valor actual del SysTick.
 *
 * @return Valor del registro VAL del SysTick.
 */
uint32_t getValue_SysTick(void)
{
    return SysTick->VAL;
}