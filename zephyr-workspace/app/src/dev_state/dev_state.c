#include "dev_state.h"

#include <zephyr/input/input.h>

// Timeout timers
static void screen_timeout_expiry(struct k_timer *timer_id);
static void off_timeout_expiry(struct k_timer *timer_id);

static K_TIMER_DEFINE(screen_timeout_timer, screen_timeout_expiry, NULL);
static K_TIMER_DEFINE(off_timeout_timer, off_timeout_expiry, NULL);
static K_MUTEX_DEFINE(s_mux);

// Consistent device state in runtime
static dev_state_t s_device_state = {0};

// Registered by whoever decides what a timeout means (the UI layer)
static dev_state_timeout_cb_t s_timeout_cb = NULL;

// Converts a stored ms value to a relative k_timeout_t, 0 meaning "never expire"
static inline k_timeout_t ms_to_timeout(uint32_t ms)
{
  return (ms == 0) ? K_FOREVER : K_MSEC(ms);
}

bool dev_state_init(void)
{
  // Initialize mutex
  k_mutex_init(&s_mux);
  /** @todo Load from Flash */

  // Load default state
  k_mutex_lock(&s_mux, K_FOREVER);
  s_device_state.screen_timeout_ms = 30000;
  s_device_state.off_screen_timeout_ms = 60000;
  k_mutex_unlock(&s_mux);

  // Arm both timers with the loaded values
  dev_state_kick_activity();

  return true;
}

void dev_state_set_timeout_cb(dev_state_timeout_cb_t cb)
{
  s_timeout_cb = cb;
}

bool dev_state_get(dev_state_t *dev_state)
{
  k_mutex_lock(&s_mux, K_FOREVER);
  *dev_state = s_device_state;
  k_mutex_unlock(&s_mux);
  return true;
}

bool dev_state_set_off_timeout(uint32_t timeout)
{
  k_mutex_lock(&s_mux, K_FOREVER);
  s_device_state.off_screen_timeout_ms = timeout;
  /** @todo Persist to Flash */
  k_mutex_unlock(&s_mux);

  k_timer_start(&off_timeout_timer, ms_to_timeout(timeout), K_NO_WAIT);
  return true;
}

bool dev_state_set_screen_timeout(uint32_t timeout)
{
  k_mutex_lock(&s_mux, K_FOREVER);
  s_device_state.screen_timeout_ms = timeout;
  /** @todo Persist to Flash */
  k_mutex_unlock(&s_mux);

  k_timer_start(&screen_timeout_timer, ms_to_timeout(timeout), K_NO_WAIT);
  return true;
}

void dev_state_kick_activity(void)
{
  k_mutex_lock(&s_mux, K_FOREVER);
  uint32_t screen_ms = s_device_state.screen_timeout_ms;
  uint32_t off_ms = s_device_state.off_screen_timeout_ms;
  k_mutex_unlock(&s_mux);

  // One-shot: no period, they get manually restarted on the next kick
  k_timer_start(&screen_timeout_timer, ms_to_timeout(screen_ms), K_NO_WAIT);
  k_timer_start(&off_timeout_timer, ms_to_timeout(off_ms), K_NO_WAIT);
}

/**
 * @brief Called when it's time to go back to the menu screen
 */
static void screen_timeout_expiry(struct k_timer *timer_id)
{
  if (s_timeout_cb) { s_timeout_cb(DEV_STATE_TIMEOUT_SCREEN); }
}

/**
 * @brief Called when it's time to power off the screen
 */
static void off_timeout_expiry(struct k_timer *timer_id)
{
  if (s_timeout_cb) { s_timeout_cb(DEV_STATE_TIMEOUT_PWR_OFF); }
}

/**
 * @brief Fires for every raw input event reported by any input device
 * (encoder rotation and its button, currently the only ones registered).
 * Runs in the input subsystem's own thread (CONFIG_INPUT_MODE_THREAD is
 * the project default), so blocking on the mutex here is safe.
 */
static void encoder_activity_cb(struct input_event *evt, void *user_data)
{
  dev_state_kick_activity();
}
INPUT_CALLBACK_DEFINE(NULL, encoder_activity_cb, NULL);
