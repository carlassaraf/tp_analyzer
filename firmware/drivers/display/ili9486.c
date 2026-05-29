#include "drivers/display/ili9486.h"
#include "ili9486_write.pio.h"
#include "hal/hal_gpio.h"
#include <pico/stdlib.h>
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"

struct ili9486_t {
    const ili9486_config_t *config;
    uint32_t data_mask;
    PIO      pio;
    uint     sm;
    uint     pio_prog_offset;
    int      dma_chan;
};

#define ILI9486_SWRESET  0x01
#define ILI9486_SLPIN    0x10
#define ILI9486_SLPOUT   0x11
#define ILI9486_NORON    0x13
#define ILI9486_INVOFF   0x20
#define ILI9486_INVON    0x21
#define ILI9486_DISPON   0x29
#define ILI9486_CASET    0x2A
#define ILI9486_RASET    0x2B
#define ILI9486_RAMWR    0x2C
#define ILI9486_MADCTL   0x36
#define ILI9486_COLMOD   0x3A
#define ILI9486_PWCTRL1  0xC0
#define ILI9486_PWCTRL2  0xC1
#define ILI9486_PWCTRL3  0xC2
#define ILI9486_VMCTRL1  0xC5
#define ILI9486_DFUNCTR  0xB6
#define ILI9486_PGAMCTRL 0xE0
#define ILI9486_NGAMCTRL 0xE1

// MADCTL bits
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_BGR 0x08

static ili9486_t ili9486_instance;

// --- GPIO <-> PIO handoff ---------------------------------------------------

// Reclaim D0-D7 and WR from PIO so write_byte_raw() works (used by set_window).
static void gpio_claim_from_pio(const ili9486_t *ctx) {
    pio_sm_set_enabled(ctx->pio, ctx->sm, false);
    hal_gpio_set_function(ctx->config->pin_wr, GPIO_FUNC_SIO);
    hal_gpio_write(ctx->config->pin_wr, 1);
    for (int i = 0; i < 8; i++) {
        hal_gpio_set_function(ctx->config->data_pin_base + i, GPIO_FUNC_SIO);
    }
}

// Hand D0-D7 and WR back to PIO after command writes.
// Flushes any stale FIFO data and resets the SM to a clean state.
static void pio_reclaim_gpio(const ili9486_t *ctx) {
    for (int i = 0; i < 8; i++) {
        hal_gpio_set_function(ctx->config->data_pin_base + i, GPIO_FUNC_PIO0);
    }
    hal_gpio_set_function(ctx->config->pin_wr, GPIO_FUNC_PIO0);
    pio_sm_restart(ctx->pio, ctx->sm);
    pio_sm_clear_fifos(ctx->pio, ctx->sm);
    pio_sm_exec(ctx->pio, ctx->sm, pio_encode_jmp(ctx->pio_prog_offset));
    pio_sm_set_enabled(ctx->pio, ctx->sm, true);
}

// --- PIO + DMA init (called once, after the ILI9486 register init sequence) -

static void ili9486_pio_dma_init(ili9486_t *ctx) {
    PIO  pio = pio0;
    uint sm  = pio_claim_unused_sm(pio, true);
    uint off = pio_add_program(pio, &ili9486_write_program);

    ili9486_write_program_init(pio, sm, off,
                               ctx->config->data_pin_base,
                               ctx->config->pin_wr,
                               50.0f);

    ctx->pio             = pio;
    ctx->sm              = sm;
    ctx->pio_prog_offset = off;

    int dma_chan = dma_claim_unused_channel(true);
    ctx->dma_chan = dma_chan;

    dma_channel_config cfg = dma_channel_get_default_config((uint)dma_chan);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_dreq(&cfg, pio_get_dreq(pio, sm, true));
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);
    dma_channel_configure((uint)dma_chan, &cfg,
                          &pio->txf[sm],  // write addr fixed
                          NULL,           // read addr set per transfer
                          0,              // count set per transfer
                          false);
}

// ----------------------------------------------------------------------------

static inline void write_byte_raw(const ili9486_t *ctx, uint8_t byte) {
    hal_gpio_write_masked(ctx->data_mask, (uint32_t)byte << ctx->config->data_pin_base);
    hal_gpio_write(ctx->config->pin_wr, 0);
    hal_gpio_write(ctx->config->pin_wr, 1);
}

static void write_cmd(const ili9486_t *ctx, uint8_t cmd) {
    hal_gpio_write(ctx->config->pin_rs, 0);
    write_byte_raw(ctx, cmd);
}

static void write_data(const ili9486_t *ctx, const uint8_t *data, size_t len) {
    hal_gpio_write(ctx->config->pin_rs, 1);
    for (size_t i = 0; i < len; i++) {
        write_byte_raw(ctx, data[i]);
    }
}

static void send_cmd_params(const ili9486_t *ctx, uint8_t cmd,
                            const uint8_t *params, size_t count) {
    write_cmd(ctx, cmd);
    if (count > 0) {
        write_data(ctx, params, count);
    }
}

static void hw_reset(const ili9486_config_t *cfg) {
    hal_gpio_write(cfg->pin_rst, 1);
    sleep_ms(10);
    hal_gpio_write(cfg->pin_rst, 0);
    sleep_ms(20);
    hal_gpio_write(cfg->pin_rst, 1);
    sleep_ms(150);
}

static uint8_t rotation_to_madctl(uint8_t rotation) {
    switch (rotation) {
        case 0:  return MADCTL_BGR;
        case 1:  return MADCTL_MX | MADCTL_MV | MADCTL_BGR;
        case 2:  return MADCTL_MY | MADCTL_MX | MADCTL_BGR;
        case 3:  return MADCTL_MY | MADCTL_MX | MADCTL_MV | MADCTL_BGR;
        default: return MADCTL_MV | MADCTL_BGR;
    }
}

ili9486_t *ili9486_init(const ili9486_config_t *config) {
    ili9486_instance.config    = config;
    ili9486_instance.data_mask = (uint32_t)0xFF << config->data_pin_base;

    // Data bus (D0-D7)
    for (int i = 0; i < 8; i++) {
        hal_gpio_init((uint8_t)(config->data_pin_base + i));
        hal_gpio_set_dir((uint8_t)(config->data_pin_base + i), HAL_GPIO_OUT);
    }

    // Control pins
    hal_gpio_init(config->pin_wr);
    hal_gpio_set_dir(config->pin_wr, HAL_GPIO_OUT);
    hal_gpio_write(config->pin_wr, 1);  // idle high

    hal_gpio_init(config->pin_rd);
    hal_gpio_set_dir(config->pin_rd, HAL_GPIO_OUT);
    hal_gpio_write(config->pin_rd, 1);  // idle high (read not used)

    hal_gpio_init(config->pin_rs);
    hal_gpio_set_dir(config->pin_rs, HAL_GPIO_OUT);
    hal_gpio_write(config->pin_rs, 1);

    hal_gpio_init(config->pin_cs);
    hal_gpio_set_dir(config->pin_cs, HAL_GPIO_OUT);
    hal_gpio_write(config->pin_cs, 0);  // assert permanently (single device)

    hal_gpio_init(config->pin_rst);
    hal_gpio_set_dir(config->pin_rst, HAL_GPIO_OUT);

    if (config->pin_bl >= 0) {
        hal_gpio_init((uint8_t)config->pin_bl);
        hal_gpio_set_dir((uint8_t)config->pin_bl, HAL_GPIO_OUT);
        hal_gpio_write((uint8_t)config->pin_bl, 0);  // backlight off during init
    }

    hw_reset(config);

    ili9486_t *ctx = &ili9486_instance;

    send_cmd_params(ctx, ILI9486_SWRESET, NULL, 0);
    sleep_ms(120);

    // Power controls
    send_cmd_params(ctx, ILI9486_PWCTRL1, (uint8_t[]){0x0d, 0x0d}, 2);
    send_cmd_params(ctx, ILI9486_PWCTRL2, (uint8_t[]){0x43, 0x00}, 2);
    send_cmd_params(ctx, ILI9486_PWCTRL3, (uint8_t[]){0x00}, 1);

    // VCOM
    send_cmd_params(ctx, ILI9486_VMCTRL1, (uint8_t[]){0x00, 0x48}, 2);

    // Memory access & color format
    send_cmd_params(ctx, ILI9486_MADCTL, (uint8_t[]){rotation_to_madctl(config->rotation)}, 1);
    send_cmd_params(ctx, ILI9486_COLMOD, (uint8_t[]){0x55}, 1);  // 16-bit RGB565

    // Display function control
    send_cmd_params(ctx, ILI9486_DFUNCTR, (uint8_t[]){0x00, 0x42, 0x3B}, 3);

    // Positive gamma correction
    send_cmd_params(ctx, ILI9486_PGAMCTRL,
        (uint8_t[]){0x0F, 0x24, 0x1C, 0x0A, 0x0F, 0x08, 0x43, 0x88,
                    0x32, 0x0F, 0x10, 0x06, 0x0F, 0x07, 0x00}, 15);

    // Negative gamma correction
    send_cmd_params(ctx, ILI9486_NGAMCTRL,
        (uint8_t[]){0x0F, 0x38, 0x30, 0x09, 0x0F, 0x0F, 0x4E, 0x77,
                    0x3C, 0x07, 0x10, 0x05, 0x23, 0x1B, 0x00}, 15);

    send_cmd_params(ctx, ILI9486_INVOFF, NULL, 0);

    send_cmd_params(ctx, ILI9486_SLPOUT, NULL, 0);
    sleep_ms(150);

    send_cmd_params(ctx, ILI9486_DISPON, NULL, 0);
    sleep_ms(25);

    send_cmd_params(ctx, ILI9486_NORON, NULL, 0);

    if (config->pin_bl >= 0)
        hal_gpio_write((uint8_t)config->pin_bl, 1);  // backlight on

    // Hand D0-D7 and WR to PIO; set up DMA channel.
    ili9486_pio_dma_init(ctx);

    return ctx;
}

void ili9486_set_window(ili9486_t *ctx, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2) {
    // Reclaim D0-D7 and WR from PIO so write_byte_raw() can drive them.
    gpio_claim_from_pio(ctx);

    uint8_t col[] = {
        (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF),
        (uint8_t)(x2 >> 8), (uint8_t)(x2 & 0xFF)
    };
    send_cmd_params(ctx, ILI9486_CASET, col, 4);

    uint8_t row[] = {
        (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF),
        (uint8_t)(y2 >> 8), (uint8_t)(y2 & 0xFF)
    };
    send_cmd_params(ctx, ILI9486_RASET, row, 4);

    write_cmd(ctx, ILI9486_RAMWR);
    hal_gpio_write(ctx->config->pin_rs, 1);  // stay in data mode for pixel burst

    // Return D0-D7 and WR to PIO for the upcoming DMA pixel transfer.
    pio_reclaim_gpio(ctx);
}

void ili9486_send_pixels(ili9486_t *ctx, const uint8_t *data, size_t len) {
    // RS is already high (data mode) after ili9486_set_window
    const uint32_t mask  = ctx->data_mask;
    const uint8_t  shift = ctx->config->data_pin_base;
    const uint8_t  wr    = ctx->config->pin_wr;

    for (size_t i = 0; i < len; i++) {
        hal_gpio_write_masked(mask, (uint32_t)data[i] << shift);
        hal_gpio_write(wr, 0);
        hal_gpio_write(wr, 1);
    }
}

void ili9486_send_pixels_dma(ili9486_t *ctx, const uint8_t *data, size_t len) {
    // RS is already high (data mode) and PIO is enabled after ili9486_set_window.
    dma_channel_set_read_addr((uint)ctx->dma_chan, data, false);
    dma_channel_set_trans_count((uint)ctx->dma_chan, (uint32_t)len, true);
}

void ili9486_dma_wait(ili9486_t *ctx) {
    dma_channel_wait_for_finish_blocking((uint)ctx->dma_chan);
}
