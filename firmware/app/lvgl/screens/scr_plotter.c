#include "ui.h"
#include "lvgl/screen_manager.h"
#include "lvgl/helpers/chart.h"

void scr_plotter_update_chart(const uint16_t *points, uint16_t count) {
  ui_chart_push_data(ui_scrPlotter_chartView, points, count);
}

void scr_plotter_init(void) {
  _ui_screen_change(&ui_scrPlotter, LV_SCR_LOAD_ANIM_NONE, 0, 0, &ui_scrPlotter_screen_init);
}

void scr_plotter_deinit(void) {
}

void scr_plotter_step(void) {
}