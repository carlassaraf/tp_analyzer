#include "ui.h"
#include "lvgl/screens.h"
#include "lvgl/helpers/chart.h"
#include "lvgl/helpers/animations.h"

#define CHART_PIXEL_WIDTH     414
#define SIDE_MENU_X_HIDDEN    480
#define SIDE_MENU_X_VISIBLE   296
#define SIDE_MENU_TIMEOUT_MS  5000
#define SIDE_MENU_ANIM_MS     250
#define ENCODER_ACTIVE_THRESH 50   // ms: inactive_time below this → encoder just moved
#define INIT_GRACE_MS         200  // ignore encoder for 200ms after entering screen
#define VSCALE_CNT            4
#define SPAN_CNT              5

/** @brief Menu pages for this screen */
typedef enum {
  MENU_PAGE_MAIN,     /**< Default main menu */
  MENU_PAGE_SCALE_V,  /**< Vertical scale menu */
  MENU_PAGE_SPAN      /**< Frequency span menu */
} menu_page_t;

/** @brief Displayed signal */
typedef enum {
  SIGNAL_SP_VOLTAGE,  /**< Currently showing single phase voltage */
  SIGNAL_SP_CURRENT,  /**< Currently showing single phase current */
  SIGNAL_CNT,
} displayed_signal_t;

static lv_chart_series_t *s_fft_series    = NULL;
static lv_timer_t        *s_hide_timer    = NULL;
static bool               s_menu_visible  = false;
static uint32_t           s_init_tick;
static menu_page_t        s_menu_page     = MENU_PAGE_MAIN;
static displayed_signal_t s_curr_signal   = SIGNAL_SP_VOLTAGE;
static const uint32_t s_vscales[SIGNAL_CNT][VSCALE_CNT] = {
  {100, 250, 400, 800},  // SIGNAL_SP_VOLTAGE
  {5,   10,  20,  50},   // SIGNAL_SP_CURRENT
};
static const uint32_t s_spans[SPAN_CNT] = {
  100, 500, 1000, 2000, 5000
};
static const uint32_t s_sig_colors[SIGNAL_CNT] = { 0xFFD23F, 0x33D6EE };
static uint32_t s_curr_voltage_scale = 400;
static uint32_t s_curr_current_scale = 10;
static uint32_t s_curr_vscale        = 400;
static uint32_t s_curr_span          = 1000;

static void side_menu_show(void);
static void side_menu_hide(void);
static void hide_timer_cb(lv_timer_t *timer);
static void change_signal_cb(lv_event_t *event);
static void enter_scale_menu_cb(lv_event_t *event);
static void enter_span_menu_cb(lv_event_t *event);
static void menu_go_back(lv_event_t *event);
static void change_vertical_scale_cb(lv_event_t *event);
static void change_span_cb(lv_event_t *event);
static void update_vscales(void);
static void update_spans(void);
static int32_t restore_selected_scale(lv_obj_t *parent, uint32_t idx);

// Public functions

void scr_fft_update_chart(const float *magnitudes, uint16_t count, float freq_resolution)
{
  static float decimated[CHART_PIXEL_WIDTH];
  uint16_t bins_to_show = (uint16_t)((float)s_curr_span / freq_resolution);
  if (bins_to_show > count) bins_to_show = count;

  for (uint16_t px = 0; px < CHART_PIXEL_WIDTH; px++) {
    uint32_t start = (uint32_t)px * bins_to_show / CHART_PIXEL_WIDTH;
    uint32_t end   = (uint32_t)(px + 1) * bins_to_show / CHART_PIXEL_WIDTH;
    if (start == end) {
      decimated[px] = magnitudes[start < bins_to_show ? start : bins_to_show - 1];
    } else {
      float sum = 0.0f;
      for (uint32_t i = start; i < end; i++) sum += magnitudes[i];
      decimated[px] = sum / (end - start);
    }
  }

  ui_chart_push_float_data(ui_scrFFT_chartView, decimated, CHART_PIXEL_WIDTH, (float)s_curr_vscale);
}

void scr_fft_update_peak(float raw_peak)
{
  lv_label_set_text_fmt(ui_scrFFT_lblPeakValue, "%.1f", raw_peak * s_curr_vscale);
}

void scr_fft_update_rms(float raw_rms)
{
  lv_label_set_text_fmt(ui_scrFFT_lblRmsValue, "%.1f", raw_rms * s_curr_vscale);
}

void scr_fft_update_frequency(float frequency)
{
  lv_label_set_text_fmt(ui_scrFFT_lblFrequencyValue, "%.1f", frequency);
}

void scr_fft_update_thd(float thd)
{
  lv_label_set_text_fmt(ui_scrFFT_lblThdValue, "%.1f", thd);
}

// Life cycle functions

void scr_fft_prepare(void)
{
  s_fft_series = lv_chart_add_series(ui_scrFFT_chartView,
                    lv_color_hex(_ui_theme_color_Voltage[0]),
                    LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_point_count(ui_scrFFT_chartView, CHART_PIXEL_WIDTH);
  lv_obj_set_style_pad_column(ui_scrFFT_chartView, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  ui_chart_bind_ext_array(ui_scrFFT_chartView, CHART_PIXEL_WIDTH);
  // Propagate focus event to inside objects
  lv_obj_add_flag(ui_scrFFT_cntScaleV, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntScaleH, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntBack, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntScale1, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntScale2, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntScale3, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntScale4, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntSpan1, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntSpan2, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntSpan3, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntSpan4, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrFFT_cntSpan5, LV_OBJ_FLAG_EVENT_TRICKLE);
  // Add callback event to change signal shown
  lv_obj_add_event_cb(ui_scrFFT_cntSigVoltage, change_signal_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_cntSigCorriente, change_signal_cb, LV_EVENT_CLICKED, NULL);
  // Add callback event to menu entries
  lv_obj_add_event_cb(ui_scrFFT_cntScaleV, enter_scale_menu_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_cntScaleH, enter_span_menu_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_btnScaleBack, menu_go_back, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_btnSpanBack, menu_go_back, LV_EVENT_CLICKED, NULL);
  // Add vertical scales check callback
  lv_obj_add_event_cb(ui_scrFFT_cntScale1, change_vertical_scale_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_cntScale2, change_vertical_scale_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_cntScale3, change_vertical_scale_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_cntScale4, change_vertical_scale_cb, LV_EVENT_CLICKED, NULL);
  // Add frequency span check callback
  lv_obj_add_event_cb(ui_scrFFT_cntSpan1, change_span_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_cntSpan2, change_span_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_cntSpan3, change_span_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_cntSpan4, change_span_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrFFT_cntSpan5, change_span_cb, LV_EVENT_CLICKED, NULL);
}

void scr_fft_init(void)
{
  // Position the side menu off-screen and remove HIDDEN so animations work.
  // x=480 puts the left edge exactly at the screen's right boundary (menu width=184).
  lv_obj_set_x(ui_scrFFT_cntSideMenu, SIDE_MENU_X_HIDDEN);
  lv_obj_remove_flag(ui_scrFFT_cntSideMenu, LV_OBJ_FLAG_HIDDEN);
  s_menu_visible = false;
  s_init_tick    = lv_tick_get();
  // Update scales accordingly
  lv_chart_set_axis_range(ui_scrFFT_chartView, LV_CHART_AXIS_PRIMARY_Y, 0, s_curr_vscale);
  lv_label_set_text_fmt(ui_scrFFT_lblTop, "%d", s_curr_vscale);
  lv_label_set_text(ui_scrFFT_lblBot, "0");
  lv_label_set_text_fmt(ui_scrFFT_lblScaleVValue, "%d %s", s_curr_vscale, (s_curr_signal == SIGNAL_SP_VOLTAGE) ? "V" : "A");
  lv_label_set_text_fmt(ui_scrFFT_lblScaleHValue, "%d Hz", s_curr_span);
  update_vscales();
  update_spans();
  // Restore styles and states of vertical and frequency span scales
  for (uint32_t i = 0; i < VSCALE_CNT; i++) {
    if (s_vscales[s_curr_signal][i] == s_curr_vscale) {
      restore_selected_scale(ui_scrFFT_cntScales, i);
      break;
    }
  }
  for (uint32_t i = 0; i < SPAN_CNT; i++) {
    if (s_spans[i] == s_curr_span) {
      restore_selected_scale(ui_scrFFT_cntSpans, i);
      break;
    }
  }
}

void scr_fft_deinit(void)
{
  SCR_CLEAR_GROUP();
  if (s_hide_timer != NULL) {
    lv_timer_delete(s_hide_timer);
    s_hide_timer = NULL;
  }
  lv_obj_add_flag(ui_scrFFT_cntSideMenu, LV_OBJ_FLAG_HIDDEN);
  s_menu_visible = false;
}

void scr_fft_step(void)
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
  animation_move_to_side(ui_scrFFT_cntSideMenu,
                         SIDE_MENU_X_HIDDEN, SIDE_MENU_X_VISIBLE,
                         SIDE_MENU_ANIM_MS);
  s_hide_timer = lv_timer_create(hide_timer_cb, SIDE_MENU_TIMEOUT_MS, NULL);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntSigVoltage);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntSigCorriente);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntScaleV);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntScaleH);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntBack);
  s_menu_visible = true;
}

/** @brief Called to hide the side menu. Animates to right, clears the menu and input group */
static void side_menu_hide(void)
{
  animation_move_to_side(ui_scrFFT_cntSideMenu,
                         SIDE_MENU_X_VISIBLE, SIDE_MENU_X_HIDDEN,
                         SIDE_MENU_ANIM_MS);
  SCR_CLEAR_GROUP();
  s_menu_visible = false;
  // Revert submenus to proper positions
  lv_obj_set_x(ui_scrFFT_cntMainSideMenu, 0);
  lv_obj_set_x(ui_scrFFT_cntScaleMenu, 184);
  lv_obj_set_x(ui_scrFFT_cntSpanMenu, 184);
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
  if (target == ui_scrFFT_cntSigVoltage) {
    // Set signal in menu
    lv_obj_remove_state(ui_scrFFT_cntSigCorriente, LV_STATE_CHECKED);
    lv_obj_add_flag(ui_scrFFT_cntCorriente, LV_OBJ_FLAG_HIDDEN);
    // Set signal in screen
    lv_obj_add_state(ui_scrFFT_cntSigVoltage, LV_STATE_CHECKED);
    lv_obj_remove_flag(ui_scrFFT_cntTension, LV_OBJ_FLAG_HIDDEN);
    // Update units
    lv_label_set_text(ui_scrFFT_lblPeakUnit, "V");
    lv_label_set_text(ui_scrFFT_lblRmsUnit, "V");
    // Update colors
    lv_obj_set_style_text_color(ui_scrFFT_lblPeakValue, lv_color_hex(_ui_theme_color_Voltage[0]), LV_PART_MAIN);
    lv_chart_set_series_color(ui_scrFFT_chartView, s_fft_series, lv_color_hex(_ui_theme_color_Voltage[0]));
    lv_obj_invalidate(ui_scrFFT_chartView);
    // Update signal variable
    s_curr_signal = SIGNAL_SP_VOLTAGE;
    s_curr_vscale = s_curr_voltage_scale;
  } else if (target == ui_scrFFT_cntSigCorriente) {
    lv_obj_remove_state(ui_scrFFT_cntSigVoltage, LV_STATE_CHECKED);
    lv_obj_add_flag(ui_scrFFT_cntTension, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_state(ui_scrFFT_cntSigCorriente, LV_STATE_CHECKED);
    lv_obj_remove_flag(ui_scrFFT_cntCorriente, LV_OBJ_FLAG_HIDDEN);
    // Update units
    lv_label_set_text(ui_scrFFT_lblPeakUnit, "A");
    lv_label_set_text(ui_scrFFT_lblRmsUnit, "A");
    // Update colors
    lv_obj_set_style_text_color(ui_scrFFT_lblPeakValue, lv_color_hex(_ui_theme_color_Corriente[0]), LV_PART_MAIN);
    lv_chart_set_series_color(ui_scrFFT_chartView, s_fft_series, lv_color_hex(_ui_theme_color_Corriente[0]));
    lv_obj_invalidate(ui_scrFFT_chartView);
    // Update signal variable
    s_curr_signal = SIGNAL_SP_CURRENT;
    s_curr_vscale = s_curr_current_scale;
  }
  // Update scales with current signal
  lv_chart_set_axis_range(ui_scrFFT_chartView, LV_CHART_AXIS_PRIMARY_Y, 0, s_curr_vscale);
  lv_label_set_text_fmt(ui_scrFFT_lblScaleVValue, "%d %s", s_curr_vscale, (s_curr_signal == SIGNAL_SP_VOLTAGE) ? "V" : "A");
  lv_label_set_text_fmt(ui_scrFFT_lblUnit, "%s", (s_curr_signal == SIGNAL_SP_VOLTAGE) ? "V" : "A");
  update_vscales();
  update_spans();
  for (uint32_t i = 0; i < VSCALE_CNT; i++) {
    if (s_vscales[s_curr_signal][i] == s_curr_vscale) {
      restore_selected_scale(ui_scrFFT_cntScales, i);
      break;
    }
  }
}

/** @brief Click callback to show vertical scale menu */
static void enter_scale_menu_cb(lv_event_t *event)
{
  animation_move_to_side(ui_scrFFT_cntMainSideMenu, 0, 184, 200);
  animation_move_to_side(ui_scrFFT_cntScaleMenu, 184, 0, 200);
  // Update input group items
  SCR_CLEAR_GROUP();
  SCR_ADD_TO_GROUP(ui_scrFFT_btnScaleBack);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntScale1);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntScale2);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntScale3);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntScale4);
  // Point menu to this entry
  s_menu_page = MENU_PAGE_SCALE_V;
}

/** @brief Click callback to show frequency span menu */
static void enter_span_menu_cb(lv_event_t *event)
{
  animation_move_to_side(ui_scrFFT_cntMainSideMenu, 0, 184, 200);
  animation_move_to_side(ui_scrFFT_cntSpanMenu, 184, 0, 200);
  // Update input group items
  SCR_CLEAR_GROUP();
  SCR_ADD_TO_GROUP(ui_scrFFT_btnSpanBack);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntSpan1);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntSpan2);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntSpan3);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntSpan4);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntSpan5);
  // Point menu to this entry
  s_menu_page = MENU_PAGE_SPAN;
}

/** @brief Click callback to go back to original main menu */
static void menu_go_back(lv_event_t *event)
{
  // Hide the previous menu
  if (s_menu_page == MENU_PAGE_SCALE_V) {
    animation_move_to_side(ui_scrFFT_cntScaleMenu, 0, 184, 200);
  } else if (s_menu_page == MENU_PAGE_SPAN) {
    animation_move_to_side(ui_scrFFT_cntSpanMenu, 0, 184, 200);
  }
  animation_move_to_side(ui_scrFFT_cntMainSideMenu, 184, 0, 200);
  // Update input group items
  SCR_CLEAR_GROUP();
  SCR_ADD_TO_GROUP(ui_scrFFT_cntSigVoltage);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntSigCorriente);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntScaleV);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntScaleH);
  SCR_ADD_TO_GROUP(ui_scrFFT_cntBack);
  // Point menu to main menu
  s_menu_page = MENU_PAGE_MAIN;
}

/** @brief Click callback to update vertical scale */
static void change_vertical_scale_cb(lv_event_t *event)
{
  lv_obj_t *target = lv_event_get_target(event);
  lv_obj_t *parent = ui_scrFFT_cntScales;
  uint32_t count = lv_obj_get_child_count(parent);

  int32_t selected = -1;
  for (uint32_t i = 0; i < count; i++) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    if (child == target) {
      selected = restore_selected_scale(parent, i);
    }
  }
  if (selected < 0) return;

  if (s_curr_signal == SIGNAL_SP_VOLTAGE) {
    s_curr_voltage_scale = s_vscales[s_curr_signal][selected];
    s_curr_vscale = s_curr_voltage_scale;
  } else if (s_curr_signal == SIGNAL_SP_CURRENT) {
    s_curr_current_scale = s_vscales[s_curr_signal][selected];
    s_curr_vscale = s_curr_current_scale;
  }
  lv_chart_set_axis_range(ui_scrFFT_chartView, LV_CHART_AXIS_PRIMARY_Y, 0, s_curr_vscale);
  lv_label_set_text_fmt(ui_scrFFT_lblTop, "%d", s_curr_vscale);
  lv_label_set_text(ui_scrFFT_lblBot, "0");
  lv_label_set_text_fmt(ui_scrFFT_lblScaleVValue, "%d %s", s_curr_vscale, (s_curr_signal == SIGNAL_SP_VOLTAGE) ? "V" : "A");
}

/** @brief Click callback to update frequency span */
static void change_span_cb(lv_event_t *event)
{
  lv_obj_t *target = lv_event_get_target(event);
  lv_obj_t *parent = ui_scrFFT_cntSpans;
  uint32_t count = lv_obj_get_child_count(parent);

  int32_t selected = -1;
  for (uint32_t i = 0; i < count; i++) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    if (child == target) {
      selected = restore_selected_scale(parent, i);
    }
  }
  if (selected < 0) return;
  // Update current frequency span and label
  s_curr_span = s_spans[selected];
  lv_label_set_text_fmt(ui_scrFFT_lblScaleHValue, "%d Hz", s_curr_span);
}

/** @brief Call to update vertical scales when switching signal */
static void update_vscales(void)
{
  lv_obj_t *parent = ui_scrFFT_cntScales;
  uint32_t count = lv_obj_get_child_count(parent);
  for (uint32_t i = 0; i < count; i++) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    lv_obj_t *lbl   = lv_obj_get_child(child, 0);
    lv_obj_t *tick  = lv_obj_get_child(child, 1);
    lv_color_t color = lv_color_hex(s_sig_colors[s_curr_signal]);

    lv_label_set_text_fmt(lbl, "%d %s", s_vscales[s_curr_signal][i], (s_curr_signal == SIGNAL_SP_VOLTAGE) ? "V" : "A");
    lv_obj_set_style_bg_color(child, color, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(child, color, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(lbl, color, LV_STATE_CHECKED);
    lv_obj_set_style_image_recolor(tick, color, LV_STATE_CHECKED);
    lv_obj_invalidate(child);
  }
}

/** @brief Call to update frequency span options */
static void update_spans(void)
{
  lv_obj_t *parent = ui_scrFFT_cntSpans;
  uint32_t count = lv_obj_get_child_count(parent);
  for (uint32_t i = 0; i < count; i++) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    lv_obj_t *lbl   = lv_obj_get_child(child, 0);
    lv_obj_t *tick  = lv_obj_get_child(child, 1);
    lv_color_t color = lv_color_hex(s_sig_colors[s_curr_signal]);

    lv_label_set_text_fmt(lbl, "%d Hz", s_spans[i]);
    lv_obj_set_style_bg_color(child, color, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(child, color, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(lbl, color, LV_STATE_CHECKED);
    lv_obj_set_style_image_recolor(tick, color, LV_STATE_CHECKED);
    lv_obj_invalidate(child);
  }
}

static int32_t restore_selected_scale(lv_obj_t *parent, uint32_t idx)
{
  uint32_t count = lv_obj_get_child_count(parent);
  int32_t selected = -1;
  for (uint32_t i = 0; i < count; i++) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    lv_obj_t *lbl   = lv_obj_get_child(child, 0);
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
