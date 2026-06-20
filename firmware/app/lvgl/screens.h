#ifndef SCREENS_H
#define SCREENS_H

#include "lvgl.h"
#include "screens/scr_boot.h"
#include "screens/scr_datetime.h"
#include "screens/scr_fft.h"
#include "screens/scr_information.h"
#include "screens/scr_menu.h"
#include "screens/scr_oscilloscope.h"
#include "screens/scr_settings.h"

// Register a screen by name — expands to the SquareLine-generated create/destroy
// pair (so the screen can be built lazily, on first visit) plus the app-level
// lifecycle callbacks expected by screen_manager.
#define SCR_REGISTER(name, scr, fn_name) \
  { name, &scr, NULL, scr##_screen_init, scr##_screen_destroy, \
    scr_##fn_name##_prepare, scr_##fn_name##_init, scr_##fn_name##_deinit, scr_##fn_name##_step }

// Register screen version with topbar component
#define SCR_REGISTER_TB(name, scr, fn_name) \
  { name, &scr, &scr##_contTopBar, scr##_screen_init, scr##_screen_destroy, \
    scr_##fn_name##_prepare, scr_##fn_name##_init, scr_##fn_name##_deinit, scr_##fn_name##_step }

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
    SCREEN_DATETIME,
    SCREEN_FFT,
    SCREEN_INFO,
    SCREEN_MENU,
    SCREEN_OSC,
    SCREEN_SETTINGS,
    SCREEN_COUNT
} screen_id_t;

#endif /* SCREENS_H */