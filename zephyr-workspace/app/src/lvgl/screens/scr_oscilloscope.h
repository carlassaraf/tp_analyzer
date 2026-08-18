#ifndef SCR_OSCILLOSCOPE_H
#define SCR_OSCILLOSCOPE_H

#include <stdint.h>

void scr_oscilloscope_prepare(void);
void scr_oscilloscope_init(void);
void scr_oscilloscope_deinit(void);
void scr_oscilloscope_step(void);

// Update functions

/**
 * @brief Updates oscilloscope chart
 * @param points Data points to plot
 * @param count Number of data points
 */
void scr_oscilloscope_update_chart(const uint16_t *points, uint16_t count);
/**
 * @brief Updates the peak field in the oscilloscope data brief
 * @param raw_peak Raw peak value (not scaled)
 */
void scr_oscilloscope_update_peak(float raw_peak);
/**
 * @brief Updates the RMS field in the oscilloscope data brief
 * @param raw_rms Raw RMS value (not scaled)
 */
void scr_oscilloscope_update_rms(float raw_rms);
/**
 * @brief Updates the frequency field in the oscilloscope data brief
 * @param frequency Frequency value
 */
void scr_oscilloscope_update_frequency(float frequency);

/**
 * @brief Returns the ADC channel number (0=voltage_a, 1=current_a, per the
 * board overlay) matching the signal currently selected on this screen.
 * screen_update.c uses this to drop adc_stream_block updates for the
 * channel the user isn't looking at.
 */
uint8_t scr_oscilloscope_get_active_channel(void);

#endif

