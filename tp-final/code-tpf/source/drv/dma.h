#ifndef DRV_DMA_H_
#define DRV_DMA_H_

#include <stdint.h>
#include <stdbool.h>

#define DMA_NUM_CH 16u

typedef enum {
    DMA_REQ_DISABLED = 0,

    DMA_REQ_FTM0CH0 = 20,
    DMA_REQ_FTM0CH1 = 21,
    DMA_REQ_FTM0CH2 = 22,
    DMA_REQ_FTM0CH3 = 23,
    DMA_REQ_FTM0CH4 = 24,
    DMA_REQ_FTM0CH5 = 25,
    DMA_REQ_FTM0CH6 = 26,
    DMA_REQ_FTM0CH7 = 27,

    DMA_REQ_FTM1CH0 = 28,
    DMA_REQ_FTM1CH1 = 29,

    DMA_REQ_FTM2CH0 = 30,
    DMA_REQ_FTM2CH1 = 31,

    DMA_REQ_FTM3CH0 = 32,
    DMA_REQ_FTM3CH1 = 33,
    DMA_REQ_FTM3CH2 = 34,
    DMA_REQ_FTM3CH3 = 35,
    DMA_REQ_FTM3CH4 = 36,
    DMA_REQ_FTM3CH5 = 37,
    DMA_REQ_FTM3CH6 = 38,
    DMA_REQ_FTM3CH7 = 39
} dma_req_e;

typedef void (*dma_cb_t)(void *user);

typedef struct {
    uint8_t ch;
    dma_req_e request_src;

    void *saddr;
    void *daddr;

    uint8_t elem_size;

    int16_t soff;
    int16_t doff;

    uint16_t major_count;

    int32_t slast;
    int32_t dlast;

    bool int_major;

    dma_cb_t on_major;
    void *user;
} dma_cfg_t;

int DMA_Init(void);
int DMA_Config(const dma_cfg_t *cfg);
int DMA_Start(uint8_t ch);
int DMA_Stop(uint8_t ch);

#endif /* DRV_DMA_H_ */
