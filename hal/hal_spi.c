#include <hardware/spi.h>
#include <hardware/gpio.h>
#include "hal_spi.h"
#include "hal_gpio.h"

// Helper to get spi_inst_t pointer from spi_num
static spi_inst_t* get_spi_instance(uint8_t spi_num) {
  return (spi_num == 0) ? spi0 : spi1;
}

void hal_spi_init(const hal_spi_config_t *config) {
  spi_inst_t *spi = get_spi_instance(config->spi_num);
  
  spi_init(spi, config->baudrate);

  hal_gpio_set_function(config->sck_pin, GPIO_FUNC_SPI);
  hal_gpio_set_function(config->mosi_pin, GPIO_FUNC_SPI);
  
  if (config->miso_pin != -1) {
    hal_gpio_set_function(config->miso_pin, GPIO_FUNC_SPI);
  }

  hal_gpio_init(config->cs_pin);
  hal_gpio_set_dir(config->cs_pin, HAL_GPIO_OUT);
  hal_gpio_write(config->cs_pin, 1);
}

void hal_spi_write(const hal_spi_config_t *config, const uint8_t *data, size_t len) {
  spi_inst_t *spi = get_spi_instance(config->spi_num);
  
  hal_gpio_write(config->cs_pin, 0);
  spi_write_blocking(spi, data, len);
  hal_gpio_write(config->cs_pin, 1);
}