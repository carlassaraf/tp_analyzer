#include <zephyr/kernel.h>
#include "ad_thread.h"
// #include "hal/hal_adc.h"
// #include "lvgl/screen_update.h"
// #include "board_config.h"

/** 
 * @brief Called by the DMA ISR in the ADC HAL
 * @param buf Pointer to the DMA buffer with the sampled data
 */
static void adc_ready_cb(const uint16_t *buf)
{
  // BaseType_t woken = pdFALSE;
  // xTaskNotifyFromISR(s_task_handle, (uint32_t)buf, eSetValueWithOverwrite, &woken);
  // portYIELD_FROM_ISR(woken);
}

void ad_thread(void *param1, void *param2, void *param3)
{
  ARG_UNUSED(param1);
  ARG_UNUSED(param2);
  ARG_UNUSED(param3);
  // s_task_handle = xTaskGetCurrentTaskHandle();

  // hal_adc_init(ADC_SAMPLE_RATE, ADC_GPIO, ADC_CHANNEL);
  // hal_adc_set_ready_cb(adc_ready_cb);
  // hal_adc_start();

  while (1) {
    // uint32_t data;
    // xTaskNotifyWait(0, ULONG_MAX, &data, portMAX_DELAY);
    // screen_update_cmd_push(SCREEN_UPDATE_OSC_DATA, (void*)data);
    // screen_update_cmd_push(SCREEN_UPDATE_FFT_DATA,  (void*)data);
  }
}
