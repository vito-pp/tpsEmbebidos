/**
 * @file pit.h
 * @brief Archivo de cabecera para el driver del Temporizador de Interrupción Periódica (PIT) en Kinetis K64.
 *
 * Este driver soporta 4 canales independientes de PIT, con conteo descendente de 32 bits,
 * fuente de reloj del bus del sistema (50 MHz en FRDM-K64F). Permite modo one-shot o periódico,
 * solicitud de DMA opcional y callbacks por canal.
 */

#ifndef _PIT_H_
#define _PIT_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

// System Bus Clock @ 50 MHz
#ifndef SYS_BUS_CLK
#define SYS_BUS_CLK (50000000UL)
#endif

/**
 * @def PIT_TICKS_FROM_US(us)
 * @brief Convierte microsegundos a ticks del PIT.
 */
#define PIT_TICKS_FROM_US(us) ((uint32_t)((((SYS_BUS_CLK/1000000UL)*(us)) - 1)))

/**
 * @def PIT_TICKS_FROM_MS(ms)
 * @brief Convierte milisegundos a ticks del PIT.
 */
#define PIT_TICKS_FROM_MS(ms) ((uint32_t)((((SYS_BUS_CLK/1000UL)*(ms)) - 1)))

/**
 * @def PIT_CHANNELS
 * @brief Número de canales disponibles en el PIT (4).
 */
#define PIT_CHANNELS 4

/*******************************************************************************
 * PUBLIC TYPES
 ******************************************************************************/

/**
 * @brief Enumeración para identificar canales del PIT.
 */
typedef enum
{
    PIT_CH0 = 0, /**< Canal 0. */
    PIT_CH1 = 1, /**< Canal 1. */
    PIT_CH2 = 2, /**< Canal 2. */
    PIT_CH3 = 3, /**< Canal 3. */
} pit_ch_e;

/**
 * @brief Tipo de callback para interrupciones del PIT.
 *
 * @param user Puntero a datos de usuario.
 */
typedef void (*pit_cb_t)(void *user);

/**
 * @brief Estructura de configuración para un canal del PIT.
 */
typedef struct
{
    pit_ch_e   ch;          /**< Canal PIT (PIT_CH0 a PIT_CH3). */                       
    uint32_t   load_val;    /**< Valor de carga LDVAL (ticks a contar). */    
    bool       periodic;    /**< True para modo periódico, false para one-shot. */
    bool       int_en;      /**< True para habilitar interrupción (callback). */       
    bool       dma_req;     /**< True para solicitar DMA al timeout. */     
    pit_cb_t   callback;    /**< Callback (puede ser NULL). */                               
    void      *user;        /**< Cookie pasado al callback. */                 
} pit_cfg_t;

/*******************************************************************************
 * PUBLIC API
 ******************************************************************************/

/**
 * @brief Inicialización global del PIT (clocks + NVIC).
 */
void PIT_Init(void);

/**
 * @brief Configura un canal del PIT. Si modo periódico, inicia automáticamente.
 *
 * @param cfg Puntero a estructura de configuración llena.
 * @return True si éxito, false si error.
 */
bool PIT_Config(const pit_cfg_t *cfg);

/**
 * @brief (Re)inicia un canal.
 *
 * @param ch Canal a iniciar.
 * @return True si éxito, false si error.
 */
bool PIT_Start(pit_ch_e ch);

/**
 * @brief Detiene un canal (deshabilita temporizador y solicitud).
 *
 * @param ch Canal a detener.
 * @return True si éxito, false si error.
 */
bool PIT_Stop(pit_ch_e ch);

/**
 * @brief Cambia el valor de carga en tiempo real (solo modo periódico).
 *
 * @param ch Canal.
 * @param new_load Nuevo valor LDVAL.
 * @return True si éxito, false si error.
 */
bool PIT_SetLoad(pit_ch_e ch, uint32_t new_load);

/**
 * @brief Obtiene ticks restantes (para depuración/sincronización).
 *
 * @param ch Canal.
 * @return Valor actual de CVAL (0 = expirado).
 */
uint32_t PIT_GetCount(pit_ch_e ch);

#endif // _PIT_H_ 