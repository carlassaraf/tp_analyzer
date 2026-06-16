#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include <stdint.h>
#include "lvgl.h"

int32_t lvgl_port_init(void);
lv_indev_t *lvgl_port_get_encoder(void);
/**
 * @brief Returns the encoder step count since the last call and resets it to 0.
 * Safe to call once per frame from a screen's step()
 * @note lv_task_handler() (which drains the raw encoder via encoder_pop_delta()) 
 * always runs before screen_manager_step() in the UI task loop, so this just 
 * hands back the value LVGL itself already consumed for group navigation that frame.
 * @return negative steps for ccw and positive steps for cw rotation
 */
int16_t lvgl_port_get_encoder_diff(void);

#endif