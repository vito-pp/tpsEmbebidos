#ifndef DRV_FTM_H_
#define DRV_FTM_H_

#include <stdint.h>
#include "MK64F12.h"

/*
 * WS2812 timing:
 *
 * FTM clock = bus clock = 50 MHz
 * MOD = 61, so period = 62 ticks
 *
 * 50 MHz / 62 = ~806 kHz
 *
 * This is close to the WS2812 target of 800 kHz.
 */
#define WS2812_PWM_MOD      (62u - 1u)

#define WS2812_DUTY_0_PCT   34u
#define WS2812_DUTY_1_PCT   68u

#define DC2CNV(pct)         ((uint16_t)((WS2812_PWM_MOD * (pct)) / 100u))

void FTM_Init(void);
void PWM_setDuty(uint8_t duty_percent);

#endif /* DRV_FTM_H_ */
