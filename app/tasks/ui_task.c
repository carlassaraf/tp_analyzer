#include "FreeRTOS.h"
#include "task.h"
#include "lvgl.h"
#include "services/lvgl/lvgl_port.h"
#include <stdio.h>

void ui_task(void *params) {
    // LVGL must be initialized after the scheduler is running because
    // LV_OS_FREERTOS creates internal OS primitives during lv_init().
    if (lvgl_port_init() != 0) {
        puts("lvgl_port_init failed");
        vTaskDelete(NULL);
        return;
    }

    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hola Mundo!");
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);

    while (true) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
