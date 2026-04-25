#include <hardware/gpio.h>
#include "hal_gpio.h"

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
