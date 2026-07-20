#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "gpio.h"
#include "rotary_encoder.h"
#include "rtos/encoder_task.h"

static uint8_t last_rotation_state;
static int8_t rotation_accumulator;

static void encoderRotationIRQ(void);
static void encoderButtonIRQ(void);

void encoderInit(void)
{
    gpioMode(PIN_ENC_A, INPUT);
    gpioMode(PIN_ENC_B, INPUT);
    gpioMode(PIN_ENC_C, INPUT);
    gpioMode(PIN_LED_BLUE, OUTPUT);

    last_rotation_state = ((gpioRead(PIN_ENC_A) & 0x1u) << 1u) |
                          (gpioRead(PIN_ENC_B) & 0x1u);
    rotation_accumulator = 0;

    gpioIRQ(PIN_ENC_A, PORT_PCR_IRQC_INT_EITHER, encoderRotationIRQ);
    gpioIRQ(PIN_ENC_B, PORT_PCR_IRQC_INT_EITHER, encoderRotationIRQ);
    gpioIRQ(PIN_ENC_C, PORT_PCR_IRQC_INT_EITHER, encoderButtonIRQ);
}

static void encoderRotationIRQ(void)
{
    uint8_t state;
    uint8_t transition;

    state = ((gpioRead(PIN_ENC_A) & 0x1u) << 1u) |
            (gpioRead(PIN_ENC_B) & 0x1u);

    if (state == last_rotation_state) {
        return;
    }

    transition = (last_rotation_state << 2u) | state;
    last_rotation_state = state;

    switch (transition) {
    case 0b1110:
    case 0b1000:
    case 0b0001:
    case 0b0111:
        rotation_accumulator++;
        break;

    case 0b1101:
    case 0b0100:
    case 0b0010:
    case 0b1011:
        rotation_accumulator--;
        break;

    default:
        rotation_accumulator = 0;
        break;
    }

    if (state == 0b11u) {
        if (rotation_accumulator >= 4) {
            EncoderTask_PostFromISR(ENCODER_RAW_CW);
        } else if (rotation_accumulator <= -4) {
            EncoderTask_PostFromISR(ENCODER_RAW_CCW);
        }
        rotation_accumulator = 0;
    }
}

static void encoderButtonIRQ(void)
{
    EncoderTask_PostFromISR(gpioRead(PIN_ENC_C) == LOW
                                ? ENCODER_RAW_BUTTON_DOWN
                                : ENCODER_RAW_BUTTON_UP);
}
