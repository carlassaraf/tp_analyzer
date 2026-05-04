#include "ui.h"
#include "lvgl/screen_manager.h"

// Time to wait in the boot screen
#define SCR_BOOT_TIME_WAIT_MS 5000

static lv_timer_t *timer_handle = NULL;

/** @brief Handles changing screen after timeout */
static void timer_cb(lv_timer_t *timer) {
  screen_manager_go_to(SCREEN_HOME);
  lv_timer_delete(timer_handle);
}

void scr_boot_init(void) {
  // Create timer to change screen
  timer_handle = lv_timer_create(timer_cb, SCR_BOOT_TIME_WAIT_MS, NULL);
  _ui_screen_change(&ui_scrBoot, LV_SCR_LOAD_ANIM_NONE, 0, 0, &ui_scrBoot_screen_init);
}

void scr_boot_deinit(void) {
}

void scr_boot_step(void) {
}
