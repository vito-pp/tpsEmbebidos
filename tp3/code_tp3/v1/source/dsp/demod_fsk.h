#ifndef _DEMOD_FSK_
#define _DEMOD_FSK_

#include <stdint.h>
#include <stdbool.h>

float demodFSK(uint16_t adc_value);

void bitstreamReconstruction(float fir_output);

bool isDataReady(void);

bool *retrieveBitstream(void);

#endif // _DEMOD_FSK_