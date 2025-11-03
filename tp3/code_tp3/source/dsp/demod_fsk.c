#include "demod_fsk.h"
#include "fir_coefs.h"

#define DELAY 5 /* optimal delay is 446us. so DELAY = 446us * FS */

static uint16_t x[DELAY + 1];    // the FSK signal read form the ADC
static uint8_t curr;    // current index of delay array
static float m[N];    // product of FSK signal and FSK delayed signal
static uint8_t i;    // fir index

float demodFSK(uint16_t adc_value)
{
    // get value form ADC
    x[curr] = adc_value;

    // get the product of the fsk and delayed fsk
    m[i] = (float)x[curr] * (float)x[(curr + 1) % (DELAY + 1)];
    curr = (curr + 1) % (DELAY + 1);

    // calculate FIR
    float d = 0.0f;
    int r = i;
    for (int k = 0; k < N; k++) 
    {
        d += m[r] * h[k];
        if (--r < 0) r = N-1;
    }

    uint8_t i_old = i;
    i = (i + 1) % N;
    
    // return m[i_old];
    return d;
    // return 1;
}