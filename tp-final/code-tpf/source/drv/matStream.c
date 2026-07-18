#include "drv/matStream.h"

#include <stdbool.h>

#include "drv/FTM.h"
#include "drv/dma.h"

#include "MK64F12.h"

#define WS2812_LED_COUNT       64u
#define WS2812_BITS_PER_LED    24u
#define DUTY_BUFFER_SIZE       (WS2812_LED_COUNT * WS2812_BITS_PER_LED)

#define LOGICAL_1              DC2CNV(WS2812_DUTY_1_PCT)
#define LOGICAL_0              DC2CNV(WS2812_DUTY_0_PCT)

static uint16_t duty_cycles[DUTY_BUFFER_SIZE];
static volatile bool sendingDMA = false;

static void WS2812_FrameDone(void *user)
{
    (void)user;

    DMA_Stop(0);

    /*
     * Hold line low after the frame.
     * This gives the WS2812 reset/latch time.
     */
    PWM_setDuty(0);

    sendingDMA = false;
}

void dispBus_init(void)
{
    uint32_t i;
    dma_cfg_t cfg;

    FTM_Init();
    DMA_Init();

    for (i = 0; i < DUTY_BUFFER_SIZE; i++) {
        duty_cycles[i] = DC2CNV(0);
    }

    cfg.ch = 0;
    cfg.request_src = DMA_REQ_FTM3CH0;

    cfg.saddr = duty_cycles;
    cfg.daddr = (void *)&(FTM3->CONTROLS[0].CnV);

    cfg.elem_size = 2;

    cfg.soff = 2;
    cfg.doff = 0;

    cfg.major_count = DUTY_BUFFER_SIZE;

    cfg.slast = -(int32_t)sizeof(duty_cycles);
    cfg.dlast = 0;

    cfg.int_major = true;
    cfg.on_major = WS2812_FrameDone;
    cfg.user = NULL;

    DMA_Config(&cfg);
}

void loadDisplay(uint32_t *word, size_t n)
{
    size_t i;
    uint8_t j;
    size_t k = 0;

    if (word == NULL) {
        return;
    }

    if (n > WS2812_LED_COUNT) {
        n = WS2812_LED_COUNT;
    }

    /*
     * Gonza's original implementation effectively maps each 24-bit LED word
     * into a sequence of PWM duty cycles.
     *
     * This keeps the same LSB-first bit order used by the working branch.
     */
    for (i = 0; i < n; i++) {
        for (j = 0; j < WS2812_BITS_PER_LED; j++) {
            if (word[i] & (1u << j)) {
                duty_cycles[k] = LOGICAL_1;
            } else {
                duty_cycles[k] = LOGICAL_0;
            }

            k++;
        }
    }

    /*
     * Turn off remaining LEDs if n < 64.
     */
    while (k < DUTY_BUFFER_SIZE) {
        duty_cycles[k] = LOGICAL_0;
        k++;
    }
}

void WS2812_Update(void)
{
    if (sendingDMA) {
        return;
    }

    sendingDMA = true;

    DMA_Start(0);
}
