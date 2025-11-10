/**
 * @file NCO.h
 * @brief Numerically Controlled Oscillator (NCO) interface for FSK generation.
 *
 * Provides a lightweight phase-accumulator based NCO that emits Q15 samples
 * from a sine lookup table and helpers to convert Q15 samples to DAC/PWM
 * outputs. Also includes utilities for FSK tone switching (MARK/SPACE).
 *
 * Design notes:
 * - Phase accumulator width is configurable via NCO_PHASE_BITS.
 * - Tuning words are 32-bit fixed-point values representing frequency.
 * - Functions are intended for use in embedded ISR/fast paths.
 * 
 */

#ifndef _NCO_H_
#define _NCO_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @def NCO_PHASE_BITS
 * @brief Number of bits used for the phase accumulator (default: 32).
 *
 * Controls wrap-around and tuning resolution. Override at compile time to
 * use a different accumulator width.
 */
#ifndef NCO_PHASE_BITS
#define NCO_PHASE_BITS (32u)
#endif

/**
 * @def FS
 * @brief Sampling frequency used for tuning word macros (Hz).
 */
#define FS      (50000u)  // frecuencia de muestreo

/**
 * @def TW32_ROUND
 * @brief Convert a frequency (Hz) to a 32-bit tuning word with rounding.
 *
 * Formula: K = round( (f / FS) * 2^32 )
 *
 * @param f   Frequency in Hz.
 * @param FS  Sampling frequency in Hz.
 */
#define TW32_ROUND(f, FS) ( (uint32_t)((((uint64_t)(f) << 32) + ((FS)/2)) / (uint64_t)(FS)) )

/**
 * @brief Precomputed tuning word for MARK tone (1200 Hz).
 */
#define K_MARK  TW32_ROUND(1200u,  FS)

/**
 * @brief Precomputed tuning word for SPACE tone (2200 Hz).
 */
#define K_SPACE TW32_ROUND(2200u,  FS)

/**
 * @brief Enable linear interpolation between sine LUT points.
 *
 * If defined, the implementation may perform simple linear interpolation to
 * reduce quantization error from the LUT. Disabled by default.
 */
// #define NCO_ENABLE_LERP

/**
 * @struct NCO_Handle
 * @brief Runtime state for a single NCO instance.
 *
 * Fields:
 * - phase: phase accumulator (wraps naturally).
 * - K: current tuning word (adds to phase each tick).
 * - K_mark/K_space: precomputed tuning words for FSK tones.
 */
typedef struct {
    uint32_t phase;     /**< 32-bit phase accumulator */
    uint32_t K;         /**< current tuning word */
    uint32_t K_mark;    /**< precomputed MARK tuning word */
    uint32_t K_space;   /**< precomputed SPACE tuning word */
} NCO_Handle;

/* ------------ Core ------------ */

/**
 * @brief Initialize an NCO with fixed tuning words.
 *
 * Sets the MARK/SPACE tuning words and initializes the current tuning word to
 * K_mark or K_space depending on K_init_is_mark. The phase accumulator is
 * typically cleared.
 *
 * @param nco           Pointer to NCO_Handle to initialize.
 * @param K_mark        Precomputed tuning word for MARK tone.
 * @param K_space       Precomputed tuning word for SPACE tone.
 * @param K_init_is_mark If true, start with MARK tuning word; otherwise SPACE.
 */
void NCO_InitFixed(NCO_Handle* nco, uint32_t K_mark, uint32_t K_space,
                   bool K_init_is_mark);

/**
 * @brief Set the NCO tuning word directly.
 *
 * Inline helper to change the current tuning word. This does not modify the
 * stored K_mark/K_space fields.
 *
 * @param nco Pointer to NCO_Handle.
 * @param K   New tuning word.
 */
static inline void NCO_SetTuningWord(NCO_Handle* nco, uint32_t K) 
{ 
    nco->K = K; 
}

/**
 * @brief Advance the NCO one sample and return a Q15 sine sample.
 *
 * Advances the phase accumulator by the current tuning word and looks up the
 * corresponding sine value in a Q15-formatted LUT. The returned value is
 * signed 16-bit where -32768..32767 maps to -1..(1-2^-15).
 *
 * @param nco Pointer to NCO_Handle.
 * @return int16_t Q15 sample.
 */
int16_t NCO_TickQ15(NCO_Handle* nco);

/* -------- Conversions -------- */

/**
 * @brief Map a Q15 sample to a 12-bit DAC code centered at dac_mid.
 *
 * Scales and offsets the signed Q15 sample into an unsigned 12-bit DAC value
 * in the range [dac_mid - dac_amp, dac_mid + dac_amp].
 *
 * @param q15      Signed Q15 sample (int16_t).
 * @param dac_mid  DAC center value (e.g., 2048 for 12-bit).
 * @param dac_amp  Peak amplitude in DAC units.
 * @return uint16_t  12-bit DAC code.
 */
uint16_t NCO_Q15ToDAC12(int16_t q15, uint16_t dac_mid, uint16_t dac_amp);

/**
 * @brief Tick the NCO and return a 12-bit DAC value (convenience).
 *
 * Combines NCO_TickQ15 and NCO_Q15ToDAC12.
 *
 * @param nco      Pointer to NCO_Handle.
 * @param dac_mid  DAC center value.
 * @param dac_amp  DAC amplitude.
 * @return uint16_t  12-bit DAC code.
 */
static inline uint16_t NCO_TickDAC12(NCO_Handle* nco, uint16_t dac_mid, 
                                     uint16_t dac_amp) 
{
    return NCO_Q15ToDAC12(NCO_TickQ15(nco), dac_mid, dac_amp);
}

/**
 * @brief Map a Q15 sample to a PWM duty in the range [0..mod].
 *
 * Scales the signed Q15 value into an unsigned PWM duty within the timer's
 * MOD range.
 *
 * @param q15  Signed Q15 sample.
 * @param mod  Timer PWM modulus value (maximum duty).
 * @return uint16_t PWM duty value.
 */
uint16_t NCO_Q15ToPWMDutyMOD(int16_t q15, uint16_t mod);

/**
 * @brief Tick the NCO and return a PWM duty (convenience).
 *
 * Combines NCO_TickQ15 and NCO_Q15ToPWMDutyMOD.
 *
 * @param nco Pointer to NCO_Handle.
 * @param mod Timer PWM modulus.
 * @return uint16_t PWM duty.
 */
static inline uint16_t NCO_TickPWMDutyMOD(NCO_Handle* nco, uint16_t mod) 
{
    return NCO_Q15ToPWMDutyMOD(NCO_TickQ15(nco), mod);
}

/* -------------- FSK -------------- */

/**
 * @brief Switch NCO tone according to FSK bit while preserving phase continuity.
 *
 * Selects the MARK or SPACE tuning word on a bit boundary. This function only
 * updates the tuning word; the phase accumulator is left intact to ensure
 * continuous phase across tone changes.
 *
 * @param nco Pointer to NCO_Handle.
 * @param bit If true select MARK tone, if false select SPACE tone.
 */
static inline void NCO_FskBit(NCO_Handle* nco, bool bit /*1=MARK, 0=SPACE*/) 
{
    if (bit){
        nco->K = nco->K_mark;
    } else {
        nco->K = nco->K_space;
    }
}

#endif // _NCO_H_