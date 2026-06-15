#include "ui.h"
#include "lvgl/screens.h"
#include "lvgl/screen_manager.h"
#include "lvgl/helpers/chart.h"
#include "lvgl/helpers/animations.h"
#include "hal/hal_adc.h"
#include "board_config.h"
#include "encoder/encoder.h"

#define CHART_PIXEL_WIDTH 414

static lv_timer_t *scr_oscilloscope_activity_timer = NULL;
static bool encoder_activity = false;

static void scr_oscilloscope_encoder_timeout_cb(lv_timer_t *timer);

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
                      lv_palette_main(UI_THEME_COLOR_VOLTAGE),
                      LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_point_count(ui_scrOscilloscope_chartView, CHART_PIXEL_WIDTH);
  ui_chart_bind_ext_array(ui_scrOscilloscope_chartView, CHART_PIXEL_WIDTH);

  // Propagate focus event to inside objects
  lv_obj_add_flag(ui_scrOscilloscope_iconScaleV, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_lblScaleV, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_lblScaleVValue, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_iconScaleVEnter, LV_OBJ_FLAG_EVENT_TRICKLE);

  lv_obj_add_flag(ui_scrOscilloscope_iconScaleH, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_lblScaleH, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_lblScaleHValue, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_iconScaleHEnter, LV_OBJ_FLAG_EVENT_TRICKLE);

  lv_obj_add_flag(ui_scrOscilloscope_iconBack, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_lblBack, LV_OBJ_FLAG_EVENT_TRICKLE);
}

void scr_oscilloscope_init(void) {
}

void scr_oscilloscope_deinit(void) {
  SCR_CLEAR_GROUP();
}

void scr_oscilloscope_step(void) {
  // Check activity
  if(encoder_pop_delta() && !encoder_activity) {
    scr_oscilloscope_activity_timer = lv_timer_create(scr_oscilloscope_encoder_timeout_cb, 5000, NULL);
    animation_move_to_left(ui_scrOscilloscope_cntSideMenu, 480, 196, 250);
    encoder_activity = true;
  } else if(encoder_pop_delta()) {
    // Relauch timer
    lv_timer_reset(scr_oscilloscope_activity_timer);
  }
}

static void scr_oscilloscope_encoder_timeout_cb(lv_timer_t *timer) {
  encoder_activity = false;
}