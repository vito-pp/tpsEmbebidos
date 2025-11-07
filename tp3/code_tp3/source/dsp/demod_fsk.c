#include "demod_fsk.h"
#include "fir_coefs.h"

#define DELAY 5    /* optimal delay is 446us. so DELAY = (int)446us * FS */
#define SAMPLES_PER_BIT 10    /* FS_DAC / baud */
#define ADC_MAX_VAL (1 << 12)   /* ADC no. of bits */
#define UART_LEN 11
#define SCALE 2048.0f

// i dont want them calculated at runtime
static const uint8_t HALF_SAMPLES_BIT = SAMPLES_PER_BIT >> 1;
static const uint16_t ADC_DC_VAL = ADC_MAX_VAL >> 1;

static bool idle = true;
static bool data_ready;
static uint8_t samples_per_bit_cnt;

static bool bits_recovered[UART_LEN];
static uint8_t bit_cnt;

// remember to enable FPU in main
float demodFSK(uint16_t adc_value)
{
    static float x[DELAY + 1] = {0};  // the FSK signal read form the ADC
    static float m[N] = {0};          // product of FSK and FSK delayed 
    static uint8_t curr = 0;          // current index of delay array
    static uint8_t i = 0;             // fir index

    // get value form ADC
    x[curr] = ((float)adc_value - ADC_DC_VAL) / SCALE; /* substract DC value 
    and normalize to [-1, +1]*/

    // get the product of the fsk and delayed fsk
    m[i] = x[curr] * x[(curr + 1) % (DELAY + 1)];

    // calculate FIR
    float d = 0.0f;
    int r = i;
    for (int k = 0; k < N; k++)
    {
        d += m[r] * h[k];
        if (--r < 0)
            r = N - 1;
    }

    // advance indices ring buffer style
    curr = (curr + 1) % (DELAY + 1);
    i = (i + 1) % N;

    return d * SCALE; // scale back
}

// to be called after each ADC EOC. has to be faster than 1 / FS_ADC
void bitstreamReconstruction(float fir_output)
{
    static uint8_t bit_democracy[3]; // oversampled bits
    static uint8_t j = 0;         // bit_democracy index

    if (idle)
    {
        if (fir_output > 0) // bit start detected
        {
            idle = false;
            samples_per_bit_cnt = 1;
            bit_cnt = 0;
        }
    }
    else // recieving data
    {
        samples_per_bit_cnt++;
        // oversampling = 3
        if ((samples_per_bit_cnt >= (HALF_SAMPLES_BIT) - 1) && 
            (samples_per_bit_cnt <= (HALF_SAMPLES_BIT) + 1)) 
        {
            bit_democracy[j] = (fir_output < 0);
            j++;
        }
        else if (samples_per_bit_cnt == SAMPLES_PER_BIT)
        {
            // we read all samples per bit: now bit election, majority wins
            if (bit_democracy[0] + bit_democracy[1] + bit_democracy[2] >= 2)
            {
                bits_recovered[bit_cnt] = 1;
            }
            else 
            {
                bits_recovered[bit_cnt] = 0;
            }
            bit_cnt++;
            samples_per_bit_cnt = 0;
            j = 0;
        }
        else; // ignore samples

        if (bit_cnt == UART_LEN) // full UART frame has been read
        {
            idle = true;
            data_ready = true;
        }
    }
}

bool isDataReady(void)
{
    return data_ready;
}

bool *retrieveBitstream(void)
{
    data_ready = false;
    return bits_recovered;
}
