#include <stddef.h>

#include "process.h"
#include "fir_coefs.h"
#include "../drv/mcal/ADC.h"

#define DELAY 5

static uint16_t x[DELAY + 1];
static uint16_t sample_cnt;
static size_t index;
static uint16_t m[N];

double processData()
{
    x[index] = ADC_getData();

    m[index] = x[index] * x[mod(index - DELAY)];

    for (int k = 0; k < N; k++)
    {
        d[i] = m[i] * h[i - k];
    }

}