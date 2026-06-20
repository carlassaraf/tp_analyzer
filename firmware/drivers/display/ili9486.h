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
    int8_t  pin_bl;        // Backlight enable, active high; -1 = not connected
    uint16_t width;
    uint16_t height;
    uint8_t rotation;      // 0=portrait, 1=landscape, 2=portrait-flip, 3=landscape-flip
} ili9486_config_t;

typedef struct ili9486_t ili9486_t;  // opaque — full definition in ili9486.c

ili9486_t *ili9486_init(const ili9486_config_t *config);
void ili9486_set_window(ili9486_t *ctx, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ili9486_send_pixels(ili9486_t *ctx, const uint8_t *data, size_t len);

// Start a DMA pixel transfer (non-blocking). Call ili9486_dma_wait() to block until done.
void ili9486_send_pixels_dma(ili9486_t *ctx, const uint8_t *data, size_t len);

// Block until the current DMA transfer is complete.
void ili9486_dma_wait(ili9486_t *ctx);

// Register a callback invoked from DMA_IRQ_1 when a pixel DMA transfer finishes.
// The handler runs in interrupt context; call ili9486_dma_irq_clear() first.
void ili9486_dma_set_completion_irq(ili9486_t *ctx, void (*handler)(void));

// Clear the DMA completion interrupt flag. Must be called at the top of the handler.
void ili9486_dma_irq_clear(ili9486_t *ctx);

#endif
