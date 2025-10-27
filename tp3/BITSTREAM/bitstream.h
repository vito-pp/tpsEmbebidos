#ifndef BITSTREAM
#define BITSTREAM

#include <stdint.h>
#include <stdbool.h>

#define BITSTREAM_SIZE 11u // 1 start + 8 data + 1 parity + 1 stop

// Formatea un byte de datos en un bitstream con start, parity y stop bits.
uint16_t format_to_uart(uint8_t data);

// Calcula el bit de paridad par para un byte de datos entrante.
bool parity_bit(const uint8_t data);

// Formatea un bitstream en un arreglo de 11 bits booleanos.
void format_bitstream(uint8_t data, bool out[11])

#endif // BITSTREAM