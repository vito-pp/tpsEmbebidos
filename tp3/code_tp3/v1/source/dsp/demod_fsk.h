/**
 * @file demod_fsk.h
 * @brief Archivo de cabecera para la demodulación FSK.
 *
 * Este archivo declara funciones para demodular señales FSK, reconstruir el bitstream,
 * verificar si hay datos listos y recuperar el bitstream.
 */

#ifndef _DEMOD_FSK_
#define _DEMOD_FSK_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Demodula una muestra ADC en formato FSK.
 *
 * Procesa la señal FSK usando un retraso y filtro FIR para obtener la salida demodulada.
 *
 * @param adc_value Valor de la muestra ADC (uint16_t).
 * @return Salida demodulada escalada (float).
 */
float demodFSK(uint16_t adc_value);

/**
 * @brief Reconstruye el bitstream a partir de la salida del FIR.
 *
 * Detecta el inicio de bits, realiza oversampling y decide bits por mayoría.
 * Debe llamarse después de cada fin de conversión ADC.
 *
 * @param fir_output Salida del filtro FIR.
 */
void bitstreamReconstruction(float fir_output);

/**
 * @brief Verifica si hay datos listos (frame UART completo recibido).
 *
 * @return True si datos listos, false caso contrario.
 */
bool isDataReady(void);

/**
 * @brief Recupera el bitstream reconstruido (11 bits).
 *
 * Limpia el flag de datos listos.
 *
 * @return Puntero al arreglo de bits booleanos.
 */
bool *retrieveBitstream(void);

#endif // _DEMOD_FSK_