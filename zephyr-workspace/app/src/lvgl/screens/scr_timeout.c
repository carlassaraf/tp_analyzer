#include "ui.h"
#include "scr_timeout.h"
#include "lvgl.h"
#include "lvgl/screens.h"

#include "dev_state/dev_state.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(scr_timeout, LOG_LEVEL_INF);

static uint32_t obj_to_timeout(lv_obj_t *obj);
static bool obj_is_off_timeout(lv_obj_t *obj);
static void timeout_set_cb(lv_event_t *e);

static lv_obj_t **btns[] = { 
  &ui_scrTimeout_btnScreenOff30s, &ui_scrTimeout_btnScreenOff1m, &ui_scrTimeout_btnScreenOff2m,
  &ui_scrTimeout_btnScreenOff5m, &ui_scrTimeout_btnScreenOff10m, &ui_scrTimeout_btnScreenOffNever,
  &ui_scrTimeout_btnScreenBack30s, &ui_scrTimeout_btnScreenBack1m, &ui_scrTimeout_btnScreenBack2m,
  &ui_scrTimeout_btnScreenBack5m, &ui_scrTimeout_btnScreenBack10m, &ui_scrTimeout_btnScreenBackNever
};

// Life cycle functions

void scr_timeout_prepare(void)
{
  for (uint8_t i = 0; i < sizeof(btns)/sizeof(btns[0]); i++) {
    lv_obj_t *btn = *(btns[i]);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_TRICKLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_STATE_TRICKLE);
    lv_obj_set_style_transform_width(btn, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_transform_height(btn, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn, timeout_set_cb, LV_EVENT_CLICKED, NULL);
  }
  // Check the button of the active timeout
  dev_state_t dev = {0};
  dev_state_get(&dev);
  uint32_t screen_timeout = dev.screen_timeout_ms;
  uint32_t off_timeout = dev.off_screen_timeout_ms;
  
  uint32_t timeouts[] = { 30000, 60000, 120000, 300000, 600000, 0 };
  for (uint8_t i = 0; i < sizeof(timeouts)/sizeof(timeouts[0]); i++) {
    if (screen_timeout == timeouts[i]) {
      lv_obj_add_state(*(btns[6 + i]), LV_STATE_CHECKED);
    }
    if (off_timeout == timeouts[i]) {
      lv_obj_add_state(*(btns[i]), LV_STATE_CHECKED);
    }
  }
}

void scr_timeout_init(void)
{
  SCR_ADD_TO_GROUP(ui_scrTimeout_btnBack);
  for (uint8_t i = 0; i < sizeof(btns)/sizeof(btns[0]); i++) {
    SCR_ADD_TO_GROUP(*(btns[i]));
  }
}

void scr_timeout_deinit(void)
{
  SCR_CLEAR_GROUP();
}

void scr_timeout_step(void)
{

}

// Private functions

static uint32_t obj_to_timeout(lv_obj_t *obj)
{
  if (obj == ui_scrTimeout_btnScreenOff30s || obj == ui_scrTimeout_btnScreenBack30s) {
    return 30000;
  }
  if (obj == ui_scrTimeout_btnScreenOff1m || obj == ui_scrTimeout_btnScreenBack1m) {
    return 60000;
  }
  if (obj == ui_scrTimeout_btnScreenOff2m || obj == ui_scrTimeout_btnScreenBack2m) {
    return 120000;
  }
  if (obj == ui_scrTimeout_btnScreenOff5m || obj == ui_scrTimeout_btnScreenBack5m) {
    return 300000;
  }
  if (obj == ui_scrTimeout_btnScreenOff10m || obj == ui_scrTimeout_btnScreenBack10m) {
    return 600000;
  }
  if (obj == ui_scrTimeout_btnScreenOffNever || obj == ui_scrTimeout_btnScreenBackNever) {
    return 0;
  }
  return 0;
}

static bool obj_is_off_timeout(lv_obj_t *obj)
{
  return (
    obj == ui_scrTimeout_btnScreenOff30s || 
    obj == ui_scrTimeout_btnScreenOff1m ||
    obj == ui_scrTimeout_btnScreenOff2m ||
    obj == ui_scrTimeout_btnScreenOff5m ||
    obj == ui_scrTimeout_btnScreenOff10m ||
    obj == ui_scrTimeout_btnScreenOffNever
  );
}

static void timeout_set_cb(lv_event_t *e)
{
  lv_obj_t *target = lv_event_get_target_obj(e);

  if (obj_is_off_timeout(target)) {
    // Clear only the type of the target
    for (uint8_t i = 0; i < (int)(sizeof(btns)/sizeof(btns[0]) / 2); i++) {
      lv_obj_remove_state(*(btns[i]), LV_STATE_CHECKED);
    }
  } else {
    for (uint8_t i = (int)(sizeof(btns)/sizeof(btns[0]) / 2); i < sizeof(btns)/sizeof(btns[0]); i++) {
      lv_obj_remove_state(*(btns[i]), LV_STATE_CHECKED);
    }
  }

  lv_obj_add_state(target, LV_STATE_CHECKED);
  uint32_t timeout = obj_to_timeout(target);
  dev_state_t dev = {0};
  dev_state_get(&dev);
  
  if (obj_is_off_timeout(target)) {
    if (dev.off_screen_timeout_ms != timeout) {
      // Only update when there's a change
      dev_state_set_off_timeout(timeout);
    }
  } else {
    if (dev.screen_timeout_ms != timeout) {
      dev_state_set_screen_timeout(timeout);
    }
  }
}