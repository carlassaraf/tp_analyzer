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
  void (*create)(void);    // SquareLine-generated ui_scrX_screen_init: builds the lv_obj_t tree
  void (*destroy)(void);   // SquareLine-generated ui_scrX_screen_destroy: frees it
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

void screen_manager_init(void) {
  // Only the first screen needs to exist up front; every other screen is
  // built lazily the first time we navigate to it (see screen_manager_step()
  // and _ui_screen_change() in ui_helpers.c), so LVGL's heap only ever has
  // to hold one screen's worth of widgets at startup instead of all of them.
  if(*screens[current].scr == NULL && screens[current].create) { screens[current].create(); }
  if(screens[current].prepare) { screens[current].prepare(); }
  if(screens[current].init)    { screens[current].init();    }
}

void screen_manager_go_to(screen_id_t id) {
  pending = id;
}

void screen_manager_step(void) {
  // Check if any event called _ui_screen_change
  if(current != screen_manager_get_active_screen()) {
    pending = screen_manager_get_active_screen();
  }
  // Resolve screen_manager_go_to calls
  if (pending != current) {
    // TEMP DEBUG: remove once the lazy-load crash is diagnosed.
    printf("screen_manager: %s -> %s\n", screens[current].name, screens[pending].name);
    lv_lock();
    if(screens[current].deinit) { screens[current].deinit(); }
    printf("screen_manager: deinit done\n");
    // Build the target screen on first visit. _ui_screen_change() already
    // does this for SquareLine-generated button events; this covers
    // screen_manager_go_to() callers (e.g. the boot timer) too.
    if(*screens[pending].scr == NULL && screens[pending].create) {
      printf("screen_manager: creating %s\n", screens[pending].name);
      screens[pending].create();
      printf("screen_manager: created %s (scr=%p)\n", screens[pending].name, (void*)*screens[pending].scr);
    }
    // Load the screen if not already changed by an LVGL event
    if(current == screen_manager_get_active_screen()) {
      lv_screen_load_anim(*screens[pending].scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
      printf("screen_manager: loaded %s\n", screens[pending].name);
    }
    // Always call prepare/init so one-time setup and group registration
    // (SCR_ADD_TO_GROUP) run regardless of whether the screen object was
    // just lazily created above or already existed.
    if(screens[pending].prepare) { screens[pending].prepare(); }
    printf("screen_manager: prepare done\n");
    if(screens[pending].init)    { screens[pending].init();    }
    printf("screen_manager: init done\n");
    lv_unlock();
    current = pending;
  }
  lv_lock();
  if(screens[current].step) { screens[current].step(); }
  lv_unlock();
}

screen_id_t screen_manager_get_active_screen(void) {
  for (screen_id_t id = SCREEN_BOOT; id < SCREEN_COUNT; id++) {
    if (lv_screen_active() == *(screens[id].scr)) {
      return id;
    }
  }
  return current;
}