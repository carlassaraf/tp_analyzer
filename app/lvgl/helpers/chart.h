#ifndef HELPERS_CHART_H
#define HELPERS_CHART_H

#include "lvgl.h"

/** 
 * @brief Called to update chart values
 * @param chart Pointer to chart to update
 * @param points Data points in Y axis
 * @param count Number of data points
 */
void ui_chart_push_data(lv_obj_t *chart, const uint16_t *points, uint16_t count);

#endif