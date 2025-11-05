#ifndef _NCO_H_
#define _NCO_H_

#include <stdint.h>
#include <stdbool.h>

// ---- Compile-time knobs ----
#ifndef NCO_PHASE_BITS
#define NCO_PHASE_BITS (32u)
#endif

#define FS      (50000u)  // frecuencia de muestreo

#define TW32_ROUND(f, FS) ( (uint32_t)((((uint64_t)(f) << 32) + ((FS)/2)) / (uint64_t)(FS)) )
#define K_MARK  TW32_ROUND(1200u,  FS)
#define K_SPACE TW32_ROUND(2200u,  FS)

// If you want linear interpolation between LUT points, enable this.
// #define NCO_ENABLE_LERP

typedef struct {
    uint32_t phase;     // 32-bit phase accumulator
    uint32_t K;         // current tuning word
    uint32_t K_mark;    // precomputed MARK tuning word
    uint32_t K_space;   // precomputed SPACE tuning word
} NCO_Handle;

/* ------------ Core ------------ */

// Initialize with precomputed tuning words. K_init_is_mark selects start tone.
void     NCO_InitFixed(NCO_Handle* nco, uint32_t K_mark, uint32_t K_space, bool K_init_is_mark);

// Set tuning word directly (rarely needed if you only call NCO_FskBit).
static inline void NCO_SetTuningWord(NCO_Handle* nco, uint32_t K) { nco->K = K; }

// Advance 1 sample and return Q15 from LUT (uses external SINE_Q15).
int16_t NCO_TickQ15(NCO_Handle* nco);

/* -------- Conversions -------- */

// Map Q15 sample to 12-bit DAC code centered at dac_mid with amplitude dac_amp.
uint16_t NCO_Q15ToDAC12(int16_t q15, uint16_t dac_mid, uint16_t dac_amp);

// Convenience: tick and return DAC12 value.
static inline uint16_t NCO_TickDAC12(NCO_Handle* nco, uint16_t dac_mid, uint16_t dac_amp) {
    return NCO_Q15ToDAC12(NCO_TickQ15(nco), dac_mid, dac_amp);
}

// Map Q15 sample to PWM duty [0..MOD].
uint16_t NCO_Q15ToPWMDutyMOD(int16_t q15, uint16_t mod);

// Convenience: tick and return PWM duty.
static inline uint16_t NCO_TickPWMDutyMOD(NCO_Handle* nco, uint16_t mod) {
    return NCO_Q15ToPWMDutyMOD(NCO_TickQ15(nco), mod);
}

/* -------------- FSK -------------- */

// Switch tones on bit boundary; preserves phase continuity.
static inline void NCO_FskBit(NCO_Handle* nco, bool bit /*1=MARK, 0=SPACE*/) {
    if (bit){
        nco->K = nco->K_mark;
    } else {
        nco->K = nco->K_space;
    }
}

#endif // _NCO_H_
