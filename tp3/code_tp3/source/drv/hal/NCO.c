#include "NCO.h"
#include "SinLutQ15.h"   // SINE_Q15[LUT_SIZE], LUT_BITS/LUT_SIZE

#define NCO_PHASE_FRAC_BITS   (NCO_PHASE_BITS - LUT_BITS)
#define NCO_PHASE_FRAC_MASK   ((NCO_PHASE_FRAC_BITS >= NCO_PHASE_BOT) ? \ 
0xFFFFFFFFu : ((1u << NCO_PHASE_FRAC_BITS) - 1u))

void NCO_InitFixed(NCO_Handle* nco, uint32_t K_mark, uint32_t K_space, 
                   bool K_init_is_mark)
{
    nco->phase  = 0u;
    nco->K_mark = K_mark;
    nco->K_space= K_space;
    nco->K      = K_init_is_mark ? K_mark : K_space;
}

int16_t NCO_TickQ15(NCO_Handle* nco)
{
    nco->phase += nco->K;  // wraps naturally
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
// Innecesario si utilizamos un DAC de 16 bits
// uint16_t NCO_Q15ToDAC12(int16_t q15, uint16_t dac_mid, uint16_t dac_amp)
// {
//     int32_t v   = ((int32_t)q15 * (int32_t)dac_amp) / 32767; // [-dac_amp..+dac_amp]
//     int32_t out = (int32_t)dac_mid + v;
//     if (out < 0) out = 0;
//     if (out > 4095) out = 4095;
//     return (uint16_t)out;
// }

// Version 2
uint16_t NCO_Q15ToPWMDutyMOD(int16_t q15, uint16_t mod)
{
    uint32_t u = (uint32_t)((int32_t)q15 + 32768);                  // [0..65535]
    uint32_t duty = ((uint64_t)u * (uint64_t)mod + 32767u) / 65535u; // round
    if (duty > mod) duty = mod;
    return (uint16_t)duty;
}

// Se llama una vez al inicio del programa
void NCO_main_initialization(NCO_Handle* nco){
    NCO_InitFixed(nco, K_MARK, K_SPACE, true);
}
