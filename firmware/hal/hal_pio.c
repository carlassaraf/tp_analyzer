#include "hal_pio.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hal_8080_write.pio.h"

struct hal_pio_t {
    PIO     pio;
    uint    sm;
    uint    prog_offset;
    uint8_t data_base;
    uint8_t pin_wr;
};

static hal_pio_t hal_pio_instance;

hal_pio_t *hal_pio_init_8080_write(uint8_t data_base, uint8_t pin_wr, float clk_mhz) {
    hal_pio_t *ctx = &hal_pio_instance;
    ctx->data_base = data_base;
    ctx->pin_wr    = pin_wr;

    PIO  pio = pio0;
    uint sm  = pio_claim_unused_sm(pio, true);
    uint off = pio_add_program(pio, &hal_8080_write_program);

    hal_8080_write_program_init(pio, sm, off, data_base, pin_wr, clk_mhz);

    ctx->pio         = pio;
    ctx->sm          = sm;
    ctx->prog_offset = off;

    return ctx;
}

void hal_pio_release_gpio(hal_pio_t *ctx) {
    pio_sm_set_enabled(ctx->pio, ctx->sm, false);
    gpio_set_function(ctx->pin_wr, GPIO_FUNC_SIO);
    gpio_put(ctx->pin_wr, 1);
    for (int i = 0; i < 8; i++) {
        gpio_set_function(ctx->data_base + i, GPIO_FUNC_SIO);
    }
}

void hal_pio_claim_gpio(hal_pio_t *ctx) {
    for (int i = 0; i < 8; i++) {
        gpio_set_function(ctx->data_base + i, GPIO_FUNC_PIO0);
    }
    gpio_set_function(ctx->pin_wr, GPIO_FUNC_PIO0);
    pio_sm_restart(ctx->pio, ctx->sm);
    pio_sm_clear_fifos(ctx->pio, ctx->sm);
    pio_sm_exec(ctx->pio, ctx->sm, pio_encode_jmp(ctx->prog_offset));
    pio_sm_set_enabled(ctx->pio, ctx->sm, true);
}

uint32_t hal_pio_get_tx_dreq(const hal_pio_t *ctx) {
    return (uint32_t)pio_get_dreq(ctx->pio, ctx->sm, true);
}

volatile void *hal_pio_get_tx_fifo_addr(const hal_pio_t *ctx) {
    return &ctx->pio->txf[ctx->sm];
}
