/**
 * @file bitstream.h
 * @brief Utilities to format and parse UART-style bitstreams.
 *
 * This header provides helpers to convert between 8-bit data bytes and a
 * UART-like framed bitstream (1 start, 8 data, 1 parity, 1 stop = 11 bits).
 * It also offers functions to compute even parity and to format/deformat
 * boolean bit arrays used by the demodulator/reconstruction code.
 *
 * Notes:
 * - The bit order and framing convention used here must match the demodulator
 *   and the receiver higher-level protocols.
 * - Array arguments for formatted bitstreams are expected to have length
 *   BITSTREAM_SIZE (11).
 */

#ifndef BITSTREAM
#define BITSTREAM

#include <stdint.h>
#include <stdbool.h>

/** Number of bits in a framed UART-like stream: start + 8 data + parity + stop */
#define BITSTREAM_SIZE 11u // 1 start + 8 data + 1 parity + 1 stop

/**
 * @brief Format a data byte into a 11-bit UART frame stored in a 16-bit value.
 *
 * Creates a framed word with: 1 start bit (0), 8 LSB-first data bits, even
 * parity bit, and 1 stop bit (1). The returned uint16_t holds the 11-bit
 * frame in its LSB..MSB bits.
 *
 * @param data  8-bit data byte to frame.
 * @return uint16_t  Framed 11-bit word (stored in LSB..).
 */
uint16_t data_to_uart(uint8_t data);

/**
 * @brief Extract the data byte from a framed 11-bit UART word.
 *
 * Inverse of data_to_uart: extracts the 8 data bits from a 11-bit framed word.
 * This function does not validate parity or framing bits — callers should
 * perform checks if required.
 *
 * @param frame  11-bit framed word packed in a uint16_t (LSB.. contains frame).
 * @return uint8_t  Extracted 8-bit data byte.
 */
uint8_t uart_to_data(uint16_t frame);

/**
 * @brief Compute even parity bit for a data byte.
 *
 * Returns true if the number of '1' bits in data is odd (parity bit = 1) so
 * the overall parity becomes even when included; returns false otherwise.
 *
 * @param data  8-bit data value.
 * @return bool  Parity bit (true => parity = 1, false => parity = 0).
 */
bool parity_bit(const uint8_t data);

/**
 * @brief Format a data byte into an array of 11 boolean bits.
 *
 * Fills the provided output array (must be at least BITSTREAM_SIZE long) with
 * the framed bits in order: start, data[0]..data[7] (LSB first), parity, stop.
 *
 * @param data  8-bit data byte to format.
 * @param out   Output array of booleans with length >= BITSTREAM_SIZE.
 */
void format_bitstream(uint8_t data, bool out[11]);

/**
 * @brief Deformat an 11-bit boolean array into a data character.
 *
 * Extracts the 8 data bits from the input boolean frame (start, data[0..7],
 * parity, stop). This returns the reconstructed byte as a char. This function
 * does not validate parity or framing bits.
 *
 * @param in  Input boolean array containing BITSTREAM_SIZE bits.
 * @return char  Recovered data byte (as char).
 */
char deformat_bitstream(bool in[11]);

#endif // BITSTREAM