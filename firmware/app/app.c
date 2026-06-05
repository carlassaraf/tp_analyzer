#include "app.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

extern void ui_task(void *params);
extern void ad_task(void *params);

bool app_init(void) {
    return true;
}

bool app_run(void) {
    if (xTaskCreate(ui_task, "UI", configMINIMAL_STACK_SIZE * 16,
                    NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
        puts("Failed to create UI task");
        return false;
    }

    if (xTaskCreate(ad_task, "ADC", configMINIMAL_STACK_SIZE * 4,
                    NULL, tskIDLE_PRIORITY + 2, NULL) != pdPASS) {
        puts("Failed to create ADC task");
        return false;
    }

    return true;
}
