#ifndef SCREENS_H
#define SCREENS_H

#include "lvgl.h"
#include "screens/scr_boot.h"
#include "screens/scr_menu.h"
#include "screens/scr_plotter.h"
#include "screens/scr_fft.h"

// Register a screen by name — expands to the three callbacks expected by screen_manager.
#define SCR_REGISTER(name, scr, fn_name)  { name, &scr, scr_##fn_name##_prepare, scr_##fn_name##_init, scr_##fn_name##_deinit, scr_##fn_name##_step }

// Add a widget to the default LVGL encoder group so it can receive input.
// Call this inside scr_xxx_init() for every focusable widget on the screen.
#define SCR_ADD_TO_GROUP(obj)   lv_group_add_obj(lv_group_get_default(), (obj))

// Remove all widgets from the default group. Call this in scr_xxx_deinit()
// if the screen added any widgets via SCR_ADD_TO_GROUP.
#define SCR_CLEAR_GROUP()       lv_group_remove_all_objs(lv_group_get_default())

/** 
 * @enum screen_id_t
 * @brief A way of keeping count and order screens in the application
 */
typedef enum screen_id {
    SCREEN_BOOT,
    SCREEN_MENU,
    SCREEN_PLOT,
    SCREEN_FFT,
    SCREEN_COUNT
} screen_id_t;

#endif /* SCREENS_H */