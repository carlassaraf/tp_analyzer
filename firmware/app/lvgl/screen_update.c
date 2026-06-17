#include "screens.h"
#include "screen_update.h"
#include "screen_manager.h"
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

// Private command handlers
static void screen_update_plot_data(void *data);
static void screen_update_fft_data(void *data);
static void screen_update_datetime(void *data);

// Command handler pointer to dispatch pending updates
static void (*screen_update_handlers[])(void*) = {
  [SCREEN_UPDATE_OSC_DATA]  = screen_update_plot_data,
  [SCREEN_UPDATE_FFT_DATA]  = screen_update_fft_data,
  [SCREEN_UPDATE_DATETIME]  = screen_update_datetime,
};

// Extra private helper prototypes
static void screen_update_helper_peak_value(void *data);
static void screen_update_helper_rms_value(void *data);

// Public functions

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

// Private functions and helpers

static void screen_update_plot_data(void *data)
{
  // Cant send data if screen is not active
  if(screen_manager_get_active_screen() != SCREEN_OSC) { return; }
  scr_oscilloscope_update_chart((const uint16_t*)data, HAL_ADC_BUFFER_SIZE);
  // Get max raw value from array for peak and rms
  const q15_t *samples = (const q15_t *)data;
  static float32_t input[N] = {0};
  float32_t peak = 0;
  uint32_t idx = 0;
  float32_t sqrt2 = 0.0;

  // Convert 12-bit ADC samples to centered float [-1, 1]
  for (uint32_t i = 0; i < N; i++) {
    input[i] = ((float32_t)samples[i] - 2048.0f) / 2048.0f;
  }

  arm_sqrt_f32(2, &sqrt2);
  arm_absmax_f32(input, N, &peak, &idx);
  
  scr_oscilloscope_update_peak(peak);
  scr_oscilloscope_update_rms(peak / sqrt2);
  // Get max bin from FFT for main frequency
  static arm_rfft_fast_instance_f32 fft_inst;
  static float32_t fft_out[N / 2];
  static bool initialized = false;
  uint32_t bin_max = 0;

  if (!initialized) {
    dsp_fft_init(&fft_inst);
    initialized = true;
  }
  dsp_fft_run(&fft_inst, input, fft_out);
  arm_max_f32(fft_out, N, &peak, &bin_max);
  scr_oscilloscope_update_frequency(bin_max * FS / N);
}

static void screen_update_fft_data(void *data)
{
  // Cant send data if screen is not active
  if(screen_manager_get_active_screen() != SCREEN_FFT) { return; }
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

static void screen_update_datetime(void *data)
{
  hal_rtc_datetime_t *dt = (hal_rtc_datetime_t*)data;
  // Update topbar datetime
  screen_manager_update_datetime(dt);
  // Only update if it's not active to load them on next prepare call
  if(screen_manager_get_active_screen() != SCREEN_DATETIME) {
    scr_datetime_update_datetime(dt);
  }
}

// Helpers

static void screen_update_helper_peak_value(void *data)
{
  const q15_t *samples = (const q15_t *)data;
  q15_t res = 0;
  uint32_t idx = 0;
  arm_max_q15(samples, N, &res, &idx);
  scr_oscilloscope_update_peak((uint16_t)res);
}

static void screen_update_helper_rms_value(void *data)
{
  const q15_t *samples = (const q15_t *)data;
  q15_t res = 0;
  uint32_t idx = 0;
  arm_max_q15(samples, N, &res, &idx);
  scr_oscilloscope_update_peak((uint16_t)res);
}