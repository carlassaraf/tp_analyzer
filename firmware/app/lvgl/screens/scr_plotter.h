#ifndef SCR_PLOT_H
#define SCR_PLOT_H

#include <stdint.h>

void scr_plotter_init(void);
void scr_plotter_deinit(void);
void scr_plotter_step(void);

// Update functions

/**
 * @brief Updates plot chart
 * @param points Data points to plot
 * @param count Number of data points
 */
void scr_plotter_update_chart(const uint16_t *points, uint16_t count);

#endif