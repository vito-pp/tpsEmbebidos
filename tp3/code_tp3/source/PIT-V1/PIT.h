#pragma once
#include <stdint.h>
#include <stdbool.h>

/* Generic PIT driver (MK64F12)
 * - Configure channel frequency from BUS clock
 * - Register a callback per channel
 */

typedef void (*pit_cb_t)(void);

bool PIT_Init(uint8_t ch, uint32_t bus_hz, uint32_t freq_hz,
              uint8_t irq_prio, pit_cb_t cb);

bool PIT_SetFreq(uint8_t ch, uint32_t bus_hz, uint32_t freq_hz);

void PIT_Start(uint8_t ch);
void PIT_Stop(uint8_t ch);
