#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "lvgl.h"
#include "services/lvgl/lvgl_port.h"
#include "lvgl/screen_manager.h"
#include "lvgl/screen_update.h"
#include "ui.h"

#include "hal_rtc.h"

#include <stdio.h>

// Private prototypes and callbacks
static void ui_init_minimal(void);
static void rtc_timer_cb(TimerHandle_t timer);

void ui_task(void *params) {
  if (lvgl_port_init() != 0) {
    puts("lvgl_port_init failed");
    vTaskDelete(NULL);
    return;
  }

  // UI related initialization
  ui_init_minimal();
  screen_manager_init();
  screen_update_init();

  // RTC and SoftTimer initialization to set UI
  /** @todo Proper RTC initialization */
  hal_rtc_datetime_t dt = { .day = 16, .month = 6, .year = 2026, .hour = 20, .min = 52 };
  hal_rtc_set(&dt);
  screen_update_cmd_push(SCREEN_UPDATE_DATETIME, (void*)&dt);

  // Software Timer every minute
  xTimerStart(xTimerCreate(
    "RTC SoftTimer",
    pdMS_TO_TICKS(60000),
    pdTRUE,
    NULL,
    rtc_timer_cb
  ), 0);

  while (true) {
    lv_task_handler();
    screen_update();
    screen_manager_step();
    vTaskDelay(pdMS_TO_TICKS(10));
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
static void rtc_timer_cb(TimerHandle_t timer)
{
  // Local datetime struct
  static hal_rtc_datetime_t dt = {0};
  if(hal_rtc_get(&dt)) {
    // Update UI command
    screen_update_cmd_push(SCREEN_UPDATE_DATETIME, (void*)&dt);
  }
}