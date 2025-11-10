/**
 * @file NCO.h
 * @brief Archivo de cabecera para el driver del Oscilador Controlado Numéricamente (NCO).
 *
 * Este driver proporciona funcionalidad para un Oscilador Controlado Numéricamente (NCO)
 * utilizado en la generación de señales, particularmente para modulación por Desplazamiento de Frecuencia (FSK).
 * Soporta acumulación de fase, gestión de palabras de sintonía y conversiones a salidas DAC o PWM.
 * El NCO utiliza una tabla de búsqueda (LUT) para la generación de ondas sinusoidales y soporta interpolación lineal opcional.
 */

#ifndef _NCO_H_
#define _NCO_H_

#include <stdint.h>
#include <stdbool.h>

// ---- Perillas de tiempo de compilación ----
#ifndef NCO_PHASE_BITS
#define NCO_PHASE_BITS (32u)
#endif

#define FS      (50000u)  // frecuencia de muestreo

#define TW32_ROUND(f, FS) ( (uint32_t)((((uint64_t)(f) << 32) + ((FS)/2)) / (uint64_t)(FS)) )
#define K_MARK  TW32_ROUND(1200u,  FS)
#define K_SPACE TW32_ROUND(2200u,  FS)

// Si deseas interpolación lineal entre puntos de la LUT, habilita esto.
// #define NCO_ENABLE_LERP

/**
 * @brief Estructura que representa el manejador del NCO.
 *
 * Esta estructura mantiene el estado del NCO, incluyendo el acumulador de fase
 * y las palabras de sintonía para frecuencias MARK y SPACE en modo FSK.
 */
typedef struct {
    uint32_t phase;     /**< Acumulador de fase de 32 bits. */
    uint32_t K;         /**< Palabra de sintonía actual. */
    uint32_t K_mark;    /**< Palabra de sintonía precomputada para MARK. */
    uint32_t K_space;   /**< Palabra de sintonía precomputada para SPACE. */
} NCO_Handle;

/* ------------ Núcleo ------------ */

/**
 * @brief Inicializa el NCO con palabras de sintonía fijas para MARK y SPACE.
 *
 * Configura el NCO con palabras de sintonía precomputadas y selecciona el tono inicial.
 *
 * @param nco Puntero a la estructura NCO_Handle.
 * @param K_mark Palabra de sintonía para frecuencia MARK.
 * @param K_space Palabra de sintonía para frecuencia SPACE.
 * @param K_init_is_mark Verdadero para iniciar con tono MARK, falso para SPACE.
 */
void     NCO_InitFixed(NCO_Handle* nco, uint32_t K_mark, uint32_t K_space, bool K_init_is_mark);

/**
 * @brief Establece la palabra de sintonía directamente.
 *
 * Esta función rara vez se necesita si se utiliza NCO_FskBit para modulación FSK.
 *
 * @param nco Puntero a la estructura NCO_Handle.
 * @param K La nueva palabra de sintonía a establecer.
 */
static inline void NCO_SetTuningWord(NCO_Handle* nco, uint32_t K) { nco->K = K; }

/**
 * @brief Avanza el NCO en una muestra y devuelve el valor sinusoidal en formato Q15.
 *
 * Utiliza una LUT externa de seno (SINE_Q15) para generar la salida. Soporta interpolación
 * lineal opcional si NCO_ENABLE_LERP está definido.
 *
 * @param nco Puntero a la estructura NCO_Handle.
 * @return El valor sinusoidal formateado en Q15.
 */
int16_t NCO_TickQ15(NCO_Handle* nco);

/* -------- Conversiones -------- */

/**
 * @brief Mapea una muestra Q15 a un código DAC de 12 bits.
 *
 * Centra la salida en dac_mid con la amplitud especificada.
 *
 * @param q15 El valor de entrada Q15.
 * @param dac_mid El valor central para la salida DAC.
 * @param dac_amp La amplitud para la salida DAC.
 * @return El código DAC de 12 bits.
 */
uint16_t NCO_Q15ToDAC12(int16_t q15, uint16_t dac_mid, uint16_t dac_amp);

/**
 * @brief Función de conveniencia para avanzar el NCO y devolver un código DAC de 12 bits.
 *
 * Combina NCO_TickQ15 y NCO_Q15ToDAC12.
 *
 * @param nco Puntero a la estructura NCO_Handle.
 * @param dac_mid El valor central para la salida DAC.
 * @param dac_amp La amplitud para la salida DAC.
 * @return El código DAC de 12 bits.
 */
static inline uint16_t NCO_TickDAC12(NCO_Handle* nco, uint16_t dac_mid, uint16_t dac_amp) {
    return NCO_Q15ToDAC12(NCO_TickQ15(nco), dac_mid, dac_amp);
}

/**
 * @brief Mapea una muestra Q15 a un ciclo de trabajo PWM en el rango [0..MOD].
 *
 * Convierte el valor Q15 firmado a un ciclo de trabajo sin signo con redondeo.
 *
 * @param q15 El valor de entrada Q15.
 * @param mod El módulo máximo de PWM.
 * @return El ciclo de trabajo PWM.
 */
uint16_t NCO_Q15ToPWMDutyMOD(int16_t q15, uint16_t mod);

/**
 * @brief Función de conveniencia para avanzar el NCO y devolver un ciclo de trabajo PWM.
 *
 * Combina NCO_TickQ15 y NCO_Q15ToPWMDutyMOD.
 *
 * @param nco Puntero a la estructura NCO_Handle.
 * @param mod El módulo máximo de PWM.
 * @return El ciclo de trabajo PWM.
 */
static inline uint16_t NCO_TickPWMDutyMOD(NCO_Handle* nco, uint16_t mod) {
    return NCO_Q15ToPWMDutyMOD(NCO_TickQ15(nco), mod);
}

/* -------------- FSK -------------- */

/**
 * @brief Cambia el tono del NCO basado en un bit FSK.
 *
 * Preserva la continuidad de fase al cambiar entre tonos MARK (1) y SPACE (0).
 *
 * @param nco Puntero a la estructura NCO_Handle.
 * @param bit El bit a modular: verdadero (1) para MARK, falso (0) para SPACE.
 */
static inline void NCO_FskBit(NCO_Handle* nco, bool bit /*1=MARK, 0=SPACE*/) {
    if (bit){
        nco->K = nco->K_mark;
    } else {
        nco->K = nco->K_space;
    }
}

#endif // _NCO_H_