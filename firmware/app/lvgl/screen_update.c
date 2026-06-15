#include "screens.h"
#include "screen_update.h"
#include "lvgl_port.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "hal/hal_adc.h"
#include "dsp.h"
#include <stdbool.h>

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
  scr_oscilloscope_update_chart((const uint16_t*)data, HAL_ADC_BUFFER_SIZE);
}

static void screen_update_fft_data(void *data) {
  const uint16_t *samples = (const uint16_t *)data;
  static arm_rfft_fast_instance_f32 fft_inst;
  static float32_t input[N];
  static float32_t fft_out[N / 2];
  static bool initialized = false;

  if (!initialized) {
    dsp_fft_init(&fft_inst);
    initialized = true;
  }
  // Hanning window to normalize sample start and end
  static float32_t hanning_window[N] = {0};
  const float hanning_gain = 0.5;
  arm_hanning_f32(hanning_window, N);

  // Convert 12-bit ADC samples to centered float [-1, 1]
  for (uint32_t i = 0; i < N; i++) {
    input[i] = ((float32_t)samples[i] - 2048.0f) / 2048.0f;
  }
  // Apply hanning window
  arm_mult_f32(input, hanning_window, input, N);
  for (uint32_t i = 0; i < N; i++) { input[i] /= hanning_gain; }
  dsp_fft_run(&fft_inst, input, fft_out);
  scr_fft_update_chart(fft_out, N / 2);
}

// Command handler pointer to dispatch pending updates
static void (*screen_update_handlers[])(void*) = {
  [SCREEN_OSC_DATA]   = screen_update_plot_data,
  [SCREEN_FFT_DATA]   = screen_update_fft_data,
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
