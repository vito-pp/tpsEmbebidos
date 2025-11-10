/**
 * @file dma.h
 * @brief Archivo de cabecera para el driver de DMA en Kinetis.
 *
 * Este driver proporciona funcionalidad para inicializar y configurar canales de DMA,
 * incluyendo selección de fuentes de solicitud, modo de trigger, configuraciones de transferencia
 * y manejo de interrupciones. Soporta hasta 16 canales.
 */

#ifndef _DMA_H_
#define _DMA_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

#define DMA_NUM_CH 16

/*******************************************************************************
 * PUBLIC TYPES
 ******************************************************************************/

/**
 * @brief Enumeración de fuentes de solicitud para canales DMA.
 */
typedef enum
{
    /* 0–9: UART0..UART3 */
    DMA_REQ_DISABLED           = 0,   /**< Canal deshabilitado. */
    DMA_REQ_UART0_RX           = 2,   /**< UART0 RX. */
    DMA_REQ_UART0_TX           = 3,   /**< UART0 TX. */
    DMA_REQ_UART1_RX           = 4,   /**< UART1 RX. */
    DMA_REQ_UART1_TX           = 5,   /**< UART1 TX. */
    DMA_REQ_UART2_RX           = 6,   /**< UART2 RX. */
    DMA_REQ_UART2_TX           = 7,   /**< UART2 TX. */
    DMA_REQ_UART3_RX           = 8,   /**< UART3 RX. */
    DMA_REQ_UART3_TX           = 9,   /**< UART3 TX. */

    /* 10–19: UART4/5, I2S0, SPI0/1/2, I2C */
    DMA_REQ_UART4_RXTX         = 10,  /**< UART4 RX/TX. */
    DMA_REQ_UART5_RXTX         = 11,  /**< UART5 RX/TX. */
    DMA_REQ_I2S0_RX            = 12,  /**< I2S0 RX. */
    DMA_REQ_I2S0_TX            = 13,  /**< I2S0 TX. */
    DMA_REQ_SPI0_RX            = 14,  /**< SPI0 RX. */
    DMA_REQ_SPI0_TX            = 15,  /**< SPI0 TX. */
    DMA_REQ_SPI1_RXTX          = 16,  /**< SPI1 RX/TX. */
    DMA_REQ_SPI2_RXTX          = 17,  /**< SPI2 RX/TX. */
    DMA_REQ_I2C0               = 18,  /**< I2C0. */  
    DMA_REQ_I2C1_OR_I2C2       = 19,  /**< I2C1 o I2C2. */  

    /* 20–39: FTM0..FTM3 channels */
    DMA_REQ_FTM0CH0            = 20,  /**< FTM0 Canal 0. */
    DMA_REQ_FTM0CH1            = 21,  /**< FTM0 Canal 1. */
    DMA_REQ_FTM0CH2            = 22,  /**< FTM0 Canal 2. */
    DMA_REQ_FTM0CH3            = 23,  /**< FTM0 Canal 3. */
    DMA_REQ_FTM0CH4            = 24,  /**< FTM0 Canal 4. */
    DMA_REQ_FTM0CH5            = 25,  /**< FTM0 Canal 5. */
    DMA_REQ_FTM0CH6            = 26,  /**< FTM0 Canal 6. */
    DMA_REQ_FTM0CH7            = 27,  /**< FTM0 Canal 7. */
    DMA_REQ_FTM1CH0            = 28,  /**< FTM1 Canal 0. */
    DMA_REQ_FTM1CH1            = 29,  /**< FTM1 Canal 1. */
    DMA_REQ_FTM2CH0            = 30,  /**< FTM2 Canal 0. */
    DMA_REQ_FTM2CH1            = 31,  /**< FTM2 Canal 1. */
    DMA_REQ_FTM3CH0            = 32,  /**< FTM3 Canal 0. */
    DMA_REQ_FTM3CH1            = 33,  /**< FTM3 Canal 1. */
    DMA_REQ_FTM3CH2            = 34,  /**< FTM3 Canal 2. */
    DMA_REQ_FTM3CH3            = 35,  /**< FTM3 Canal 3. */
    DMA_REQ_FTM3CH4            = 36,  /**< FTM3 Canal 4. */
    DMA_REQ_FTM3CH5            = 37,  /**< FTM3 Canal 5. */
    DMA_REQ_FTM3CH6            = 38,  /**< FTM3 Canal 6. */
    DMA_REQ_FTM3CH7            = 39,  /**< FTM3 Canal 7. */

    /* 40–48: analog + timers */
    DMA_REQ_ADC0               = 40,  /**< ADC0. */
    DMA_REQ_ADC1               = 41,  /**< ADC1. */
    DMA_REQ_CMP0               = 42,  /**< CMP0. */
    DMA_REQ_CMP1               = 43,  /**< CMP1. */
    DMA_REQ_CMP2               = 44,  /**< CMP2. */
    DMA_REQ_DAC0               = 45,  /**< DAC0. */
    DMA_REQ_DAC1               = 46,  /**< DAC1. */
    DMA_REQ_CMT                = 47,  /**< CMT. */
    DMA_REQ_PDB                = 48,  /**< PDB. */

    /* 49–53: Port control (GPIO) */
    DMA_REQ_PORTA              = 49,  /**< Puerto A. */
    DMA_REQ_PORTB              = 50,  /**< Puerto B. */
    DMA_REQ_PORTC              = 51,  /**< Puerto C. */
    DMA_REQ_PORTD              = 52,  /**< Puerto D. */
    DMA_REQ_PORTE              = 53,  /**< Puerto E. */

    /* 58–63: DMAMUX “Always enabled” (software trigger) */
    DMA_REQ_ALWAYS58           = 58,  /**< Siempre habilitado 58. */
    DMA_REQ_ALWAYS59           = 59,  /**< Siempre habilitado 59. */
    DMA_REQ_ALWAYS60           = 60,  /**< Siempre habilitado 60. */
    DMA_REQ_ALWAYS61           = 61,  /**< Siempre habilitado 61. */
    DMA_REQ_ALWAYS62           = 62,  /**< Siempre habilitado 62. */
    DMA_REQ_ALWAYS63           = 63   /**< Siempre habilitado 63. */
} dma_req_e;

/**
 * @brief Tipo de callback para interrupciones DMA.
 *
 * @param user Puntero a datos de usuario.
 */
typedef void (*dma_cb_t)(void *user);

/**
 * @brief Estructura de configuración para un canal DMA.
 */
typedef struct 
{
    uint8_t ch;             /**< Canal DMA (0-15). */
    dma_req_e request_src;  /**< Fuente de solicitud. */
    bool trig_mode;         /**< True para modo trigger, false para normal. */
    void *saddr;            /**< Dirección fuente. */
    void *daddr;            /**< Dirección destino. */
    uint8_t nbytes;         /**< Bytes por transferencia (1, 2 o 4). */
    int16_t soff, doff;     /**< Offset fuente y destino en bytes. */
    uint16_t major_count;   /**< Elementos por loop mayor. */
    int32_t slast;          /**< Ajuste de puntero al final mayor (fuente). */
    int32_t dlast;          /**< Ajuste de puntero al final mayor (destino). */
    bool int_major;         /**< True para interrupción al final mayor. */
    dma_cb_t on_major;      /**< Callback al final mayor (puede ser NULL). */
    void *user;             /**< Cookie de usuario para callbacks. */
} dma_cfg_t;

/*******************************************************************************
 * PUBLIC API
 ******************************************************************************/

/**
 * @brief Inicializa el driver de eDMA/DMAMUX (una vez).
 *
 * Habilita clocks de módulos, configura ajustes globales de eDMA y engancha ISRs de NVIC
 * para todos los canales DMA usados por el HAL.
 *
 * @return 0 si éxito, <0 si error (ya inicializado o problema con clock/NVIC).
 */
int DMA_Init(void);                     

/**
 * @brief Configura un canal DMA (TCD + DMAMUX) desde una estructura dma_cfg_t.
 *
 * Programa los campos TCD (SADDR/DADDR/NBYTES/SOFF/DOFF/CITER/BITER/SLAST/DLAST)
 * y el canal DMAMUX fuente/modo trigger.
 *
 * @param cfg Puntero a la configuración del canal.
 * @return 0 si éxito, <0 si error (canal inválido, tamaños/alineaciones malas o puntero nulo).
 */
int DMA_Config(const dma_cfg_t *cfg); 

/**
 * @brief Habilita solicitud (ERQ) para un canal configurado (lo arma).
 *
 * @param ch Índice de canal DMA [0..15].
 * @return 0 si éxito, <0 si error (canal inválido o no configurado).
 */
int DMA_Start(uint8_t ch);

/**
 * @brief Deshabilita solicitud (ERQ) para un canal (lo desarma).
 *
 * No limpia el TCD. Puedes llamarlo de nuevo con DMA_Start más tarde.
 *
 * @param ch Índice de canal DMA [0..15].
 * @return 0 si éxito, <0 si error (canal inválido).
 */
int DMA_Stop(uint8_t ch);

#endif