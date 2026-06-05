#include "ui.h"
#include "lvgl/screens.h"
#include "lvgl/screen_manager.h"
#include "lvgl/helpers/chart.h"
#include "hal/hal_adc.h"
#include "board_config.h"

void scr_plotter_update_chart(const uint16_t *points, uint16_t count) {
  ui_chart_push_data(ui_scrPlotter_chartView, points, count);
}

void scr_plotter_prepare(void) {
  ui_scrPlotter_screen_init();

  lv_chart_add_series(ui_scrPlotter_chartView,
                      lv_palette_main(LV_PALETTE_BLUE),
                      LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_point_count(ui_scrPlotter_chartView, HAL_ADC_BUFFER_SIZE);

}

void scr_plotter_init(void) {
  SCR_ADD_TO_GROUP(ui_scrPlotter_contTimeV);
}

void scr_plotter_deinit(void) {
  SCR_CLEAR_GROUP();
}

void scr_plotter_step(void) {
}