/**
 * @file DAC.h
 * @brief Archivo de cabecera para el driver del Convertidor Digital-Analógico (DAC) en Kinetis.
 *
 * Este driver proporciona funcionalidad básica para inicializar y escribir datos en los DAC0 y DAC1
 * del microcontrolador Kinetis (MK64F12). Soporta configuración de clocks, referencia y trigger por software.
 */

#ifndef DAC_H_
#define DAC_H_

#include "hardware.h"
#include "MK64F12.h"

/**
 * @brief Tipo de puntero a la estructura del DAC (DAC_Type).
 */
typedef DAC_Type *DAC_t;

/**
 * @brief Tipo de datos para el valor digital a convertir (uint16_t).
 */
typedef uint16_t DACData_t;

/**
 * @brief Inicializa DAC0 y DAC1.
 *
 * Habilita los clocks en SIM_SCGC2 y configura el registro C0:
 * - DACEN: habilita el convertidor.
 * - DACRFS: referencia VDDA.
 * - DACTRGSEL: disparo por software.
 */
void DAC_Init	 (void);

/**
 * @brief Escribe un valor al DAC (canal 0).
 *
 * Carga los registros DATL y DATH con el dato dividido en low y high.
 *
 * @param dac Puntero al DAC (DAC0 o DAC1).
 * @param data Valor digital a convertir (ancho según resolución del DAC).
 */
void DAC_SetData (DAC_t, DACData_t);

/**
 * @brief Función de servicio de interrupción para el DAC (PISR).
 *
 * Esta función se llama en respuesta a interrupciones del DAC. Su implementación
 * debe definirse según las necesidades de la aplicación.
 */
void DAC_PISR (void);

#endif /* DAC_H_ */