#ifndef SCR_OSCILLOSCOPE_H
#define SCR_OSCILLOSCOPE_H

#include <stdint.h>

void scr_oscilloscope_prepare(void);
void scr_oscilloscope_init(void);
void scr_oscilloscope_deinit(void);
void scr_oscilloscope_step(void);

// Update functions

/**
 * @brief Updates plot chart
 * @param points Data points to plot
 * @param count Number of data points
 */
void scr_oscilloscope_update_chart(const uint16_t *points, uint16_t count);

void scr_oscilloscope_update_peak(uint16_t raw_peak);
void scr_oscilloscope_update_rms(float rms);
void scr_oscilloscope_update_frequency(float frequency);

#endif