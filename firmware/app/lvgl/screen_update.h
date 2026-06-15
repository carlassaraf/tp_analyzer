#ifndef SCREEN_UPDATE_H
#define SCREEN_UPDATE_H

/** 
 * @enum screen_update_cmd
 * @brief List of UI update commands to be ran from any other task
 */
typedef enum screen_update_cmd {
  SCREEN_OSC_DATA,          /**< Update screen with oscilloscope */
  SCREEN_FFT_DATA,          /**< Update screen with FFT sample data */
  SCREEN_UPDATE_CMD_COUNT,
} screen_update_cmd_t;

/** @brief Called to initialize the internal command queue */
void screen_update_init(void);
/** 
 * @brief Pushes update command to internal queue
 * @param cmd One of the screen_update_cmd_t available
 * @param data Pointer to data (if any) necessary to udpate screen
 */
void screen_update_cmd_push(screen_update_cmd_t cmd, void *data);
/**
 * @brief Resolves any pending screen update command by calling
 * the appropiate handler
 */
void screen_update(void);

#endif