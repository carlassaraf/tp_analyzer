#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

/**
 * GPIO Direction constants
 */
#define HAL_GPIO_IN  0
#define HAL_GPIO_OUT 1

/**
 * Initialize a GPIO pin
 * @param pin GPIO pin number
 */
void hal_gpio_init(uint8_t pin);

/**
 * Set GPIO direction (input/output)
 * @param pin GPIO pin number
 * @param direction HAL_GPIO_IN or HAL_GPIO_OUT
 */
void hal_gpio_set_dir(uint8_t pin, uint8_t direction);

/**
 * Write GPIO pin (0 or 1)
 * @param pin GPIO pin number
 * @param value 0 or 1
 */
void hal_gpio_write(uint8_t pin, uint8_t value);

/**
 * Read GPIO pin value
 * @param pin GPIO pin number
 * @return GPIO pin value (0 or 1)
 */
uint8_t hal_gpio_read(uint8_t pin);

/**
 * Set GPIO function (e.g., SPI, UART, PWM)
 * @param pin GPIO pin number
 * @param function GPIO function (e.g., GPIO_FUNC_SPI)
 */
void hal_gpio_set_function(uint8_t pin, uint8_t function);

#endif
