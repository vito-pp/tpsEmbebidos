#ifndef ENCODER_TASK_H_
#define ENCODER_TASK_H_

typedef enum
{
    ENCODER_RAW_CW,
    ENCODER_RAW_CCW,
    ENCODER_RAW_BUTTON_DOWN,
    ENCODER_RAW_BUTTON_UP,
    ENCODER_RAW_RESET
} EncoderRawEvent_t;

void EncoderTask_Create(void);
void EncoderTask_Reset(void);
void EncoderTask_PostFromISR(EncoderRawEvent_t event);

#endif /* ENCODER_TASK_H_ */
