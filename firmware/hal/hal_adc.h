#ifndef HAL_ADC_H
#define HAL_ADC_H

#include <stdint.h>

// Samples per DMA transfer — must equal N in dsp.h so one transfer fills one FFT window
#define HAL_ADC_BUFFER_SIZE 2048

// Type of expected callback for DMA
typedef void (*hal_adc_ready_cb_t)(const uint16_t *buf);

/**
 * @brief Handles ADC initialization
 * @param sample_rate Sampling frequency in Hz
 * @param gpio_pin Number of GPIO to use
 * @param channel Number of ADC channel
 */
void hal_adc_init(uint32_t sample_rate, uint8_t gpio_pin, uint8_t channel);
/**
 * @brief Registers a callback for the DMA ISR
 * @param cb Sampling ready callback function
 */
void hal_adc_set_ready_cb(hal_adc_ready_cb_t cb);
/** @brief Starts ADC in free running mode with DMA */
void hal_adc_start(void);
/** @brief Stops ADC and clears FIFO and DMA */
void hal_adc_stop(void);

#endif
