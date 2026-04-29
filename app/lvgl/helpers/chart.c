#include "chart.h"

void ui_chart_push_data(lv_obj_t *chart, const uint16_t *points, uint16_t count) {
  if (chart == NULL) { return; }
  lv_chart_series_t *ser = lv_chart_get_series_next(chart, NULL);
  for (uint16_t i = 0; i < count; i++) {
    // Map 12-bit ADC (0-4095) to chart Y range (-250..250). Adjust per hardware scaling.
    int32_t v = (int32_t)points[i] * 500 / 4095 - 250;
    lv_chart_set_next_value(chart, ser, v);
  }
}