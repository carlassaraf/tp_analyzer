#include "ui.h"
#include "lvgl/screens.h"
#include "lvgl/screen_manager.h"
#include "lvgl/helpers/chart.h"
#include "lvgl/helpers/animations.h"
#include "hal/hal_adc.h"
#include "board_config.h"

#define CHART_PIXEL_WIDTH     414
#define SIDE_MENU_X_HIDDEN    480
#define SIDE_MENU_X_VISIBLE   296
#define SIDE_MENU_TIMEOUT_MS  5000
#define SIDE_MENU_ANIM_MS     250
#define ENCODER_ACTIVE_THRESH 50   // ms: inactive_time below this → encoder just moved
#define INIT_GRACE_MS         200  // ignore encoder for 200ms after entering screen

static lv_chart_series_t *oscilloscope_series = NULL;
static lv_timer_t *hide_timer   = NULL;
static bool        menu_visible = false;
static uint32_t    init_tick;

static void side_menu_show(void);
static void side_menu_hide(void);
static void hide_timer_cb(lv_timer_t *timer);
static void change_signal_cb(lv_event_t *event);

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
  oscilloscope_series = lv_chart_add_series(ui_scrOscilloscope_chartView,
                      lv_color_hex(_ui_theme_color_Voltage[0]),
                      LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_point_count(ui_scrOscilloscope_chartView, CHART_PIXEL_WIDTH);
  ui_chart_bind_ext_array(ui_scrOscilloscope_chartView, CHART_PIXEL_WIDTH);

  // Propagate focus event to inside objects
  lv_obj_add_flag(ui_scrOscilloscope_cntScaleV, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntScaleH, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntBack, LV_OBJ_FLAG_EVENT_TRICKLE);
  // Add callback event to change signal shown
  lv_obj_add_event_cb(ui_scrOscilloscope_cntSigVoltage, change_signal_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrOscilloscope_cntSigCorriente, change_signal_cb, LV_EVENT_CLICKED, NULL);
}

void scr_oscilloscope_init(void) {
  // Position the side menu off-screen and remove HIDDEN so animations work.
  // x=480 puts the left edge exactly at the screen's right boundary (menu width=184).
  lv_obj_set_x(ui_scrOscilloscope_cntSideMenu, SIDE_MENU_X_HIDDEN);
  lv_obj_remove_flag(ui_scrOscilloscope_cntSideMenu, LV_OBJ_FLAG_HIDDEN);
  menu_visible = false;
  init_tick    = lv_tick_get();
}

void scr_oscilloscope_deinit(void) {
  SCR_CLEAR_GROUP();
  if (hide_timer != NULL) {
    lv_timer_delete(hide_timer);
    hide_timer = NULL;
  }
  lv_obj_add_flag(ui_scrOscilloscope_cntSideMenu, LV_OBJ_FLAG_HIDDEN);
  menu_visible = false;
}

void scr_oscilloscope_step(void) {
  // Skip the first 200ms to avoid the encoder button press used to navigate here
  // from immediately triggering the side menu.
  if (lv_tick_elaps(init_tick) < INIT_GRACE_MS) return;

  // lv_task_handler() updates last_activity_time whenever enc_diff != 0,
  // so a low inactive time here means the encoder was rotated this frame.
  bool encoder_active = lv_display_get_inactive_time(lv_display_get_default()) < ENCODER_ACTIVE_THRESH;

  if (!encoder_active) return;

  if (!menu_visible) {
    side_menu_show();
  } else if (hide_timer != NULL) {
    lv_timer_reset(hide_timer);
  }
}

static void side_menu_show(void) {
  animation_move_to_left(ui_scrOscilloscope_cntSideMenu,
                         SIDE_MENU_X_HIDDEN, SIDE_MENU_X_VISIBLE,
                         SIDE_MENU_ANIM_MS);
  hide_timer   = lv_timer_create(hide_timer_cb, SIDE_MENU_TIMEOUT_MS, NULL);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntSigVoltage);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntSigCorriente);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScaleV);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScaleH);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntBack);
  menu_visible = true;
}

static void side_menu_hide(void) {
  animation_move_to_left(ui_scrOscilloscope_cntSideMenu,
                         SIDE_MENU_X_VISIBLE, SIDE_MENU_X_HIDDEN,
                         SIDE_MENU_ANIM_MS);
  SCR_CLEAR_GROUP();
  menu_visible = false;
}

static void hide_timer_cb(lv_timer_t *timer) {
  lv_timer_delete(timer);
  hide_timer = NULL;
  side_menu_hide();
}

static void change_signal_cb(lv_event_t *event) {
  lv_obj_t *target = lv_event_get_target_obj(event);
  // Check what signal is shown
  if(target == ui_scrOscilloscope_cntSigVoltage) {
    // Set signal in menu
    lv_obj_remove_state(ui_scrOscilloscope_cntSigCorriente, LV_STATE_CHECKED);
    lv_obj_add_flag(ui_scrOscilloscope_cntCorriente, LV_OBJ_FLAG_HIDDEN);
    // Set signal in screen
    lv_obj_add_state(ui_scrOscilloscope_cntSigVoltage, LV_STATE_CHECKED);
    lv_obj_remove_flag(ui_scrOscilloscope_cntTension, LV_OBJ_FLAG_HIDDEN);
    // Update units
    lv_label_set_text(ui_scrOscilloscope_lblPeakUnit, "V");
    lv_label_set_text(ui_scrOscilloscope_lblRmsUnit, "V");
    // Update colors
    lv_obj_set_style_text_color(ui_scrOscilloscope_lblPeakValue, lv_color_hex(_ui_theme_color_Voltage[0]), LV_PART_MAIN);
    lv_chart_set_series_color(ui_scrOscilloscope_chartView, oscilloscope_series, lv_color_hex(_ui_theme_color_Voltage[0]));
    lv_obj_invalidate(ui_scrOscilloscope_chartView);
  } else if(target == ui_scrOscilloscope_cntSigCorriente) {
    lv_obj_remove_state(ui_scrOscilloscope_cntSigVoltage, LV_STATE_CHECKED);
    lv_obj_add_flag(ui_scrOscilloscope_cntTension, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_state(ui_scrOscilloscope_cntSigCorriente, LV_STATE_CHECKED);
    lv_obj_remove_flag(ui_scrOscilloscope_cntCorriente, LV_OBJ_FLAG_HIDDEN);
    // Update units
    lv_label_set_text(ui_scrOscilloscope_lblPeakUnit, "A");
    lv_label_set_text(ui_scrOscilloscope_lblRmsUnit, "A");
    // Update colors
    lv_obj_set_style_text_color(ui_scrOscilloscope_lblPeakValue, lv_color_hex(_ui_theme_color_Corriente[0]), LV_PART_MAIN);
    lv_chart_set_series_color(ui_scrOscilloscope_chartView, oscilloscope_series, lv_color_hex(_ui_theme_color_Corriente[0]));
    lv_obj_invalidate(ui_scrOscilloscope_chartView);
  }
}