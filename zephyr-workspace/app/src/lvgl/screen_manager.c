#include "ui.h"
#include "screen_manager.h"

#include "lvgl/helpers/components.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @struct screen
 * @brief Screen struct for the manager to register
 * and run callback functions
 */
typedef struct screen {
  const char *name;
  lv_obj_t **scr;         /**< Screen pointer object */
  lv_obj_t **topbar;      /**< Common topbar component across most screen. NULL when it's not available */
  void (*create)(void);   /**< SquareLine Studio init function */
  void (*destroy)(void);  /**< SquareLine Studio destroy function */
  void (*prepare)(void);  /**< Called one time after screen creation */
  void (*init)(void);     /**< Called when entering the screen to set up */
  void (*deinit)(void);   /**< Called after leaving screen to clear up local variables/states */
  void (*step)(void);     /**< Called every screen_manager step cycle */
} screen_t;

// Longest spd/anim-time passed to _ui_screen_change() anywhere in the
// generated ui_scrX.c event handlers (currently 200ms, for the Settings
// <-> Menu/Datetime slides). LVGL keeps the screen we're leaving alive for
// the whole animation, so we must not free it before this elapses, or the
// in-flight slide would render/animate a deleted object. Bump this if a
// slower transition is ever added in SquareLine.
#define SCR_DESTROY_DELAY_MS 300

// Used to register all screens in the applications with their callback functions
static const screen_t screens[SCREEN_COUNT] = {
  [SCREEN_BOOT]     = SCR_REGISTER("Boot",            ui_scrBoot,         boot    ),
  [SCREEN_DATETIME] = SCR_REGISTER_TB("Datetime",     ui_scrDatetime,     datetime),
  [SCREEN_FFT]      = SCR_REGISTER_TB("FFT",          ui_scrFFT,          fft     ),
  [SCREEN_INFO]     = SCR_REGISTER_TB("Info",         ui_scrInformation,  information),
  [SCREEN_MENU]     = SCR_REGISTER_TB("Menu",         ui_scrMenu,         menu    ),
  [SCREEN_OSC]      = SCR_REGISTER_TB("Oscilloscope", ui_scrOscilloscope, oscilloscope ),
  [SCREEN_SETTINGS] = SCR_REGISTER_TB("Setting",      ui_scrSettings,     settings ),
};

// Keep track of running screens and transitions
static screen_id_t current = SCREEN_BOOT;
static screen_id_t pending = SCREEN_BOOT;

// Tracks whether prepare() has already run for the screen's *current*
// lv_obj_t (not "ever" — destroy_timer_cb() clears the bit when a screen is
// freed, so a later rebuild gets prepare() re-run exactly once again).
// Needed because prepare() does non-idempotent setup (lv_chart_add_series,
// lv_obj_add_event_cb on scr_fft/scr_oscilloscope) and the screen object may
// have been built by us *or* by _ui_screen_change() in ui_helpers.c, so we
// can't infer "just built" from timing alone.
static bool prepared[SCREEN_COUNT] = { false };

static void destroy_timer_cb(lv_timer_t *timer)
{
  screen_id_t id = (screen_id_t)(intptr_t)lv_timer_get_user_data(timer);
  lv_lock();
  // Skip if the user already navigated back to it before the delay elapsed
  // (and skip re-checking against an in-flight transition target too).
  if (id != screen_manager_get_active_screen() && id != pending && screens[id].destroy) {
    screens[id].destroy();
    prepared[id] = false;
  }
  lv_unlock();
}

void screen_manager_init(void)
{
  // Only the first screen needs to exist up front; every other screen is
  // built lazily the first time we navigate to it (see screen_manager_step()
  // and _ui_screen_change() in ui_helpers.c), so LVGL's heap only ever has
  // to hold one screen's worth of widgets at startup instead of all of them.
  if(*screens[current].scr == NULL && screens[current].create) { screens[current].create(); }
  if(!prepared[current] && screens[current].prepare) { screens[current].prepare(); }
  prepared[current] = true;
  if(screens[current].init) { screens[current].init(); }
}

void screen_manager_go_to(screen_id_t id)
{
  pending = id;
}

void screen_manager_step(void)
{
  // Check if any event called _ui_screen_change
  if(current != screen_manager_get_active_screen()) {
    pending = screen_manager_get_active_screen();
  }
  // Resolve screen_manager_go_to calls
  if (pending != current) {
    screen_id_t leaving = current;
    lv_lock();
    if(screens[current].deinit) { screens[current].deinit(); }
    // Build the target screen on first visit. _ui_screen_change() already
    // does this for SquareLine-generated button events; this covers
    // screen_manager_go_to() callers (e.g. the boot timer) too.
    if(*screens[pending].scr == NULL && screens[pending].create) {
      screens[pending].create();
    }
    // Load the screen if not already changed by an LVGL event
    if(current == screen_manager_get_active_screen()) {
      lv_screen_load_anim(*screens[pending].scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }
    // Run prepare() exactly once per build of this screen (see `prepared`
    // above), then always run init() so group registration
    // (SCR_ADD_TO_GROUP) happens regardless.
    if(!prepared[pending] && screens[pending].prepare) { screens[pending].prepare(); }
    prepared[pending] = true;
    if(screens[pending].init) { screens[pending].init(); }
    current = pending;
    // Update clock if necessary
    hal_rtc_datetime_t dt;
    if (hal_rtc_get(&dt)) {
      screen_manager_update_datetime(&dt);
    }
    lv_unlock();

    // Free the screen we just left once any slide/fade animation it might
    // still be part of has had time to finish (see SCR_DESTROY_DELAY_MS).
    // The timer re-checks it's still not the active/target screen before
    // actually deleting it, in case the user navigated straight back.
    if (screens[leaving].destroy) {
      lv_timer_t *t = lv_timer_create(destroy_timer_cb, SCR_DESTROY_DELAY_MS, (void*)(intptr_t)leaving);
      lv_timer_set_repeat_count(t, 1);
    }
  }
  lv_lock();
  if(screens[current].step) { screens[current].step(); }
  lv_unlock();
}

screen_id_t screen_manager_get_active_screen(void)
{
  for (screen_id_t id = SCREEN_BOOT; id < SCREEN_COUNT; id++) {
    if (lv_screen_active() == *(screens[id].scr)) {
      return id;
    }
  }
  return current;
}

void screen_manager_update_datetime(hal_rtc_datetime_t *dt)
{
  if(screens[current].topbar && *screens[current].topbar) {
    ui_topbar_update_datetime(*screens[current].topbar, dt);
  }
}