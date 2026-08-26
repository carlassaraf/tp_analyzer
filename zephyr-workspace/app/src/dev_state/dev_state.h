#ifndef DEV_STATE_H
#define DEV_STATE_H

#include <zephyr/kernel.h>

typedef struct dev_state {
  uint32_t off_screen_timeout_ms; /**< Time in ms for the screen to turn off when there is no activity */
  uint32_t screen_timeout_ms;     /**< Time in ms for the screen to return to the menu screen */
} dev_state_t;

// Public functions

bool dev_state_init(void);
bool dev_state_get(dev_state_t *dev_state);
bool dev_state_set_off_timeout(uint32_t timeout);
bool dev_state_set_screen_timeout(uint32_t timeout);

#endif