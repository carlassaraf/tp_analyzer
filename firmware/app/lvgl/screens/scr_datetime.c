#include "ui.h"
#include "lvgl/screens.h"
#include "lvgl_port.h"

#include <stdlib.h>

// One entry per editable field. `value` is kept in sync with the label text
// at all times so scr_datetime_step() never has to re-parse it.
typedef struct {
  lv_obj_t *cnt;    /**< Container widget to match */
  lv_obj_t *lbl;    /**< Label containing actual field value */
  int32_t   value;  /**< Matching RAM field value */
  int32_t   min;    /**< Lower cap */
  int32_t   max;    /**< Upper cap */
} datetime_field_t;

/** @brief List of available datetime fields */
typedef enum {
  FIELD_DAY,
  FIELD_MONTH,
  FIELD_YEAR,
  FIELD_HOUR,
  FIELD_MIN,
  FIELD_COUNT,
} datetime_field_id_t;

// Private variables

static datetime_field_t s_fields[FIELD_COUNT];
static datetime_field_t *s_locked_field = NULL;
static bool locked_input_group = false;

// Callbacks and private functions

static void datetime_restore_full_input_group(void);
static void datetime_lock_input_group(lv_obj_t *obj);
static void datetime_click_cb(lv_event_t *event);
static int32_t datetime_parse_value(lv_obj_t *lbl);
static datetime_field_t *datetime_field_for_obj(lv_obj_t *obj);
static void datetime_field_update_labels(void);

// Screen lifecycle

void scr_datetime_prepare(void)
{
  // Pass events 
  lv_obj_add_flag(ui_scrDatetime_cntDay, LV_OBJ_FLAG_STATE_TRICKLE);
  lv_obj_add_flag(ui_scrDatetime_cntMonth, LV_OBJ_FLAG_STATE_TRICKLE);
  lv_obj_add_flag(ui_scrDatetime_cntYear, LV_OBJ_FLAG_STATE_TRICKLE);
  lv_obj_add_flag(ui_scrDatetime_cntHour, LV_OBJ_FLAG_STATE_TRICKLE);
  lv_obj_add_flag(ui_scrDatetime_cntMin, LV_OBJ_FLAG_STATE_TRICKLE);

  // Add callbacks to datetime containers
  lv_obj_add_event_cb(ui_scrDatetime_cntDay, datetime_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrDatetime_cntMonth, datetime_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrDatetime_cntYear, datetime_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrDatetime_cntHour, datetime_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_scrDatetime_cntMin, datetime_click_cb, LV_EVENT_CLICKED, NULL);

  // Seed each field from whatever the label currently shows, so the encoder edits start from the displayed value instead of clobbering it.
  s_fields[FIELD_DAY]   = (datetime_field_t){ ui_scrDatetime_cntDay,   ui_scrDatetime_lblDayV,   s_fields[FIELD_DAY].value,   1,  31 };
  s_fields[FIELD_MONTH] = (datetime_field_t){ ui_scrDatetime_cntMonth, ui_scrDatetime_lblMonthV, s_fields[FIELD_MONTH].value, 1,  12 };
  s_fields[FIELD_YEAR]  = (datetime_field_t){ ui_scrDatetime_cntYear,  ui_scrDatetime_lblYearV,  s_fields[FIELD_YEAR].value % 100,  0,  99 };
  s_fields[FIELD_HOUR]  = (datetime_field_t){ ui_scrDatetime_cntHour,  ui_scrDatetime_lblHourV,  s_fields[FIELD_HOUR].value,  0,  23 };
  s_fields[FIELD_MIN]   = (datetime_field_t){ ui_scrDatetime_cntMin,   ui_scrDatetime_lblMinV,   s_fields[FIELD_MIN].value,   0,  59 };
}

void scr_datetime_init(void)
{
  SCR_ADD_TO_GROUP(ui_scrDatetime_btnBack);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntDay);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntMonth);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntYear);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntHour);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntMin);
  SCR_ADD_TO_GROUP(ui_scrDatetime_btnSave);
  // Update labels
  datetime_field_update_labels();
}

void scr_datetime_deinit(void)
{
  SCR_CLEAR_GROUP();
}

void scr_datetime_step(void)
{
  // Only edit a value while the group is locked on one of the fields.
  if (!locked_input_group || s_locked_field == NULL) { return; }

  int16_t diff = lvgl_port_get_encoder_diff();
  if (diff == 0) { return; }

  int32_t value = s_locked_field->value + diff;
  // Clamp (not wrap) so the user can't dial a field past its logical range,
  // e.g. minutes past 59 or hours past 23.
  if (value < s_locked_field->min) { value = s_locked_field->min; }
  if (value > s_locked_field->max) { value = s_locked_field->max; }

  s_locked_field->value = value;
  lv_label_set_text_fmt(s_locked_field->lbl, "\n%02d", (int)value);
}

/** @brief Update field values with RTC data */
void scr_datetime_update_datetime(hal_rtc_datetime_t *dt) {
  s_fields[FIELD_DAY].value = dt->day;
  s_fields[FIELD_MONTH].value = dt->month;
  s_fields[FIELD_YEAR].value = dt->year % 100;
  s_fields[FIELD_HOUR].value = dt->hour;
  s_fields[FIELD_MIN].value = dt->min;
}

// Private functions

/** @brief Call to restore all interactive widgets to input group */
static void datetime_restore_full_input_group(void)
{
  SCR_CLEAR_GROUP();
  SCR_ADD_TO_GROUP(ui_scrDatetime_btnBack);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntDay);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntMonth);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntYear);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntHour);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntMin);
  SCR_ADD_TO_GROUP(ui_scrDatetime_btnSave);
}

/** @brief Call to lock input group in a single datetime field */
static void datetime_lock_input_group(lv_obj_t *obj)
{
  SCR_CLEAR_GROUP();
  SCR_ADD_TO_GROUP(obj);
}

/** @brief Called to lock/unlock input group on a single datetime field */
static void datetime_click_cb(lv_event_t *event)
{
  lv_obj_t *target = lv_event_get_target(event);
  // Dont let user move to other object without unlocking
  if(!locked_input_group) {
    // Lock field to modify with encoder
    datetime_lock_input_group(target);
    s_locked_field = datetime_field_for_obj(target);
    locked_input_group = true;
  } else {
    // Unlock field and restore normal operation
    datetime_restore_full_input_group();
    lv_group_focus_obj(target);
    s_locked_field = NULL;
    locked_input_group = false;
  }
}

/** @brief Reads the numeric value out of a field's label (format "\nNN") */
static int32_t datetime_parse_value(lv_obj_t *lbl)
{
  const char *text = lv_label_get_text(lbl);
  while (*text && (*text < '0' || *text > '9')) { text++; }
  return (int32_t)atoi(text);
}

/** @brief Finds the field struct whose container is `obj`, or NULL */
static datetime_field_t *datetime_field_for_obj(lv_obj_t *obj)
{
  for (int i = 0; i < FIELD_COUNT; i++) {
    if (s_fields[i].cnt == obj) { return &s_fields[i]; }
  }
  return NULL;
}

/** @brief Helper to update all datetime field labels from RAM */
static void datetime_field_update_labels(void)
{
  for (int i = 0; i < FIELD_COUNT; i++) {
    if(s_fields[i].lbl) {
      lv_label_set_text_fmt(s_fields[i].lbl, "\n%d", s_fields[i].value);
    }
  }
}
