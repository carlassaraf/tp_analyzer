#include "app.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

extern void ui_task(void *params);

bool app_init(void) {
    return true;
}

bool app_run(void) {
    // UI task initializes LVGL internally (must run after scheduler starts)
    if (xTaskCreate(ui_task, "UI", configMINIMAL_STACK_SIZE * 16,
                    NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
        puts("Failed to create UI task");
        return false;
    }
    return true;
}
