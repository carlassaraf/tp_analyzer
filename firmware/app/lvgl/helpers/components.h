#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "lvgl.h"
#include "hal/hal_rtc.h"

/** 
 * @brief Handles topbar component update for datetime value
 * @param parent Pointer to LVGL parent
 * @param dt Pointer to datetime object
 */
void ui_topbar_update_datetime(lv_obj_t *parent, hal_rtc_datetime_t *dt);

#endif