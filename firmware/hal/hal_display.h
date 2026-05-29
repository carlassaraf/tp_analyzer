#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t x_offset;
    uint16_t y_offset;
} hal_display_info_t;

// Backend flush signature: set window + send pixels, call done_cb when transfer is complete.
// For synchronous drivers, done_cb must be called before returning.
// For async DMA drivers, done_cb is called from the transfer-complete ISR.
typedef void (*hal_display_flush_fn)(uint16_t x1, uint16_t y1,
                                     uint16_t x2, uint16_t y2,
                                     const uint8_t *buf, size_t len,
                                     void (*done_cb)(void));

// Called once during init to wire up the display driver backend.
void hal_display_set_backend(hal_display_flush_fn flush_fn, const hal_display_info_t *info);

const hal_display_info_t *hal_display_get_info(void);

void hal_display_flush(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                       const uint8_t *buf, size_t len, void (*done_cb)(void));

#endif
