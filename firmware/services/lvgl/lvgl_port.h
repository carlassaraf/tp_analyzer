#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include <stdint.h>
#include "lvgl.h"

int32_t     lvgl_port_init(void);
lv_indev_t *lvgl_port_get_encoder(void);

#endif