/**
 * @file NCO.c
 * @brief Archivo de implementación para el driver del Oscilador Controlado Numéricamente (NCO).
 *
 * Este archivo contiene las implementaciones de las funciones declaradas en NCO.h.
 * Maneja la acumulación de fase, acceso a la LUT de seno y conversiones de salida.
 */

#include "NCO.h"
#include "SinLutQ15.h"   // SINE_Q15[LUT_SIZE], LUT_BITS/LUT_SIZE

#define NCO_PHASE_FRAC_BITS   (NCO_PHASE_BITS - LUT_BITS)
#define NCO_PHASE_FRAC_MASK   ((NCO_PHASE_FRAC_BITS >= NCO_PHASE_BOT) ?\
0xFFFFFFFFu : ((1u << NCO_PHASE_FRAC_BITS) - 1u))

/**
 * @brief Inicializa el NCO con palabras de sintonía fijas para MARK y SPACE.
 *
 * @param nco Puntero a la estructura NCO_Handle.
 * @param K_mark Palabra de sintonía para frecuencia MARK.
 * @param K_space Palabra de sintonía para frecuencia SPACE.
 * @param K_init_is_mark Verdadero para iniciar con tono MARK, falso para SPACE.
 */
void NCO_InitFixed(NCO_Handle* nco, uint32_t K_mark, uint32_t K_space, 
                   bool K_init_is_mark)
{
    nco->phase  = 0u;
    nco->K_mark = K_mark;
    nco->K_space= K_space;
    nco->K      = K_init_is_mark ? K_mark : K_space;
}

/**
 * @brief Avanza el NCO en una muestra y devuelve el valor sinusoidal en formato Q15.
 *
 * @param nco Puntero a la estructura NCO_Handle.
 * @return El valor sinusoidal formateado en Q15.
 */
int16_t NCO_TickQ15(NCO_Handle* nco)
{
    nco->phase += nco->K;  // se envuelve naturalmente
    const uint32_t idx = nco->phase >> NCO_PHASE_FRAC_BITS;

#if defined(NCO_ENABLE_LERP)
    const uint32_t frac = nco->phase & NCO_PHASE_FRAC_MASK;
    const int32_t  a    = (int32_t)SINE_Q15[idx];
    const int32_t  b    = (int32_t)SINE_Q15[(idx + 1u) & (LUT_SIZE - 1u)];
    const int32_t  diff = b - a;
    const int32_t  interp = a + (int32_t)(((int64_t)diff * (int64_t)frac) >> NCO_PHASE_FRAC_BITS);
    return (int16_t)interp;
#else
    return SINE_Q15[idx];
#endif
}

/**
 * @brief Mapea una muestra Q15 a un ciclo de trabajo PWM en el rango [0..MOD].
 *
 * @param q15 El valor de entrada Q15.
 * @param mod El módulo máximo de PWM.
 * @return El ciclo de trabajo PWM.
 */
uint16_t NCO_Q15ToPWMDutyMOD(int16_t q15, uint16_t mod)
{
    uint32_t u = (uint32_t)((int32_t)q15 + 32768);                  // [0..65535]
    uint32_t duty = ((uint64_t)u * (uint64_t)mod + 32767u) / 65535u; // redondeo
    if (duty > mod) duty = mod;
    return (uint16_t)duty;
}

/**
 * @brief Función de inicialización principal para el NCO.
 *
 * Llama a NCO_InitFixed con constantes predefinidas MARK y SPACE, iniciando con MARK.
 * Esta función se debe llamar una vez al inicio del programa.
 *
 * @param nco Puntero a la estructura NCO_Handle.
 */
void NCO_main_initialization(NCO_Handle* nco){
    NCO_InitFixed(nco, K_MARK, K_SPACE, true);
}
