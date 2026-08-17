#include "app.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

// extern void ui_task(void *, void *, void *);
extern void ad_thread(void *, void *, void *);

// Thread definitions
K_THREAD_STACK_DEFINE(ad_thread_stack, CONFIG_AD_THREAD_STACK_SIZE);
struct k_thread ad_thread_data;
k_tid_t ad_tid;

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

bool app_init(void)
{
    return true;
}

bool app_run(void)
{
    ad_tid = k_thread_create(
        &ad_thread_data, ad_thread_stack,
        K_THREAD_STACK_SIZEOF(ad_thread_stack),
        ad_thread,
        NULL, NULL, NULL,
        CONFIG_AD_THREAD_PRIORITY, 0, K_NO_WAIT
    );

    // if (xTaskCreate(ui_task, "UI", configMINIMAL_STACK_SIZE * 16,
    //                 NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
    //     puts("Failed to create UI task");
    //     return false;
    // }

    return true;
}
