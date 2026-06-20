#include "hal_dma.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

static const enum dma_channel_transfer_size dma_size_map[] = {
    [HAL_DMA_SIZE_8]  = DMA_SIZE_8,
    [HAL_DMA_SIZE_16] = DMA_SIZE_16,
    [HAL_DMA_SIZE_32] = DMA_SIZE_32,
};

struct hal_dma_t {
    int chan;
};

static hal_dma_t hal_dma_instance;

hal_dma_t *hal_dma_init_pio_tx(volatile void *tx_fifo, uint32_t dreq, hal_dma_size_t size) {
    hal_dma_t *ctx = &hal_dma_instance;

    int chan = dma_claim_unused_channel(true);
    ctx->chan = chan;

    dma_channel_config cfg = dma_channel_get_default_config((uint)chan);
    channel_config_set_transfer_data_size(&cfg, dma_size_map[size]);
    channel_config_set_dreq(&cfg, (uint)dreq);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);
    dma_channel_configure((uint)chan, &cfg,
                          tx_fifo,  // fixed write address
                          NULL,     // read address set per transfer
                          0,        // count set per transfer
                          false);

    return ctx;
}

void hal_dma_start(hal_dma_t *ctx, const void *src, size_t count) {
    dma_channel_set_read_addr((uint)ctx->chan, src, false);
    dma_channel_set_trans_count((uint)ctx->chan, (uint32_t)count, true);
}

void hal_dma_wait(hal_dma_t *ctx) {
    dma_channel_wait_for_finish_blocking((uint)ctx->chan);
}

void hal_dma_set_completion_irq(hal_dma_t *ctx, void (*handler)(void)) {
    if (handler) {
        irq_set_exclusive_handler(DMA_IRQ_1, handler);
        dma_channel_set_irq1_enabled((uint)ctx->chan, true);
        irq_set_enabled(DMA_IRQ_1, true);
    } else {
        irq_set_enabled(DMA_IRQ_1, false);
        dma_channel_set_irq1_enabled((uint)ctx->chan, false);
    }
}

void hal_dma_irq_clear(hal_dma_t *ctx) {
    dma_hw->ints1 = 1u << (uint)ctx->chan;
}
