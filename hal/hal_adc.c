#include "hal_adc.h"
#include <hardware/adc.h>
#include <hardware/dma.h>
#include <hardware/irq.h>

// Ping pong buffers
static uint16_t s_buf_a[HAL_ADC_BUFFER_SIZE];
static uint16_t s_buf_b[HAL_ADC_BUFFER_SIZE];

static int s_dma_ch;
static dma_channel_config s_dma_cfg;
static hal_adc_ready_cb_t s_ready_cb;

// HAL DMA ISR
static void dma_isr(void);

void hal_adc_init(uint32_t sample_rate, uint8_t gpio_pin, uint8_t channel) {
  adc_init();
  adc_gpio_init(gpio_pin);
  adc_select_input(channel);
  adc_fifo_setup(true, true, 1, false, false);
  adc_set_clkdiv((float)48e6f / (float)sample_rate - 1.0f);

  s_dma_ch = dma_claim_unused_channel(true);
  s_dma_cfg = dma_channel_get_default_config(s_dma_ch);
  channel_config_set_transfer_data_size(&s_dma_cfg, DMA_SIZE_16);
  channel_config_set_read_increment(&s_dma_cfg, false);
  channel_config_set_write_increment(&s_dma_cfg, true);
  channel_config_set_dreq(&s_dma_cfg, DREQ_ADC);

  dma_channel_set_irq0_enabled(s_dma_ch, true);
  irq_set_exclusive_handler(DMA_IRQ_0, dma_isr);
  irq_set_priority(DMA_IRQ_0, PICO_LOWEST_IRQ_PRIORITY);
  irq_set_enabled(DMA_IRQ_0, true);
}

void hal_adc_set_ready_cb(hal_adc_ready_cb_t cb) {
  s_ready_cb = cb;
}

void hal_adc_start(void) {
  dma_channel_configure(s_dma_ch, &s_dma_cfg,
                        s_buf_a, &adc_hw->fifo,
                        HAL_ADC_BUFFER_SIZE, true);
  adc_run(true);
}

void hal_adc_stop(void) {
  adc_run(false);
  dma_channel_abort(s_dma_ch);
  adc_fifo_drain();
}

static void dma_isr(void) {
  static uint16_t *s_active = s_buf_a;

  uint16_t *completed = s_active;
  s_active = (s_active == s_buf_a) ? s_buf_b : s_buf_a;

  dma_hw->ints0 = 1u << s_dma_ch;
  adc_fifo_drain();
  dma_channel_configure(s_dma_ch, &s_dma_cfg,
                        s_active, &adc_hw->fifo,
                        HAL_ADC_BUFFER_SIZE, true);

  if (s_ready_cb) {
    s_ready_cb(completed);
  }
}
