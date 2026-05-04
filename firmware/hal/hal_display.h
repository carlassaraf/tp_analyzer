#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

#include <stdint.h>

void hal_display_init(void);
void hal_display_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint16_t *buf);

#endif