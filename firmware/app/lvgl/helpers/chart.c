#include "chart.h"
#include "hal/hal_adc.h"

#define FFT_OUT_MAX 1024

static int32_t s_scaled[HAL_ADC_BUFFER_SIZE];
static int32_t s_scaled_fft[FFT_OUT_MAX];

void ui_chart_push_data(lv_obj_t *chart, const uint16_t *points, uint16_t count) {
  if (chart == NULL) return;
  lv_chart_series_t *ser = lv_chart_get_series_next(chart, NULL);
  if (ser == NULL) return;
  for (uint16_t i = 0; i < count; i++) {
    // Map 12-bit ADC (0-4095) to chart Y range (-250..250). Adjust per hardware scaling.
    s_scaled[i] = (int32_t)points[i] * 500 / 4095 - 250;
  }
  lv_chart_set_series_ext_y_array(chart, ser, s_scaled);
}

void ui_chart_push_float_data(lv_obj_t *chart, const float *points, uint16_t count, float scale) {
  if (chart == NULL) return;
  lv_chart_series_t *ser = lv_chart_get_series_next(chart, NULL);
  if (ser == NULL) return;
  if (count > FFT_OUT_MAX) count = FFT_OUT_MAX;
  for (uint16_t i = 0; i < count; i++) {
    s_scaled_fft[i] = (int32_t)(points[i] * scale);
  }
  lv_chart_set_series_ext_y_array(chart, ser, s_scaled_fft);
}