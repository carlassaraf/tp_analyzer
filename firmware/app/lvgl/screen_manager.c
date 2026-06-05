#include "ui.h"
#include "screen_manager.h"

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
  [SCREEN_BOOT] = SCR_REGISTER("Boot",    ui_scrBoot,     boot    ),
  [SCREEN_PLOT] = SCR_REGISTER("Plotter", ui_scrPlotter,  plotter ),
};

// Keep track of running screens and transitions
static screen_id_t current = SCREEN_BOOT;
static screen_id_t pending = SCREEN_BOOT;

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
  if (pending != current) {
    lv_lock();
    if(screens[current].deinit) { screens[current].deinit(); }
    if(screens[pending].init) { 
      _ui_screen_change(screens[pending].scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, screens[pending].init);
      screens[pending].init();
    }
    lv_unlock();
    current = pending;
  }
  lv_lock();
  if(screens[current].step) { screens[current].step(); }
  lv_unlock();
}
