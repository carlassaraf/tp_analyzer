#include "screens.h"
#include "screen_update.h"
#include "lvgl_port.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "hal/hal_adc.h"

/** 
 * @struct screen_update_msg
 * @brief Wrapper struct to hold screen update information
 */
typedef struct screen_update_msg {
  screen_update_cmd_t cmd;  /**< Type of update to run */
  void *data;               /**< Necessary data (if any) */
} screen_update_msg_t;

// Internal queue for receiving and dispaching commands
static QueueHandle_t screen_update_queue = NULL;

static void screen_update_plot_data(void *data) {
  scr_plotter_update_chart((const uint16_t*)data, HAL_ADC_BUFFER_SIZE);
}

// Command handler pointer to dispatch pending updates
static void (*screen_update_handlers[])(void*) = {
  [SCREEN_PLOT_DATA] = screen_update_plot_data
};

void screen_update_init(void) {
  screen_update_queue = xQueueCreate(10, sizeof(screen_update_msg_t));
}

void screen_update_cmd_push(screen_update_cmd_t cmd, void *data) {
  screen_update_msg_t msg = { .cmd = cmd, .data = data };
  if (screen_update_queue != NULL) {
    xQueueSend(screen_update_queue, &msg, 0);
  }
}

void screen_update(void) {
  screen_update_msg_t msg;
  if (xQueueReceive(screen_update_queue, &msg, 0)) {
    lv_lock();
    screen_update_handlers[msg.cmd](msg.data);
    lv_unlock();
  }
}
