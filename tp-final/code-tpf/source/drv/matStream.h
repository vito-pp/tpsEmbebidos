#ifndef MATSTREAM_H
#define MATSTREAM_H

#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Starts a WS2812 frame transmission using DMA.
 *
 * Configures the DMA channel to transfer the duty cycle buffer to the FTM
 * channel value register (CnV), enables the DMA transfer, and marks the
 * display as busy until the major loop completion callback is executed.
 *
 * @note The duty cycle buffer must be filled before calling this function.
 *       The transmission completion is signaled by the
 *       WS2812_FrameDone() callback.
 */
void WS2812_Update(void);

/**
 * @brief Encodes LED color data into the WS2812 duty cycle buffer.
 *
 * Converts each 24-bit RGB word into the corresponding sequence of PWM duty
 * cycles required by the WS2812 protocol. Each bit is translated to either a
 * logical '0' or logical '1' duty cycle and stored in the DMA transmission
 * buffer.
 *
 * @param word Pointer to an array of 24-bit RGB values (one per LED).
 * @param n    Number of LEDs to encode.
 *
 * @note The generated duty cycle buffer must be transmitted by calling
 *       WS2812_Update().
 */
void loadDisplay(uint32_t * word, size_t n);

/**
 * @brief Initializes the WS2812 display bus.
 *
 * Initializes the FTM and DMA peripherals, sets a default duty cycle for all
 * DMA transfers, and configures the DMA channel used to update the FTM CnV
 * register. The DMA transfer is not started by this function.
 *
 * DMA configuration:
 *   - Trigger source : FTM3 Channel 0
 *   - Source buffer  : duty_cycles[]
 *   - Destination    : FTM3_C0V
 *   - Transfer size  : 16 bits
 *   - Interrupt      : Major loop completion
 *
 * @note Call this function once during system initialization before updating
 *       the LED matrix or starting a DMA transfer.
 */
void dispBus_init(void);

#endif
