#include "app.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

#include "dev_state/dev_state.h"

extern void ui_thread(void *, void *, void *);
extern void ad_thread(void *, void *, void *);

// Thread definitions

K_THREAD_STACK_DEFINE(ad_thread_stack, CONFIG_AD_THREAD_STACK_SIZE);
struct k_thread ad_thread_data;
k_tid_t ad_tid;

K_THREAD_STACK_DEFINE(ui_thread_stack, CONFIG_UI_THREAD_STACK_SIZE);
struct k_thread ui_thread_data;
k_tid_t ui_tid;

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

bool app_init(void)
{
    return true;
}

bool app_run(void)
{
    dev_state_init();

    ad_tid = k_thread_create(
        &ad_thread_data, ad_thread_stack,
        K_THREAD_STACK_SIZEOF(ad_thread_stack),
        ad_thread,
        NULL, NULL, NULL,
        CONFIG_AD_THREAD_PRIORITY, 0, K_NO_WAIT
    );

    ui_tid = k_thread_create(
        &ui_thread_data, ui_thread_stack,
        K_THREAD_STACK_SIZEOF(ui_thread_stack),
        ui_thread,
        NULL, NULL, NULL,
        CONFIG_UI_THREAD_PRIORITY, 0, K_NO_WAIT
    );

    return true;
}
