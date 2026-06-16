#include "FreeRTOS.h"
#include "task.h"
#include "lvgl.h"
#include "services/lvgl/lvgl_port.h"
#include "lvgl/screen_manager.h"
#include "lvgl/screen_update.h"
#include "ui.h"
#include <stdio.h>

static void ui_init_minimal(void) {
  LV_EVENT_GET_COMP_CHILD = lv_event_register_id();
  ui_scrBoot_screen_init();
  // ui____initial_actions0 = lv_obj_create(NULL);
  lv_screen_load(ui_scrBoot);
}

void ui_task(void *params) {
  if (lvgl_port_init() != 0) {
    puts("lvgl_port_init failed");
    vTaskDelete(NULL);
    return;
  }

  ui_init_minimal();
  screen_manager_init();
  screen_update_init();

  while (true) {
    lv_task_handler();
    screen_update();
    screen_manager_step();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
