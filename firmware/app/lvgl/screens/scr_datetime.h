#ifndef SCR_DATETIME_H
#define SCR_DATETIME_H

#include "hal/hal_rtc.h"

void scr_datetime_prepare(void);
void scr_datetime_init(void);
void scr_datetime_deinit(void);
void scr_datetime_step(void);

// Helpers

/**
 * @brief
 */
void scr_datetime_update_datetime(hal_rtc_datetime_t *dt);

#endif