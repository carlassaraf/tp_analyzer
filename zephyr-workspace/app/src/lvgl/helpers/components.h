#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <zephyr/drivers/rtc.h>
#include "lvgl.h"

/** 
 * @brief Handles topbar component update for datetime value
 * @param parent Pointer to LVGL parent
 * @param dt Pointer to datetime object
 */
void ui_topbar_update_datetime(lv_obj_t *parent,  struct rtc_time *dt);

#endif