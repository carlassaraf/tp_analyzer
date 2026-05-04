#ifndef ST7789_H
#define ST7789_H

#include <stdint.h>
#include <stddef.h>
#include "hal/hal_spi.h"

typedef struct {
    hal_spi_config_t spi_config;
    uint8_t pin_dc;
    uint8_t pin_rst;
    uint8_t pin_bl;
    uint16_t width;
    uint16_t height;
    uint8_t rotation;
} st7789_config_t;

typedef struct {
    const st7789_config_t *config;
} st7789_t;

st7789_t *st7789_init(const st7789_config_t *config);

// Set the write window in controller coordinates (already offset-adjusted by caller).
void st7789_set_window(st7789_t *ctx, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

// Send raw pixel data (RGB565, 2 bytes/pixel) after st7789_set_window.
void st7789_send_pixels(st7789_t *ctx, const uint8_t *data, size_t len);

#endif
