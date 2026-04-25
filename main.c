#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app/app.h"

int main(void) {
    stdio_init_all();

    // Wait up to 2s for USB; continue regardless so the display works standalone.
    for (int i = 0; i < 20 && !stdio_usb_connected(); i++) { sleep_ms(100); }
    printf("Starting firmware...\n");

    if (!app_init()) {
        puts("app_init failed");
        return 1;
    }

    if (!app_run()) {
        puts("app_run failed");
        return 1;
    }

    vTaskStartScheduler();

    // Should never reach here
    for (;;) {}
    return 0;
}
