#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    while(1) {
        LOG_INF("Hello Zephyr...");
        k_msleep(200);
    }

    // if (!app_init()) {
    //     puts("app_init failed");
    //     return 1;
    // }

    // if (!app_run()) {
    //     puts("app_run failed");
    //     return 1;
    // }

    // vTaskStartScheduler();

    // // Should never reach here
    // for (;;) {}
    return 0;
}
