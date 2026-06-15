#include "ui.h"
#include "lvgl/screens.h"
#include "lvgl/screen_manager.h"
#include "lvgl/helpers/chart.h"
#include "hal/hal_adc.h"
#include "board_config.h"

#define CHART_PIXEL_WIDTH 414

void scr_oscilloscope_update_chart(const uint16_t *points, uint16_t count) {
  static uint16_t decimated[CHART_PIXEL_WIDTH];

  for (uint16_t px = 0; px < CHART_PIXEL_WIDTH; px++) {
    uint32_t start = (uint32_t)px * count / CHART_PIXEL_WIDTH;
    uint32_t end   = (uint32_t)(px + 1) * count / CHART_PIXEL_WIDTH;
    if (end > count) end = count;
    uint32_t sum = 0;
    for (uint32_t i = start; i < end; i++) sum += points[i];
    decimated[px] = (uint16_t)(sum / (end - start));
  }

  ui_chart_push_data(ui_scrOscilloscope_chartView, decimated, CHART_PIXEL_WIDTH);
}

void scr_oscilloscope_prepare(void) {
  lv_chart_add_series(ui_scrOscilloscope_chartView,
                      lv_palette_main(_ui_theme_color_Voltage[0]),
                      LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_point_count(ui_scrOscilloscope_chartView, CHART_PIXEL_WIDTH);

  // // The default LVGL theme applies a white background to lv_obj_create() objects.
  // // These sub-panels use remove_style_all() + border-only styling, so the theme's
  // // white bg bleeds through and renders as a solid horizontal bar over cntValues.
  // lv_obj_set_style_bg_opa(ui_scrOscilloscope_cntPeak,      LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(ui_scrOscilloscope_cntRMS,       LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(ui_scrOscilloscope_cntFrequency, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(ui_scrOscilloscope_cntTimeDiv,   LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void scr_oscilloscope_init(void) {
}

void scr_oscilloscope_deinit(void) {
  SCR_CLEAR_GROUP();
}

void scr_oscilloscope_step(void) {
  static uint32_t i = 0;
  lv_label_set_text_fmt(ui_scrOscilloscope_lblPeakValue, "%d", i++);
}