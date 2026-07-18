#ifndef DRV_MATSTREAM_H_
#define DRV_MATSTREAM_H_

#include <stdint.h>
#include <stddef.h>

void dispBus_init(void);
void loadDisplay(uint32_t *word, size_t n);
void WS2812_Update(void);

#endif /* DRV_MATSTREAM_H_ */
