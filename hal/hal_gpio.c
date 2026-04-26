#include <hardware/gpio.h>
#include "hal_gpio.h"

#define HAL_GPIO_MAX_PINS 30

static hal_gpio_irq_cb_t irq_callbacks[HAL_GPIO_MAX_PINS];

static void gpio_irq_dispatcher(uint gpio, uint32_t events) {
  if (gpio < HAL_GPIO_MAX_PINS && irq_callbacks[gpio]) {
    irq_callbacks[gpio]((uint8_t)gpio, events);
  }
}

void hal_gpio_init(uint8_t pin) {
    gpio_init(pin);
}

void hal_gpio_set_dir(uint8_t pin, uint8_t direction) {
    gpio_set_dir(pin, direction == HAL_GPIO_OUT ? GPIO_OUT : GPIO_IN);
}

void hal_gpio_write(uint8_t pin, uint8_t value) {
    gpio_put(pin, value ? 1 : 0);
}

uint8_t hal_gpio_read(uint8_t pin) {
    return gpio_get(pin) ? 1 : 0;
}

void hal_gpio_set_function(uint8_t pin, uint8_t function) {
    gpio_set_function(pin, function);
}

void hal_gpio_set_pull(uint8_t pin, bool pull_up, bool pull_down) {
  gpio_set_pulls(pin, pull_up, pull_down);
}

void hal_gpio_set_irq(uint8_t pin, uint32_t event_mask, bool enabled, hal_gpio_irq_cb_t cb) {
  if (pin < HAL_GPIO_MAX_PINS) {
    irq_callbacks[pin] = enabled ? cb : NULL;
  }
  gpio_set_irq_enabled_with_callback(pin, event_mask, enabled, gpio_irq_dispatcher);
}
