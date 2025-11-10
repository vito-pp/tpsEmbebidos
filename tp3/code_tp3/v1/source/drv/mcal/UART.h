/**
 * @file UART.h
 * @brief Archivo de cabecera para el driver de UART en Kinetis.
 *
 * Este driver proporciona funciones para inicializar UART, configurar paridad y baudrate,
 * y enviar/recibir datos individuales. Soporta configuración de pines y habilitación de interrupciones.
 */

#ifndef UART_H_
#define UART_H_
#include "MK64F12.h"

/**
 * @def UART_HAL_DEFAULT_BAUDRATE
 * @brief Baudrate por defecto para UART (1200).
 */
#define UART_HAL_DEFAULT_BAUDRATE 1200

/**
 * @def UART0_TX_PIN
 * @brief Pin para transmisión UART0 (17 - PTB17).
 */
#define UART0_TX_PIN 	17   //PTB17

/**
 * @def UART0_RX_PIN
 * @brief Pin para recepción UART0 (16 - PTB16).
 */
#define UART0_RX_PIN 	16   //PTB16

/**
 * @brief Enumeración para modos de multiplexación de pines (MUX).
 */
typedef enum
{
	PORT_mAnalog,  /**< Modo analógico. */
	PORT_mGPIO,    /**< Modo GPIO. */
	PORT_mAlt2,    /**< Alternativa 2. */
	PORT_mAlt3,    /**< Alternativa 3. */
	PORT_mAlt4,    /**< Alternativa 4. */
	PORT_mAlt5,    /**< Alternativa 5. */
	PORT_mAlt6,    /**< Alternativa 6. */
	PORT_mAlt7,    /**< Alternativa 7. */
} PORTMux_t;

/**
 * @brief Enumeración para eventos de interrupción en pines.
 */
typedef enum
{
	PORT_eDisabled				= 0x00,  /**< Deshabilitado. */
	PORT_eDMARising				= 0x01,  /**< DMA en flanco ascendente. */
	PORT_eDMAFalling			= 0x02,  /**< DMA en flanco descendente. */
	PORT_eDMAEither				= 0x03,  /**< DMA en cualquier flanco. */
	PORT_eInterruptDisasserted	= 0x08,  /**< Interrupción en desasertado. */
	PORT_eInterruptRising		= 0x09,  /**< Interrupción en ascendente. */
	PORT_eInterruptFalling		= 0x0A,  /**< Interrupción en descendente. */
	PORT_eInterruptEither		= 0x0B,  /**< Interrupción en cualquier flanco. */
	PORT_eInterruptAsserted		= 0x0C,  /**< Interrupción en asertado. */
} PORTEvent_t;

/**
 * @brief Enumeración para tipos de paridad en UART.
 */
typedef enum {
    UART_PARITY_NONE = 0,  /**< Sin paridad. */
    UART_PARITY_EVEN,      /**< Paridad par. */
    UART_PARITY_ODD        /**< Paridad impar. */
} uart_parity_t;

/**
 * @brief Inicializa el UART con paridad especificada.
 *
 * Habilita clocks, configura pines para UART0 y establece baudrate y paridad por defecto.
 *
 * @param parity Tipo de paridad (none, even, odd).
 */
void UART_Init (char parity);

/**
 * @brief Configura la paridad para un UART específico.
 *
 * @param uart Puntero al UART (UART0, etc.).
 * @param parity Tipo de paridad.
 */
void UART_SetParity(UART_Type *uart, uart_parity_t parity);

/**
 * @brief Configura el baudrate para un UART específico.
 *
 * @param uart Puntero al UART.
 * @param baudrate Baudrate deseado.
 */
void UART_SetBaudRate (UART_Type *uart, uint32_t baudrate);

/**
 * @brief Envía un byte de datos por UART0.
 *
 * Espera a que el transmisor esté listo (TDRE=1).
 *
 * @param txdata Byte a enviar.
 */
void UART_Send_Data(unsigned char txdata);

/**
 * @brief Recibe un byte de datos por UART0.
 *
 * Verifica si hay datos disponibles (RDRF=1) y maneja errores de paridad.
 *
 * @return Byte recibido si disponible, 0 caso contrario.
 */
unsigned char UART_Recieve_Data();

#endif /* UART_H_ */