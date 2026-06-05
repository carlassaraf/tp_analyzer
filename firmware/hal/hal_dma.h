#ifndef HAL_DMA_H
#define HAL_DMA_H

#include <stdint.h>
#include <stddef.h>

typedef struct hal_dma_t hal_dma_t;

typedef enum {
    HAL_DMA_SIZE_8  = 0,
    HAL_DMA_SIZE_16 = 1,
    HAL_DMA_SIZE_32 = 2,
} hal_dma_size_t;

// Claim a DMA channel configured for streaming to a peripheral TX FIFO.
// dreq:    data request signal (from hal_pio_get_tx_dreq).
// tx_fifo: fixed write address   (from hal_pio_get_tx_fifo_addr).
// size:    per-element transfer width.
hal_dma_t *hal_dma_init_pio_tx(volatile void *tx_fifo, uint32_t dreq, hal_dma_size_t size);

// Start a DMA transfer (non-blocking).
void hal_dma_start(hal_dma_t *ctx, const void *src, size_t count);

// Block until the current transfer completes.
void hal_dma_wait(hal_dma_t *ctx);

#endif
