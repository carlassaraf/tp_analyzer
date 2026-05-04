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
  screens[current].init();
}

void screen_manager_go_to(screen_id_t id) {
  pending = id;
}

void screen_manager_step(void) {
  if (pending != current) {
    lv_lock();
    screens[current].deinit();
    screens[pending].init();
    lv_unlock();
    current = pending;
  }
  lv_lock();
  screens[current].step();
  lv_unlock();
}
