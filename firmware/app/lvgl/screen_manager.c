#include "ui.h"
#include "screen_manager.h"

#include <stdio.h>

/**
 * @struct screen
 * @brief Screen struct for the manager to register
 * and run callback functions
 */
typedef struct screen {
  const char *name;
  lv_obj_t **scr;
  void (*prepare)(void);
  void (*init)(void);
  void (*deinit)(void);
  void (*step)(void);
} screen_t;

// Used to register all screens in the applications with their callback functions
static const screen_t screens[SCREEN_COUNT] = {
  [SCREEN_BOOT]     = SCR_REGISTER("Boot",          ui_scrBoot,         boot    ),
  [SCREEN_DATETIME] = SCR_REGISTER("Datetime",      ui_scrDatetime,     datetime),
  [SCREEN_FFT]      = SCR_REGISTER("FFT",           ui_scrFFT,          fft     ),
  [SCREEN_MENU]     = SCR_REGISTER("Menu",          ui_scrMenu,         menu    ),
  [SCREEN_OSC]      = SCR_REGISTER("Oscilloscope",  ui_scrOscilloscope, oscilloscope ),
  [SCREEN_SETTINGS] = SCR_REGISTER("Setting",       ui_scrSettings,     settings ),
};

// Keep track of running screens and transitions
static screen_id_t current = SCREEN_BOOT;
static screen_id_t pending = SCREEN_BOOT;

static screen_id_t screen_manager_get_screen(void);

void screen_manager_init(void) {
  // Prepare screens
  for(screen_id_t i = 0; i < SCREEN_COUNT; i++) {
    if(screens[i].prepare) { screens[i].prepare(); }
  }
  // Init first screen
  if(screens[current].init) { screens[current].init(); }
}

void screen_manager_go_to(screen_id_t id) {
  pending = id;
}

void screen_manager_step(void) {
  // Check if any event called _ui_screen_change
  if(current != screen_manager_get_screen()) {
    pending = screen_manager_get_screen();
  }
  // Resolve screen_manager_go_to calls
  if (pending != current) {
    lv_lock();
    if(screens[current].deinit) { screens[current].deinit(); }
    // Load the screen if not already changed by an LVGL event
    if(current == screen_manager_get_screen()) {
      lv_screen_load_anim(*screens[pending].scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }
    // Always call init so group registration (SCR_ADD_TO_GROUP) runs regardless
    // of whether the screen object was pre-created by ui_init()
    if(screens[pending].init) { screens[pending].init(); }
    lv_unlock();
    current = pending;
  }
  lv_lock();
  if(screens[current].step) { screens[current].step(); }
  lv_unlock();
}

/** @brief Returns ID of current active screen */
static screen_id_t screen_manager_get_screen(void) {
  for (screen_id_t id = SCREEN_BOOT; id < SCREEN_COUNT; id++) {
    if (lv_screen_active() == *(screens[id].scr)) {
      return id;
    }
  }
  return current;
}