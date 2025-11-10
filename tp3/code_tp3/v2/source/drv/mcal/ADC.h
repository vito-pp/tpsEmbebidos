
#ifndef SOURCES_TEMPLATE_ADC_H_
#define SOURCES_TEMPLATE_ADC_H_

#include "hardware.h"

typedef enum
{
	ADC_b8,
	ADC_b12,
	ADC_b10,
	ADC_b16,
} ADCBits_t;

typedef enum
{
	ADC_c24,
	ADC_c16,
	ADC_c10,
	ADC_c6,
	ADC_c4,
} ADCCycles_t;

typedef enum
{
	ADC_t4,
	ADC_t8,
	ADC_t16,
	ADC_t32,
	ADC_t1,
} ADCTaps_t;

typedef enum
{
	ADC_mA,
	ADC_mB,
} ADCMux_t;

typedef ADC_Type *ADC_t;
typedef uint8_t ADCChannel_t; /* Channel 0-23 */
typedef uint16_t ADCData_t;

/**
 * @brief   Inicializa el subsistema ADC (ADC0/ADC1) y configura ADC0 por defecto.
 *
 * - Habilita clocks a ADC0 y ADC1.
 * - Habilita interrupciones en NVIC.
 * - Ajusta prescaler/config base (CFG1).
 * - Setea resolución y ciclos de muestreo de ADC0.
 * - Ejecuta calibración de ADC0.
 *
 * @post    ADC0 queda listo para convertir con la resolución/ciclos elegidos.
 */
void 		ADC_Init 			   (void);
/**
 * @brief   Habilita o deshabilita el modo de interrupción por fin de conversión para un ADC.
 * @param   adc    ADC0/ADC1.
 * @param   mode   true para habilitar interrupción, false para deshabilitar.
 * @note    Actualiza un flag interno usado al disparar conversiones (SC1.AIEN).
 */
void 		ADC_SetInterruptMode   (ADC_t, bool);

/**
 * @brief   Indica si hay fin de conversión pendiente (COCO=1) en el canal 0.
 * @param   adc   ADC0/ADC1.
 * @return  true si COCO está en 1, false caso contrario.
 * @note    Lee SC1[0] y evalúa el bit COCO.
 */
bool 		ADC_IsInterruptPending (ADC_t);

/**
 * @brief   Limpia el flag de conversión completada en SC1[0].
 * @param   adc   ADC0/ADC1.
 * @note    Escribe SC1[0] para limpiar COCO según el manual de referencia.
 */
void 		ADC_ClearInterruptFlag (ADC_t);

/**
 * @brief   Selecciona la resolución de conversión del ADC (8/10/12/16 bits, según SoC).
 * @param   adc   Puntero al periférico ADC (ADC0 o ADC1).
 * @param   bits  Modo de resolución (enum ADCBits_t).
 * @note    Actualiza el campo MODE de CFG1.
 */
void 		ADC_SetResolution 	   (ADC_t, ADCBits_t);


/**
 * @brief   Obtiene la resolución configurada del ADC.
 * @param   adc   ADC0/ADC1.
 * @return  Valor del campo MODE de CFG1 (enum ADCBits_t).
 */
ADCBits_t 	ADC_GetResolution 	   (ADC_t);

/**
 * @brief   Configura tiempo de muestreo (ciclos) del ADC.
 * @param   adc     ADC0/ADC1.
 * @param   cycles  Selección de ciclos (enum ADCCycles_t).
 * @details Si el valor requiere modo de muestreo largo, habilita ADLSMP y programa ADLSTS.
 *          Caso contrario, deshabilita muestreo largo.
 */
void 		ADC_SetCycles	 	   (ADC_t, ADCCycles_t);

/**
 * @brief   Retorna la configuración de ciclos de muestreo vigente.
 * @param   adc   ADC0/ADC1.
 * @return  ADCCycles_t correspondiente a la configuración actual (muestreo corto/largo).
 * @note    Si ADLSMP=1 devuelve ADC_c4 (muestreo largo); de lo contrario, usa ADLSTS.
 */

ADCCycles_t ADC_GetCycles	 	   (ADC_t);

/**
 * @brief   Configura promediado por hardware del ADC.
 * @param   adc   ADC0/ADC1.
 * @param   taps  Cantidad de muestras a promediar (enum ADCTaps_t).
 * @details Si @p taps no es válido, deshabilita el promedio (AVGE=0).
 *          Caso contrario, habilita AVGE y programa AVGS.
 */
void 		ADC_SetHardwareAverage (ADC_t, ADCTaps_t);

/**
 * @brief   Consulta si el promediado por hardware está activo y su configuración.
 * @param   adc   ADC0/ADC1.
 * @return  Si AVGE=1 devuelve ADC_t1 (indicando promedio activo),
 *          si no, devuelve AVGS (taps configurados).
 */
ADCTaps_t   ADC_GetHardwareAverage (ADC_t);

/**
 * @brief   Ejecuta rutina de calibración del ADC.
 *
 * @param   adc   ADC0/ADC1.
 * @return  true si la calibración finaliza sin errores; false si se detecta CALF.
 *
 * @details
 * - Realiza una calibración inicial y verifica CALF.
 * - Calcula PG/MG en base a sumatoria de coeficientes (CLPx/CLMx).
 * - Ejecuta iteraciones adicionales (2^4) para refinar OFS/ganancias.
 * - Restaura SC3 original al finalizar.
 *
 * @post    PG, MG, OFS y coeficientes CLP/CLM* quedan actualizados
 */
bool 		ADC_Calibrate 		   (ADC_t);

/**
 * @brief   Dispara una conversión en el canal indicado.
 * @param   adc      ADC0/ADC1.
 * @param   channel  Canal a convertir (enum ADCChannel_t).
 * @param   mux      Selección del MUX (enum ADCMux_t).
 * @note    Ajusta MUXSEL (CFG2) y escribe SC1[0] con ADCH y AIEN (según flag interno).
 */
void 		ADC_Start 			   (ADC_t, ADCChannel_t, ADCMux_t);


/**
 * @brief   Consulta si la conversión en curso finalizó (COCO=1).
 * @param   adc   ADC0/ADC1.
 * @return  true si COCO=1; false en caso contrario.
 */
bool 		ADC_IsReady 	       (ADC_t);


/**
 * @brief   Lee el dato convertido del resultado R[0].
 * @param   adc   ADC0/ADC1.
 * @return  Muestra convertida (tipo ADCData_t).
 * @note    Debe llamarse luego de que COCO indique fin de conversión.
 */
ADCData_t 	ADC_getData 		   (ADC_t);

#endif /* SOURCES_TEMPLATE_ADC_H_ */
