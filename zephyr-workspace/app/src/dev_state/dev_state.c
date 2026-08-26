#include "dev_state.h"

static K_MUTEX_DEFINE(s_mux);

// Consistent device state in runtime
static dev_state_t s_device_state = {0};

bool dev_state_init(void)
{
  // Initialize mutex
  k_mutex_init(&s_mux);
  /** @todo Load from Flash */

  // Load default state
  k_mutex_lock(&s_mux, K_FOREVER);
  s_device_state.off_screen_timeout_ms = 60000;
  s_device_state.screen_timeout_ms = 30000;
  k_mutex_unlock(&s_mux);

  return true;
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
  k_mutex_unlock(&s_mux);
  return true;
}

bool dev_state_set_screen_timeout(uint32_t timeout)
{
  k_mutex_lock(&s_mux, K_FOREVER);
  s_device_state.screen_timeout_ms = timeout;
  k_mutex_unlock(&s_mux);
  return true;
}