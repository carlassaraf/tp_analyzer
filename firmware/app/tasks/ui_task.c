#include "FreeRTOS.h"
#include "task.h"
#include "lvgl.h"
#include "services/lvgl/lvgl_port.h"
#include "lvgl/screen_manager.h"
#include "lvgl/screen_update.h"
#include "ui.h"
#include <stdio.h>

void ui_task(void *params) {
  if (lvgl_port_init() != 0) {
    puts("lvgl_port_init failed");
    vTaskDelete(NULL);
    return;
  }

  ui_init();
  screen_manager_init();
  screen_update_init();

  while (true) {
    lv_task_handler();
    screen_update();
    screen_manager_step();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
