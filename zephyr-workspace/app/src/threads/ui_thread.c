#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/logging/log.h>

#include "lvgl.h"
#include "lvgl/screen_manager.h"
#include "lvgl/screen_update.h"
#include "ui.h"

LOG_MODULE_REGISTER(ui_thread, LOG_LEVEL_INF);

// LVGL Display device
static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static const struct device *rtc = DEVICE_DT_GET(DT_NODELABEL(powman_rtc));

// Private prototypes and callbacks
static void ui_init_minimal(void);
static void rtc_timer_cb(struct k_timer *timer_id);

// Zephyr Timer to handle the RTC callback
K_TIMER_DEFINE(rtc_timer, rtc_timer_cb, NULL);

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
  screen_manager_init();
  screen_update_init();

  // RTC and SoftTimer initialization to set UI
  /** @todo Proper RTC initialization */
  struct rtc_time default_dt = { .tm_mday = 16, .tm_mon = 6, .tm_year = 2026, .tm_hour = 20, .tm_min = 52 };
  if (rtc_set_time(rtc, &default_dt)) {
    LOG_ERR("Failed to initialize RTC");
  }
  LOG_INF("Successfully initialized RTC to %02d/%02d/%02d %02d:%02d:00", 
    default_dt.tm_mday, default_dt.tm_mon + 1, default_dt.tm_year, default_dt.tm_hour, default_dt.tm_min);
  k_timer_start(&rtc_timer, K_SECONDS(60), K_SECONDS(60));
  // screen_update_cmd_push(SCREEN_UPDATE_DATETIME, (void*)&dt);

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
static void rtc_timer_cb(struct k_timer *timer_id)
{
  // Local datetime struct
  static struct rtc_time dt = {0};
  if(rtc_get_time(rtc, &dt)) {
    LOG_ERR("Unable to get RTC datetime");
    return;
  }
  // Update UI command
  // screen_update_cmd_push(SCREEN_UPDATE_DATETIME, (void*)&dt);
  LOG_INF("RTC datetime %02d/%02d/%02d %02d:%02d:00", 
    dt.tm_mday, dt.tm_mon + 1, dt.tm_year, dt.tm_hour, dt.tm_min);
}