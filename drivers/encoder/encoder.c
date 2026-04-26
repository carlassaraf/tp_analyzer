#include "encoder/encoder.h"
#include "hal/hal_gpio.h"
#include <hardware/sync.h>

static uint8_t enc_pin_a;
static uint8_t enc_pin_b;
static uint8_t enc_pin_btn;

static volatile int32_t enc_delta = 0;

// On any edge of A: if A == B the encoder moved CCW, otherwise CW.
// Swap the +1/-1 here if rotation direction feels backwards.
static void enc_a_irq(uint8_t pin, uint32_t events) {
  (void)pin;
  (void)events;
  uint8_t a = hal_gpio_read(enc_pin_a);
  uint8_t b = hal_gpio_read(enc_pin_b);
  enc_delta += (a == b) ? -1 : 1;
}

void encoder_init(const encoder_config_t *config) {
  enc_pin_a   = config->pin_a;
  enc_pin_b   = config->pin_b;
  enc_pin_btn = config->pin_btn;

  hal_gpio_init(enc_pin_a);
  hal_gpio_set_dir(enc_pin_a, HAL_GPIO_IN);
  hal_gpio_set_pull(enc_pin_a, true, false);

  hal_gpio_init(enc_pin_b);
  hal_gpio_set_dir(enc_pin_b, HAL_GPIO_IN);
  hal_gpio_set_pull(enc_pin_b, true, false);

  hal_gpio_init(enc_pin_btn);
  hal_gpio_set_dir(enc_pin_btn, HAL_GPIO_IN);
  hal_gpio_set_pull(enc_pin_btn, true, false);

  hal_gpio_set_irq(enc_pin_a,
                    HAL_GPIO_IRQ_EDGE_FALL | HAL_GPIO_IRQ_EDGE_RISE,
                    true,
                    enc_a_irq);
}

int32_t encoder_pop_delta(void) {
  uint32_t saved = save_and_disable_interrupts();
  int32_t d = enc_delta;
  enc_delta = 0;
  restore_interrupts(saved);
  return d;
}

// Button is active-LOW (internal pull-up). Returns true while pressed.
bool encoder_btn_down(void) {
  return hal_gpio_read(enc_pin_btn) == 0;
}
