#ifndef HAL_PIO_H
#define HAL_PIO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct hal_pio_t hal_pio_t;

// Claim a PIO state machine and load the 8080 parallel-write program.
// data_base: first of 8 consecutive GPIO pins for the data bus.
// pin_wr:    WR strobe GPIO.
// clk_mhz:  desired PIO clock frequency in MHz.
hal_pio_t *hal_pio_init_8080_write(uint8_t data_base, uint8_t pin_wr, float clk_mhz);

// Disable the SM and reassign the data bus + WR GPIOs to SIO for bit-bang use.
void hal_pio_release_gpio(hal_pio_t *ctx);

// Reassign the data bus + WR GPIOs back to PIO, reset the SM, and re-enable it.
void hal_pio_claim_gpio(hal_pio_t *ctx);

// Returns the DREQ signal index for wiring a DMA channel to this SM's TX FIFO.
uint32_t hal_pio_get_tx_dreq(const hal_pio_t *ctx);

// Returns the TX FIFO address for use as a fixed DMA write destination.
volatile void *hal_pio_get_tx_fifo_addr(const hal_pio_t *ctx);

#endif
