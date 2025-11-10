/**
 * @file ADC.h
 * @brief Archivo de cabecera para el driver del Convertidor Analógico-Digital (ADC) en Kinetis.
 *
 * Este driver proporciona funcionalidad para inicializar, configurar y operar los ADC0 y ADC1
 * del microcontrolador Kinetis (MK64F12). Incluye configuración de resolución, ciclos de muestreo,
 * promediado por hardware, calibración, disparo de conversión y lectura de resultados.
 * Habilita clocks, NVIC e inicializa ADC0 por defecto con calibración.
 */

#ifndef SOURCES_TEMPLATE_ADC_H_
#define SOURCES_TEMPLATE_ADC_H_

#include "hardware.h"
#include "MK64F12.h"

/**
 * @brief Enumeración para la resolución de conversión del ADC (bits).
 */
typedef enum
{
	ADC_b8,   /**< 8 bits de resolución. */
	ADC_b12,  /**< 12 bits de resolución. */
	ADC_b10,  /**< 10 bits de resolución. */
	ADC_b16,  /**< 16 bits de resolución. */
} ADCBits_t;

/**
 * @brief Enumeración para los ciclos de muestreo del ADC.
 */
typedef enum
{
	ADC_c24,  /**< 24 ciclos de muestreo. */
	ADC_c16,  /**< 16 ciclos de muestreo. */
	ADC_c10,  /**< 10 ciclos de muestreo. */
	ADC_c6,   /**< 6 ciclos de muestreo. */
	ADC_c4,   /**< 4 ciclos de muestreo. */
} ADCCycles_t;

/**
 * @brief Enumeración para el número de muestras a promediar por hardware.
 */
typedef enum
{
	ADC_t4,   /**< Promedio de 4 muestras. */
	ADC_t8,   /**< Promedio de 8 muestras. */
	ADC_t16,  /**< Promedio de 16 muestras. */
	ADC_t32,  /**< Promedio de 32 muestras. */
	ADC_t1,   /**< Sin promedio (1 muestra). */
} ADCTaps_t;

/**
 * @brief Enumeración para la selección del multiplexor (MUX) del ADC.
 */
typedef enum
{
	ADC_mA,  /**< MUX A. */
	ADC_mB,  /**< MUX B. */
} ADCMux_t;

/**
 * @brief Tipo de puntero a la estructura del ADC (ADC_Type).
 */
typedef ADC_Type *ADC_t;

/**
 * @brief Tipo para el número de canal del ADC (0-23).
 */
typedef uint8_t ADCChannel_t; /* Channel 0-23 */

/**
 * @brief Tipo para el valor de datos convertido del ADC (uint16_t).
 */
typedef uint16_t ADCData_t;

/**
 * @brief Inicializa el subsistema ADC (ADC0/ADC1) y configura ADC0 por defecto.
 *
 * - Habilita clocks a ADC0 y ADC1.
 * - Habilita interrupciones en NVIC.
 * - Ajusta prescaler/config base (CFG1).
 * - Setea resolución y ciclos de muestreo de ADC0.
 * - Ejecuta calibración de ADC0.
 *
 * @param dma_req True para habilitar DMA, false para deshabilitar.
 * @post ADC0 queda listo para convertir con la resolución/ciclos elegidos.
 */
void 		ADC_Init 			   (bool dma_req);

/**
 * @brief Habilita o deshabilita el modo de interrupción por fin de conversión para un ADC.
 *
 * @param adc ADC0 o ADC1.
 * @param mode True para habilitar interrupción, false para deshabilitar.
 * @note Actualiza un flag interno usado al disparar conversiones (SC1.AIEN).
 */
void 		ADC_SetInterruptMode   (ADC_t, bool);

/**
 * @brief Indica si hay fin de conversión pendiente (COCO=1) en el canal 0.
 *
 * @param adc ADC0 o ADC1.
 * @return True si COCO está en 1, false caso contrario.
 * @note Lee SC1[0] y evalúa el bit COCO.
 */
bool 		ADC_IsInterruptPending (ADC_t);

/**
 * @brief Limpia el flag de conversión completada en SC1[0].
 *
 * @param adc ADC0 o ADC1.
 * @note Escribe SC1[0] para limpiar COCO según el manual de referencia.
 */
void 		ADC_ClearInterruptFlag (ADC_t);

/**
 * @brief Selecciona la resolución de conversión del ADC (8/10/12/16 bits, según SoC).
 *
 * @param adc Puntero al periférico ADC (ADC0 o ADC1).
 * @param bits Modo de resolución (enum ADCBits_t).
 * @note Actualiza el campo MODE de CFG1.
 */
void 		ADC_SetResolution 	   (ADC_t, ADCBits_t);

/**
 * @brief Obtiene la resolución configurada del ADC.
 *
 * @param adc ADC0 o ADC1.
 * @return Valor del campo MODE de CFG1 (enum ADCBits_t).
 */
ADCBits_t 	ADC_GetResolution 	   (ADC_t);

/**
 * @brief Configura tiempo de muestreo (ciclos) del ADC.
 *
 * @param adc ADC0 o ADC1.
 * @param cycles Selección de ciclos (enum ADCCycles_t).
 * @details Si el valor requiere modo de muestreo largo, habilita ADLSMP y programa ADLSTS.
 *          Caso contrario, deshabilita muestreo largo.
 */
void 		ADC_SetCycles	 	   (ADC_t, ADCCycles_t);

/**
 * @brief Retorna la configuración de ciclos de muestreo vigente.
 *
 * @param adc ADC0 o ADC1.
 * @return ADCCycles_t correspondiente a la configuración actual (muestreo corto/largo).
 * @note Si ADLSMP=1 devuelve ADC_c4 (muestreo largo); de lo contrario, usa ADLSTS.
 */
ADCCycles_t ADC_GetCycles	 	   (ADC_t);

/**
 * @brief Configura promediado por hardware del ADC.
 *
 * @param adc ADC0 o ADC1.
 * @param taps Cantidad de muestras a promediar (enum ADCTaps_t).
 * @details Si taps no es válido, deshabilita el promedio (AVGE=0).
 *          Caso contrario, habilita AVGE y programa AVGS.
 */
void 		ADC_SetHardwareAverage (ADC_t, ADCTaps_t);

/**
 * @brief Consulta si el promediado por hardware está activo y su configuración.
 *
 * @param adc ADC0 o ADC1.
 * @return Si AVGE=1 devuelve ADC_t1 (indicando promedio activo),
 *         si no, devuelve AVGS (taps configurados).
 */
ADCTaps_t   ADC_GetHardwareAverage (ADC_t);

/**
 * @brief Ejecuta rutina de calibración del ADC.
 *
 * @param adc ADC0 o ADC1.
 * @return True si la calibración finaliza sin errores; false si se detecta CALF.
 *
 * @details
 * - Realiza una calibración inicial y verifica CALF.
 * - Calcula PG/MG en base a sumatoria de coeficientes (CLPx/CLMx).
 * - Ejecuta iteraciones adicionales (2^4) para refinar OFS/ganancias.
 * - Restaura SC3 original al finalizar.
 *
 * @post PG, MG, OFS y coeficientes CLP/CLM* quedan actualizados.
 */
bool 		ADC_Calibrate 		   (ADC_t);

/**
 * @brief Dispara una conversión en el canal indicado.
 *
 * @param adc ADC0 o ADC1.
 * @param channel Canal a convertir (enum ADCChannel_t).
 * @param mux Selección del MUX (enum ADCMux_t).
 * @note Ajusta MUXSEL (CFG2) y escribe SC1[0] con ADCH y AIEN (según flag interno).
 */
void 		ADC_Start 			   (ADC_t, ADCChannel_t, ADCMux_t);

/**
 * @brief Consulta si la conversión en curso finalizó (COCO=1).
 *
 * @param adc ADC0 o ADC1.
 * @return True si COCO=1; false en caso contrario.
 */
bool 		ADC_IsReady 	       (ADC_t);

/**
 * @brief Lee el dato convertido del resultado R[0].
 *
 * @param adc ADC0 o ADC1.
 * @return Muestra convertida (tipo ADCData_t).
 * @note Debe llamarse luego de que COCO indique fin de conversión.
 */
ADCData_t 	ADC_getData 		   (ADC_t);

#endif /* SOURCES_TEMPLATE_ADC_H_ */