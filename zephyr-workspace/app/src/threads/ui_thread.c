#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>

#include "lvgl.h"
#include "lvgl/screen_manager.h"
#include "lvgl/screen_update.h"
#include "ui.h"

LOG_MODULE_REGISTER(ui_thread, LOG_LEVEL_INF);

// LVGL itself is already initialized by this point: CONFIG_LV_Z_AUTO_INIT
// (default y) runs the module's lvgl_init() via SYS_INIT before app_run(),
// wiring up display + input devices straight from the "zephyr,display"
// chosen node in the board overlay. No hand-rolled lvgl_port_init() needed —
// we just grab the same device to flip blanking off once the first frame
// is drawn, same as zephyr/samples/subsys/display/lvgl.
static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

// Private prototypes and callbacks
static void ui_init_minimal(void);
// static void rtc_timer_cb(TimerHandle_t timer);

void ui_thread(void *param1, void *param2, void *param3)
{
  ARG_UNUSED(param1);
  ARG_UNUSED(param2);
  ARG_UNUSED(param3);

  if (!device_is_ready(display_dev)) {
    LOG_ERR("Display device not ready");
    return;
  }

  // UI related initialization
  ui_init_minimal();
  // screen_manager_init();
  // screen_update_init();

  // RTC and SoftTimer initialization to set UI
  /** @todo Proper RTC initialization */
  // hal_rtc_datetime_t dt = { .day = 16, .month = 6, .year = 2026, .hour = 20, .min = 52 };
  // hal_rtc_set(&dt);
  // screen_update_cmd_push(SCREEN_UPDATE_DATETIME, (void*)&dt);

  // Software Timer every minute
  // xTimerStart(xTimerCreate(
  //   "RTC SoftTimer",
  //   pdMS_TO_TICKS(60000),
  //   pdTRUE,
  //   NULL,
  //   rtc_timer_cb
  // ), 0);

  // Render the first frame before turning blanking off, so we don't flash
  // whatever garbage was left in the panel's RAM at boot.
  lv_timer_handler();
  if (display_blanking_off(display_dev) < 0) {
    LOG_ERR("Failed to turn display blanking off");
  }

  LOG_INF("Successfully initialized display");

  while (true) {
    // screen_update();
    // screen_manager_step();
    lv_timer_handler();
    k_msleep(5);
  }
}

// Private functions

/** @brief Trimmed out version of ui_init from SquareLine Studio */
static void ui_init_minimal(void) {
  LV_EVENT_GET_COMP_CHILD = lv_event_register_id();
  ui_scrBoot_screen_init();
  lv_screen_load(ui_scrBoot);
}

/**
 * @brief Called every 1 min to update datetime in UI
 */
// static void rtc_timer_cb(TimerHandle_t timer)
// {
  // Local datetime struct
  // static hal_rtc_datetime_t dt = {0};
  // if(hal_rtc_get(&dt)) {
  //   // Update UI command
  //   screen_update_cmd_push(SCREEN_UPDATE_DATETIME, (void*)&dt);
  // }
// }