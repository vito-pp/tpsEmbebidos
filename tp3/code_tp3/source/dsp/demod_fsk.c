#include "demod_fsk.h"
#include "fir_coefs.h"
#include "../drv/mcal/ADC.h"

#define DELAY 5

// true modulo function: a mod b
static unsigned int mod(int a, int b);

static uint16_t x[DELAY + 1];    // the FSK signal read form the ADC
static uint8_t curr;    // current index of delay array
static float m[N];    // product of FSK signal and FSK delayed signal
static uint8_t i;    // current write index

float demodFSK()
{
    // get value form ADC
    x[curr] = ADC_getData();

    // get the product of the fsk and delayed fsk
    m[i] = (float)x[curr] * (float)x[mod(curr - DELAY, DELAY + 1)];
    curr = mod(curr + 1, DELAY + 1);

    // calculate FIR
    float d = 0.0f;
    for (int k = 0; k < N; k++)
    {
        d += m[mod(i - k, N)] * h[k];
    }
    i = mod(i + 1, N);
    return d;
}

static unsigned int mod(int a, int b) 
{
    return ((a % b) + b) % b;
}