#ifndef DEV_STATE_H
#define DEV_STATE_H

#include <zephyr/kernel.h>

typedef struct dev_state {
  uint32_t off_screen_timeout_ms; /**< Time in ms for the screen to turn off when there is no activity */
  uint32_t screen_timeout_ms;     /**< Time in ms for the screen to return to the menu screen */
} dev_state_t;

/**
 * @enum dev_state_timeout_evt
 * @brief Identifies which of the two timeouts just expired
 */
typedef enum dev_state_timeout_evt {
  DEV_STATE_TIMEOUT_SCREEN,   /**< screen_timeout_ms elapsed with no activity */
  DEV_STATE_TIMEOUT_PWR_OFF,  /**< off_screen_timeout_ms elapsed with no activity */
} dev_state_timeout_evt_t;

/**
 * @brief Callback invoked when a timeout expires.
 * @note Runs in k_timer expiry context (interrupt-level) — must not block
 * or call into LVGL directly. Hop through a queue (e.g. screen_update_cmd_push)
 * if the reaction needs to do more than a plain variable write.
 * @param evt Which timeout fired
 */
typedef void (*dev_state_timeout_cb_t)(dev_state_timeout_evt_t evt);

// Public functions

/**
 * @brief Initializes the mutex, loads state (defaults for now, flash later)
 * and arms both timeout timers.
 * @note Call dev_state_set_timeout_cb() before this, so a very short
 * configured timeout can't expire before a callback is registered.
 */
bool dev_state_init(void);

/**
 * @brief Registers the function called when a timeout expires
 * @param cb Callback to invoke, or NULL to clear it
 */
void dev_state_set_timeout_cb(dev_state_timeout_cb_t cb);

/** @brief Copies out the current device state */
bool dev_state_get(dev_state_t *dev_state);

/**
 * @brief Updates the power-off timeout and restarts its timer
 * @param timeout New timeout in ms, 0 means never expire
 */
bool dev_state_set_off_timeout(uint32_t timeout);

/**
 * @brief Updates the screen (back-to-menu) timeout and restarts its timer
 * @param timeout New timeout in ms, 0 means never expire
 */
bool dev_state_set_screen_timeout(uint32_t timeout);

/** @brief Restarts both timeout timers from their currently stored durations */
void dev_state_kick_activity(void);

#endif
