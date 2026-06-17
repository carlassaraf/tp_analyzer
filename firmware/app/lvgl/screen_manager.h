#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "screens.h"

/** @brief Initializes initial application screen */
void screen_manager_init(void);

/**
 * @brief Registers next go to screen
 * @param id Screen to transition to next
 */
void screen_manager_go_to(screen_id_t id);

/** @brief Run current screen step function */
void screen_manager_step(void);

/**
 * @brief Returns current active screen
 * @return ID of active screen
 */
screen_id_t screen_manager_get_active_screen(void);

/**
 * @brief Updates datetime on current screen if available
 * @param dt Pointer to datetime
 */
void screen_manager_update_datetime(hal_rtc_datetime_t *dt);

#endif
