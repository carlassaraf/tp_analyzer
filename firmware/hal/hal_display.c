#include "hal_display.h"

/**
 * Generic display initialization
 * Specific device drivers (e.g., ST7789) handle their own initialization
 */
void hal_display_init(void) {
    // This is now handled by driver initialization
    // e.g., st7789_init() in drivers/display/st7789.c
}

/**
 * Generic display flush (write buffer to VRAM)
 * Specific device drivers handle the actual protocol
 */
void hal_display_flush(int32_t x1, int32_t y1,
                       int32_t x2, int32_t y2,
                       const uint16_t *buf) {
    // This is now handled by LVGL callbacks to the driver
    // e.g., st7789_send_color() in drivers/display/st7789.c
}
