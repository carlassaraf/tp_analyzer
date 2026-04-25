#ifndef HAL_SPI_H
#define HAL_SPI_H

#include <stdint.h>
#include <stddef.h>

// SPI configuration structure
typedef struct {
    uint8_t spi_num;            // SPI instance (0 or 1)
    uint32_t baudrate;          // SPI baudrate
    int8_t sck_pin;             // SCK pin number
    int8_t mosi_pin;            // MOSI pin number
    int8_t miso_pin;            // MISO pin number (-1 = not used)
    int8_t cs_pin;              // Chip select pin number
} hal_spi_config_t;

void hal_spi_init(const hal_spi_config_t *config);
void hal_spi_write(const hal_spi_config_t *config, const uint8_t *data, size_t length);

#endif