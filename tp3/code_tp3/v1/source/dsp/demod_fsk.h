/**
 * @file demod_fsk.h
 * @brief FSK demodulator public interface.
 *
 * This header declares the functions used to demodulate FSK signals from ADC
 * samples, reconstruct a bitstream from filtered demodulated samples, and to
 * query / retrieve the reconstructed bitstream.
 *
 * Notes:
 * - Functions operate on internal state; callers should not attempt to free
 *   returned pointers or directly modify internal buffers.
 * - Timing / calling sequence:
 *     1. Call demodFSK() for each ADC sample (uint16_t).
 *     2. Feed the resulting float to bitstreamReconstruction() after any FIR
 *        filtering you perform.
 *     3. Use isDataReady() to detect when a complete bit buffer is available.
 *     4. Call retrieveBitstream() to obtain a pointer to the internal buffer.
 */

#ifndef _DEMOD_FSK_
#define _DEMOD_FSK_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Demodulate a single ADC sample.
 *
 * Processes one raw ADC sample and returns the instantaneous demodulated
 * value. The return value is a signed float representing the discriminator /
 * frequency-deviation output (soft sample).
 *
 * @param adc_value  Raw ADC sample (12/16-bit depending on ADC configuration).
 * @return float     Instantaneous demodulated output (signed).
 */
float demodFSK(uint16_t adc_value);

/**
 * @brief Reconstruct bitstream from filtered demodulated output.
 *
 * Call this with the FIR-filtered demodulator output (soft sample). The
 * function updates internal bitstream reconstruction state and buffers.
 *
 * @param fir_output  Filtered demodulator output (float).
 */
void bitstreamReconstruction(float fir_output);

/**
 * @brief Check if reconstructed data is ready to be retrieved.
 *
 * Returns true when a complete bit buffer / frame is available for reading.
 * After retrieveBitstream() has been called the ready flag may be cleared by
 * the implementation.
 *
 * @return true  Data ready to retrieve.
 * @return false Data not ready yet.
 */
bool isDataReady(void);

/**
 * @brief Retrieve pointer to the reconstructed bitstream buffer.
 *
 * Returns a pointer to the internal bit buffer containing the last-completed
 * reconstructed bits. The returned pointer is owned by the demodulator module;
 * do not free or modify the buffer contents directly. The format (length,
 * bit-order) depends on the implementation and should be documented where the
 * module is implemented.
 *
 * @return bool*  Pointer to internal bit buffer (NULL if no data available).
 */
bool *retrieveBitstream(void);

#endif // _DEMOD_FSK_