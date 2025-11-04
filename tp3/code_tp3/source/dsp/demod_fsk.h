#include <stdint.h>
#include <stdbool.h>

float demodFSK(uint16_t adc_value);

bool bitDiscrimination(float fir_output);
