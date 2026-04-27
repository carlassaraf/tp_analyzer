#ifndef SCR_PLOT_H
#define SCR_PLOT_H

#include <stdint.h>

void scr_plot_init(void);
void scr_plot_deinit(void);
void scr_plot_step(void);

// Update functions

/**
 * @brief Updates plot chart
 * @param points Data points to plot
 * @param count Number of data points
 */
void scr_plot_update_chart(const uint16_t *points, uint16_t count);

#endif