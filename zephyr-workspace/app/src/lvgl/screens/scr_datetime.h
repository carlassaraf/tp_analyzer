#ifndef SCR_DATETIME_H
#define SCR_DATETIME_H

#include <zephyr/drivers/rtc.h>

void scr_datetime_prepare(void);
void scr_datetime_init(void);
void scr_datetime_deinit(void);
void scr_datetime_step(void);

// Helpers

/** @brief Update RTC value for this screen */
void scr_datetime_update_datetime(struct rtc_time *dt);

#endif