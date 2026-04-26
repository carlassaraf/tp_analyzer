#include "drivers/display/st7789.h"
#include "hal/hal_gpio.h"
#include "hal/hal_spi.h"
#include <pico/stdlib.h>

#define ST7789_NOP      0x00
#define ST7789_SWRESET  0x01
#define ST7789_SLPIN    0x10
#define ST7789_SLPOUT   0x11
#define ST7789_NORON    0x13
#define ST7789_INVOFF   0x20
#define ST7789_INVON    0x21
#define ST7789_DISPOFF  0x28
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_COLMOD   0x3A
#define ST7789_MADCTL   0x36

static st7789_t st7789_instance;

static void write_cmd(const st7789_config_t *cfg, uint8_t cmd) {
    hal_gpio_write(cfg->pin_dc, 0);
    hal_spi_write(&cfg->spi_config, &cmd, 1);
}

static void write_data(const st7789_config_t *cfg, const uint8_t *data, size_t len) {
    hal_gpio_write(cfg->pin_dc, 1);
    hal_spi_write(&cfg->spi_config, data, len);
}

static void send_cmd_params(const st7789_config_t *cfg, uint8_t cmd,
                            const uint8_t *params, size_t count) {
    write_cmd(cfg, cmd);
    if (count > 0) {
        write_data(cfg, params, count);
    }
}

static void hw_reset(const st7789_config_t *cfg) {
    hal_gpio_init(cfg->pin_rst);
    hal_gpio_set_dir(cfg->pin_rst, HAL_GPIO_OUT);
    hal_gpio_write(cfg->pin_rst, 1);
    sleep_ms(10);
    hal_gpio_write(cfg->pin_rst, 0);
    sleep_ms(10);
    hal_gpio_write(cfg->pin_rst, 1);
    sleep_ms(120);
}

st7789_t *st7789_init(const st7789_config_t *config) {
    st7789_instance.config = config;

    hal_gpio_init(config->pin_dc);
    hal_gpio_set_dir(config->pin_dc, HAL_GPIO_OUT);

    hal_spi_init(&config->spi_config);
    hw_reset(config);

    write_cmd(config, ST7789_SWRESET);
    sleep_ms(150);
    write_cmd(config, ST7789_SLPOUT);
    sleep_ms(10);

    // RGB565 color mode
    send_cmd_params(config, ST7789_COLMOD, (uint8_t[]){0x55}, 1);
    sleep_ms(10);

    // This panel has inverted colors by default
    write_cmd(config, ST7789_INVON);

    // Landscape orientation (MX + MV = 90° CW)
    send_cmd_params(config, ST7789_MADCTL, (uint8_t[]){0x60}, 1);

    write_cmd(config, ST7789_NORON);
    sleep_ms(10);
    write_cmd(config, ST7789_DISPON);
    sleep_ms(10);

    hal_gpio_init(config->pin_bl);
    hal_gpio_set_dir(config->pin_bl, HAL_GPIO_OUT);
    hal_gpio_write(config->pin_bl, 1);

    return &st7789_instance;
}

void st7789_set_window(st7789_t *ctx, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    const st7789_config_t *cfg = ctx->config;

    uint8_t col[] = {
        (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF),
        (uint8_t)(x2 >> 8), (uint8_t)(x2 & 0xFF)
    };
    send_cmd_params(cfg, ST7789_CASET, col, 4);

    uint8_t row[] = {
        (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF),
        (uint8_t)(y2 >> 8), (uint8_t)(y2 & 0xFF)
    };
    send_cmd_params(cfg, ST7789_RASET, row, 4);

    write_cmd(cfg, ST7789_RAMWR);
}

void st7789_send_pixels(st7789_t *ctx, const uint8_t *data, size_t len) {
    write_data(ctx->config, data, len);
}
