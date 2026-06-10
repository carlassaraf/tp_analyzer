#include "ui.h"
#include "scr_fft.h"
#include "lvgl.h"
#include "lvgl/helpers/chart.h"
#include "lvgl/screens.h"
#include "drivers/encoder/encoder.h"
#include "dsp.h"

#define FFT_CHART_Y_MAX  250.0f
#define FFT_MAX_OFFSET   ((int16_t)(N / 2 - VISIBLE_BINS))

static float32_t s_fft_bins[N / 2];
static int16_t   s_scroll_offset = 0;

// FS/N = 10240/2048 = 5 Hz per bin exactly
#define HZ_PER_BIN  (uint32_t)(FS / N)

static void scr_fft_redraw(void) {
  ui_chart_push_float_data(ui_scrFFT_chartFFT,
                           &s_fft_bins[s_scroll_offset],
                           VISIBLE_BINS, FFT_CHART_Y_MAX);
  lv_chart_refresh(ui_scrFFT_chartFFT);

  int32_t f_start = (int32_t)s_scroll_offset * HZ_PER_BIN;
  lv_scale_set_range(ui_scrFFT_chartFFT_Xaxis, f_start, f_start + 500);
}

void scr_fft_update_chart(const float *magnitudes, uint16_t count) {
  uint16_t bins = count < (N / 2) ? count : (N / 2);
  for (uint16_t i = 0; i < bins; i++) {
    s_fft_bins[i] = magnitudes[i];
  }
}

void scr_fft_prepare(void) {
  ui_scrFFT_screen_init();
  lv_chart_add_series(ui_scrFFT_chartFFT,
                      lv_palette_main(LV_PALETTE_RED),
                      LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_point_count(ui_scrFFT_chartFFT, VISIBLE_BINS);
  // X axis: 51 ticks over 500 Hz → 10 Hz minor, 50 Hz major (every 5th tick)
  lv_scale_set_total_tick_count(ui_scrFFT_chartFFT_Xaxis, 51);
  lv_scale_set_major_tick_every(ui_scrFFT_chartFFT_Xaxis, 5);
  lv_scale_set_range(ui_scrFFT_chartFFT_Xaxis, 0, 500);
}

void scr_fft_init(void) {
  s_scroll_offset = 0;
}

void scr_fft_deinit(void) {
}

void scr_fft_step(void) {
  // Periodically get encoder turns
  s_scroll_offset += encoder_pop_delta() * SCROLL_BINS;
  if (s_scroll_offset > FFT_MAX_OFFSET) {
    s_scroll_offset = FFT_MAX_OFFSET;
  } else if(s_scroll_offset < 0) { 
    s_scroll_offset = 0;
  }
  scr_fft_redraw();
}
