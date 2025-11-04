#include "demod_fsk.h"
#include "fir_coefs.h"
#include "ADC.h"
#include "fir_coefs.h"

#define TBAUD (1/1200)
#define DELAY 5            /* optimal delay is 446us. so DELAY = 446us * FS */
#define BITS_PER_SAMPLE 10 /*FS_DAC / baud*/
#define FSK_THRESHOLD (1 << 11)
#define WAITING_SAMPLES_INIT TBAUD * FS_ADC

static uint16_t x[DELAY + 1]; // the FSK signal read form the ADC
static uint8_t curr;          // current index of delay array
static float m[N];            // product of FSK signal and FSK delayed signal
static uint8_t i;             // fir index

static float last_filter_value;
static int waiting_samples = 0;
// estados

static bool reciving_data = false;
static bool starting_bit = false;
static int contador_bits = 0;

static bool idle = true;

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
        if (--r < 0)
            r = N - 1;
    }

    uint8_t i_old = i;
    i = (i + 1) % N;

    // return m[i_old];
    return d;
    // return 1;
}

bool bitDiscrimination(float fir_output)
{
    if (idle)
    {
        if (fir_output < (1 << 11)) // threshold
        {
            idle = false;
            return false; // bit start detected
        }
        else
            return true;
    }

    else
    {
    }
}
// Funcion a llamar a frecuencia Fs_ADC
// Falta calcular FS necesario para que TiemboBaud*Fs = 10
// O modificar la funcion para no usar 10 tipo magic value.
void get_filter_value(void)
{
    if (waiting_samples > 0)
    {
        uint16_t adc_value = ADC_getData(ADC0);
        last_filter_value = demodFSK(adc_value);
        if (!reciving_data && (last_filter_value < FSK_THRESHOLD))
        {
            starting_bit = true;
            reciving_data = true;
            waiting_samples = WAITING_SAMPLES_INIT;
        }
        if (starting_bit)
        {
            waiting_samples += 5;
            starting_bit = false;
        }

        contador_bits += 1;
        if (contador_bits == 11){
            reciving_data = 0;
            contador_bits = 0;
        }
    }
    waiting_samples--;
}
