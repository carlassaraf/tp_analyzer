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
#define VSCALE_CNT            4

/** @brief Menu pages for this screen */
typedef enum {
  MENU_PAGE_MAIN,     /**< Default main manu */
  MENU_PAGE_SCALE_V,  /**< Vertical scale menu */
  MENU_PAGE_SCALE_H   /**< Horizontal scale menu */
} menu_page_t;

/** @brief Displayed signal */
typedef enum {
  SIGNAL_SP_VOLTAGE,  /**< Currently showing single phase voltage */
  SIGNAL_SP_CURRENT,  /**< Currently showing single phase current */
  SIGNAL_CNT,
} displayed_signal_t;

static lv_chart_series_t *s_oscilloscope_series = NULL;
static lv_timer_t *s_hide_timer   = NULL;
static bool        s_menu_visible = false;
static uint32_t    s_init_tick;
static menu_page_t s_menu_page = MENU_PAGE_MAIN;
static displayed_signal_t s_curr_signal = SIGNAL_SP_VOLTAGE;
static const uint32_t s_vscales[SIGNAL_CNT][VSCALE_CNT] = {
  {100, 250, 400, 800},  // SIGNAL_SP_VOLTAGE
  {5,   10,  20,  50},   // SIGNAL_SP_CURRENT
};
static const uint32_t s_sig_colors[SIGNAL_CNT] = { 0xFFD23F, 0x33D6EE };
static uint32_t s_curr_voltage_scale = 400;
static uint32_t s_curr_current_scale = 10;
static uint32_t s_curr_vscale = 400;

static void side_menu_show(void);
static void side_menu_hide(void);
static void hide_timer_cb(lv_timer_t *timer);
static void change_signal_cb(lv_event_t *event);
static void enter_scale_menu_cb(lv_event_t *event);
static void enter_time_menu_cb(lv_event_t *event);
static void menu_go_back(lv_event_t *event);
static void change_vertical_scale_cb(lv_event_t *event);
static void update_vscales(void);
static int32_t restore_selected_scale(uint32_t idx);

// Public functions

void scr_oscilloscope_update_chart(const uint16_t *points, uint16_t count)
{
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

// Life cycle functions

void scr_oscilloscope_prepare(void)
{
  s_oscilloscope_series = lv_chart_add_series(ui_scrOscilloscope_chartView,
                      lv_color_hex(_ui_theme_color_Voltage[0]),
                      LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_point_count(ui_scrOscilloscope_chartView, CHART_PIXEL_WIDTH);
  ui_chart_bind_ext_array(ui_scrOscilloscope_chartView, CHART_PIXEL_WIDTH);
  // Propagate focus event to inside objects
  lv_obj_add_flag(ui_scrOscilloscope_cntScaleV, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntScaleH, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntBack, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntScale1, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntScale2, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntScale3, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntScale4, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntTime1, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntTime2, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntTime3, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntTime4, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrOscilloscope_cntTime5, LV_OBJ_FLAG_EVENT_TRICKLE);
  // Add callback event to change signal shown
  lv_obj_add_event_cb(ui_scrOscilloscope_cntSigVoltage, change_signal_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrOscilloscope_cntSigCorriente, change_signal_cb, LV_EVENT_CLICKED, NULL);
  // Add callback event to menu entries
  lv_obj_add_event_cb(ui_scrOscilloscope_cntScaleV, enter_scale_menu_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrOscilloscope_cntScaleH, enter_time_menu_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrOscilloscope_btnScaleBack, menu_go_back, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrOscilloscope_btnTimeBack, menu_go_back, LV_EVENT_CLICKED, NULL);
  // Add scales check callback
  lv_obj_add_event_cb(ui_scrOscilloscope_cntScale1, change_vertical_scale_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrOscilloscope_cntScale2, change_vertical_scale_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrOscilloscope_cntScale3, change_vertical_scale_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrOscilloscope_cntScale4, change_vertical_scale_cb, LV_EVENT_CLICKED, NULL);
}

void scr_oscilloscope_init(void)
{
  // Position the side menu off-screen and remove HIDDEN so animations work.
  // x=480 puts the left edge exactly at the screen's right boundary (menu width=184).
  lv_obj_set_x(ui_scrOscilloscope_cntSideMenu, SIDE_MENU_X_HIDDEN);
  lv_obj_remove_flag(ui_scrOscilloscope_cntSideMenu, LV_OBJ_FLAG_HIDDEN);
  s_menu_visible = false;
  s_init_tick    = lv_tick_get();
  // Update scales accordingly
  lv_label_set_text_fmt(ui_scrOscilloscope_lblTop, "+%d", s_curr_vscale);
  lv_label_set_text_fmt(ui_scrOscilloscope_lblBot, "-%d", s_curr_vscale);
  lv_label_set_text_fmt(ui_scrOscilloscope_lblScaleVValue, "%d %s", s_curr_vscale, (s_curr_signal == SIGNAL_SP_VOLTAGE)? "V" : "A");
  update_vscales();
  for(uint32_t i = 0; i < VSCALE_CNT; i++) {
    if(s_vscales[s_curr_signal][i] == s_curr_vscale) {
      restore_selected_scale(i);
      break;
    }
  }
}

void scr_oscilloscope_deinit(void)
{
  SCR_CLEAR_GROUP();
  if (s_hide_timer != NULL) {
    lv_timer_delete(s_hide_timer);
    s_hide_timer = NULL;
  }
  lv_obj_add_flag(ui_scrOscilloscope_cntSideMenu, LV_OBJ_FLAG_HIDDEN);
  s_menu_visible = false;
}

void scr_oscilloscope_step(void)
{
  // Skip the first 200ms to avoid the encoder button press used to navigate here
  // from immediately triggering the side menu.
  if (lv_tick_elaps(s_init_tick) < INIT_GRACE_MS) return;

  // lv_task_handler() updates last_activity_time whenever enc_diff != 0,
  // so a low inactive time here means the encoder was rotated this frame.
  bool encoder_active = lv_display_get_inactive_time(lv_display_get_default()) < ENCODER_ACTIVE_THRESH;

  if (!encoder_active) return;

  if (!s_menu_visible) {
    side_menu_show();
  } else if (s_hide_timer != NULL) {
    lv_timer_reset(s_hide_timer);
  }
}

// Private functions

/** @brief Called to show the side menu. Animates to left and updates input group */
static void side_menu_show(void)
{
  animation_move_to_side(ui_scrOscilloscope_cntSideMenu,
                         SIDE_MENU_X_HIDDEN, SIDE_MENU_X_VISIBLE,
                         SIDE_MENU_ANIM_MS);
  s_hide_timer   = lv_timer_create(hide_timer_cb, SIDE_MENU_TIMEOUT_MS, NULL);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntSigVoltage);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntSigCorriente);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScaleV);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScaleH);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntBack);
  s_menu_visible = true;
}

/** @brief Called to hide the side menu. Animates to right, clears the menu and input group */
static void side_menu_hide(void) {
  animation_move_to_side(ui_scrOscilloscope_cntSideMenu,
                         SIDE_MENU_X_VISIBLE, SIDE_MENU_X_HIDDEN,
                         SIDE_MENU_ANIM_MS);
  SCR_CLEAR_GROUP();
  s_menu_visible = false;
  // Revert submenus to proper positions
  lv_obj_set_x(ui_scrOscilloscope_cntMainSideMenu, 0);
  lv_obj_set_x(ui_scrOscilloscope_cntScaleMenu, 184);
  lv_obj_set_x(ui_scrOscilloscope_cntTimeMenu, 184);
  // Restore menu pointer
  s_menu_page = MENU_PAGE_MAIN;
}

/** @brief Timer callback to clear menu */
static void hide_timer_cb(lv_timer_t *timer)
{
  lv_timer_delete(timer);
  s_hide_timer = NULL;
  side_menu_hide();
}

/** @brief Clicked callback to update signal displayed */
static void change_signal_cb(lv_event_t *event)
{
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
    lv_chart_set_series_color(ui_scrOscilloscope_chartView, s_oscilloscope_series, lv_color_hex(_ui_theme_color_Voltage[0]));
    lv_obj_invalidate(ui_scrOscilloscope_chartView);
    // Update signal variable
    s_curr_signal = SIGNAL_SP_VOLTAGE;
    s_curr_vscale = s_curr_voltage_scale;
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
    lv_chart_set_series_color(ui_scrOscilloscope_chartView, s_oscilloscope_series, lv_color_hex(_ui_theme_color_Corriente[0]));
    lv_obj_invalidate(ui_scrOscilloscope_chartView);
    // Update signal variable
    s_curr_signal = SIGNAL_SP_CURRENT;
    s_curr_vscale = s_curr_current_scale;
  }
  // Update scales with current signal
  lv_label_set_text_fmt(ui_scrOscilloscope_lblScaleVValue, "%d %s", s_curr_vscale, (s_curr_signal == SIGNAL_SP_VOLTAGE)? "V" : "A");
  update_vscales();
  for(uint32_t i = 0; i < VSCALE_CNT; i++) {
    if(s_vscales[s_curr_signal][i] == s_curr_vscale) {
      restore_selected_scale(i);
      break;
    }
  }
}

/** @brief Click callback to show vertical scale menu */
static void enter_scale_menu_cb(lv_event_t *event)
{
  animation_move_to_side(ui_scrOscilloscope_cntMainSideMenu, 0, 184, 200);
  animation_move_to_side(ui_scrOscilloscope_cntScaleMenu, 184, 0, 200);
  // Update input group items
  SCR_CLEAR_GROUP();
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_btnScaleBack);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScale1);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScale2);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScale3);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScale4);
  // Point menu to this entry
  s_menu_page = MENU_PAGE_SCALE_V;
}

/** @brief Click callback to show horizontal scale menu */
static void enter_time_menu_cb(lv_event_t *event)
{
  animation_move_to_side(ui_scrOscilloscope_cntMainSideMenu, 0, 184, 200);
  animation_move_to_side(ui_scrOscilloscope_cntTimeMenu, 184, 0, 200);
  // Update input group items
  SCR_CLEAR_GROUP();
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_btnTimeBack);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntTime1);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntTime2);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntTime3);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntTime4);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntTime5);
  // Point menu to this entry
  s_menu_page = MENU_PAGE_SCALE_H;
}

/** @brief Click callback to go back to original main menu */
static void menu_go_back(lv_event_t *event)
{
  // Hide the previous menu
  if(s_menu_page == MENU_PAGE_SCALE_V) {
    animation_move_to_side(ui_scrOscilloscope_cntScaleMenu, 0, 184, 200);
  } else if(s_menu_page == MENU_PAGE_SCALE_H) {
    animation_move_to_side(ui_scrOscilloscope_cntTimeMenu, 0, 184, 200);
  }
  animation_move_to_side(ui_scrOscilloscope_cntMainSideMenu, 184, 0, 200);
  // Update input group items
  SCR_CLEAR_GROUP();
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntSigVoltage);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntSigCorriente);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScaleV);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntScaleH);
  SCR_ADD_TO_GROUP(ui_scrOscilloscope_cntBack);
  // Point menu to main menu
  s_menu_page = MENU_PAGE_MAIN;
}

/** @brief Click callback to update vertical scale */
static void change_vertical_scale_cb(lv_event_t *event)
{
  lv_obj_t *target = lv_event_get_target(event);
  lv_obj_t *parent = ui_scrOscilloscope_cntScales;
  uint32_t count = lv_obj_get_child_count(parent);

  int32_t selected = -1;
  for (uint32_t i = 0; i < count; i++) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    if (child == target) {
      selected = restore_selected_scale(i);
    }
  }
  if (selected < 0) return;

  if(s_curr_signal == SIGNAL_SP_VOLTAGE) {
    s_curr_voltage_scale = s_vscales[s_curr_signal][selected];
    s_curr_vscale = s_curr_voltage_scale;
  } else if(s_curr_signal == SIGNAL_SP_CURRENT) {
    s_curr_current_scale = s_vscales[s_curr_signal][selected];
    s_curr_vscale = s_curr_current_scale;
  }
  lv_label_set_text_fmt(ui_scrOscilloscope_lblTop, "+%d", s_curr_vscale);
  lv_label_set_text_fmt(ui_scrOscilloscope_lblBot, "-%d", s_curr_vscale);
  lv_label_set_text_fmt(ui_scrOscilloscope_lblScaleVValue, "%d %s", s_curr_vscale, (s_curr_signal == SIGNAL_SP_VOLTAGE)? "V" : "A");
}

/** @brief Call to update vertical scales when switching signal */
static void update_vscales(void) 
{
  lv_obj_t *parent = ui_scrOscilloscope_cntScales;
  uint32_t count = lv_obj_get_child_count(parent);
  for (uint32_t i = 0; i < count; i++) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    lv_obj_t *lbl = lv_obj_get_child(child, 0);
    lv_obj_t *tick  = lv_obj_get_child(child, 1);
    lv_color_t color = lv_color_hex(s_sig_colors[s_curr_signal]);

    lv_label_set_text_fmt(lbl, "%d %s", s_vscales[s_curr_signal][i], (s_curr_signal == SIGNAL_SP_VOLTAGE)? "V" : "A");
    lv_obj_set_style_bg_color(child, color, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(child, color, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(lbl, color, LV_STATE_CHECKED);
    lv_obj_set_style_image_recolor(tick, color, LV_STATE_CHECKED);
    lv_obj_invalidate(child);
  }
}

static int32_t restore_selected_scale(uint32_t idx) {

  lv_obj_t *parent = ui_scrOscilloscope_cntScales;
  uint32_t count = lv_obj_get_child_count(parent);

  int32_t selected = -1;
  for (uint32_t i = 0; i < count; i++) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    lv_obj_t *lbl = lv_obj_get_child(child, 0);
    lv_obj_t *tick  = lv_obj_get_child(child, 1);
    if (i == idx) {
      selected = (int32_t)i;
      lv_obj_add_state(child, LV_STATE_CHECKED);
      lv_obj_add_state(lbl, LV_STATE_CHECKED);
      lv_obj_add_state(tick, LV_STATE_CHECKED);
      lv_obj_remove_flag(tick, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_remove_state(child, LV_STATE_CHECKED);
      lv_obj_remove_state(lv_obj_get_child(child, 0), LV_STATE_CHECKED);
      lv_obj_remove_state(tick, LV_STATE_CHECKED);
      lv_obj_add_flag(tick, LV_OBJ_FLAG_HIDDEN);
    }
  }
  return selected;
}