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

/**
 * @brief Updates chart from a float array, scaling each value by `scale`
 * @param chart   Pointer to chart object
 * @param points  Float magnitudes (expected range 0..1)
 * @param count   Number of points (max 1024)
 * @param scale   Multiplier to map into chart Y range
 */
void ui_chart_push_float_data(lv_obj_t *chart, const float *points, uint16_t count, float scale);

#endif