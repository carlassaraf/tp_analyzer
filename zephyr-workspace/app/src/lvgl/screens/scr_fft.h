#ifndef SCR_FFT_H_
#define SCR_FFT_H_

#include <stdint.h>

void scr_fft_prepare(void);
void scr_fft_init(void);
void scr_fft_deinit(void);
void scr_fft_step(void);

// Update functions

/**
 * @brief Updates FFT chart
 * @param magnitudes Float magnitude array (normalized 0..1)
 * @param count Number of data points
 */
void scr_fft_update_chart(const float *magnitudes, uint16_t count, float freq_resolution);
/**
 * @brief Updates the peak field in the FFT data brief
 * @param raw_peak Raw peak value (not scaled)
 */
void scr_fft_update_peak(float raw_peak);
/**
 * @brief Updates the RMS field in the FFT data brief
 * @param raw_rms Raw RMS value (not scaled)
 */
void scr_fft_update_rms(float raw_rms);
/**
 * @brief Updates the frequency field in the FFT data brief
 * @param frequency Frequency value
 */
void scr_fft_update_frequency(float frequency);
/**
 * @brief Updates the THD field in the FFT data brief
 * @param thd Total harmonic distortion percentage
 */
void scr_fft_update_thd(float thd);

/**
 * @brief Returns the ADC channel number (0=voltage_a, 1=current_a, per the
 * board overlay) matching the signal currently selected on this screen.
 * screen_update.c uses this to drop adc_stream_block updates for the
 * channel the user isn't looking at.
 */
uint8_t scr_fft_get_active_channel(void);

#endif
