#include "scr_plot.h"
#include "lvgl.h"
#include "ui/screens.h"
#include "ui/helpers/chart.h"

lv_obj_t *scr_plot = NULL;
lv_obj_t *scr_plot_chart = NULL;
lv_obj_t *scr_plot_y_scale = NULL;

void scr_plot_update_chart(const uint16_t *points, uint16_t count) {
  ui_chart_push_data(scr_plot_chart, points, count);
}

void scr_plot_init(void) {
  // Screen creation
  scr_plot = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_plot, lv_color_black(), LV_PART_MAIN);

  scr_plot_chart = lv_chart_create(scr_plot);
  lv_obj_set_style_size(scr_plot_chart, 0, 0, LV_PART_INDICATOR);
  lv_obj_set_size(scr_plot_chart, 420, 260);
  lv_obj_align(scr_plot_chart, LV_ALIGN_CENTER, 20, 0);
  lv_obj_set_style_border_color(scr_plot_chart, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_border_width(scr_plot_chart, 2, LV_PART_MAIN);
  lv_chart_set_update_mode(scr_plot_chart, LV_CHART_UPDATE_MODE_CIRCULAR);
  lv_chart_set_point_count(scr_plot_chart, 256);
  lv_chart_set_axis_range(scr_plot_chart, LV_CHART_AXIS_PRIMARY_Y, -250, 250);
  lv_chart_set_div_line_count(scr_plot_chart, 3, 7);  // 3 lines → 4 sections, one per 125 V interval
  lv_chart_series_t *series = lv_chart_add_series(scr_plot_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  uint32_t i;
  for(i = 0; i < 256; i++) {
      lv_chart_set_next_value(scr_plot_chart, series, ((int32_t)lv_rand(0, 440)) - 220);
  }

  // Scale must be a sibling of the chart (child of the screen), not a child of the chart.
  // 9 ticks over [-250, 250]: step = 500/8 = 62.5 V; major every 2 → labels at -250,-125,0,125,250.
  scr_plot_y_scale = lv_scale_create(scr_plot);
  lv_obj_set_size(scr_plot_y_scale, 45, 260);
  lv_obj_align_to(scr_plot_y_scale, scr_plot_chart, LV_ALIGN_OUT_LEFT_MID, -3, 0);
  lv_obj_set_style_length(scr_plot_y_scale, 5, LV_PART_ITEMS);
  lv_obj_set_style_length(scr_plot_y_scale, 10, LV_PART_INDICATOR);
  lv_scale_set_mode(scr_plot_y_scale, LV_SCALE_MODE_VERTICAL_LEFT);
  lv_scale_set_label_show(scr_plot_y_scale, true);
  lv_scale_set_total_tick_count(scr_plot_y_scale, 9);
  lv_scale_set_major_tick_every(scr_plot_y_scale, 2);
  lv_scale_set_range(scr_plot_y_scale, -250, 250);

  lv_screen_load_anim(scr_plot, LV_SCR_LOAD_ANIM_FADE_IN, 300, 0, true);
}

void scr_plot_deinit(void) {
  scr_plot = NULL;
  scr_plot_chart = NULL;
}

void scr_plot_step(void) {
}