#ifndef ILI9486_H
#define ILI9486_H

#include <stdint.h>
#include <stddef.h>

// D0-D7 must be 8 consecutive GPIO pins starting at data_pin_base.
typedef struct {
    uint8_t data_pin_base; // GPIO of D0; D1..D7 = data_pin_base+1..+7
    uint8_t pin_wr;        // Write strobe, active low
    uint8_t pin_rd;        // Read strobe, active low (tie high if unused)
    uint8_t pin_rs;        // Register select: 0=command, 1=data
    uint8_t pin_cs;        // Chip select, active low
    uint8_t pin_rst;       // Reset, active low
    uint8_t pin_bl;        // Backlight enable, active high
    uint16_t width;
    uint16_t height;
    uint8_t rotation;      // 0=portrait, 1=landscape, 2=portrait-flip, 3=landscape-flip
} ili9486_config_t;

typedef struct {
    const ili9486_config_t *config;
    uint32_t data_mask;    // 0xFF << data_pin_base, precomputed
} ili9486_t;

ili9486_t *ili9486_init(const ili9486_config_t *config);
void ili9486_set_window(ili9486_t *ctx, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ili9486_send_pixels(ili9486_t *ctx, const uint8_t *data, size_t len);

#endif
