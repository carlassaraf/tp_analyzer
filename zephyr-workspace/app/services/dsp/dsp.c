#include "dsp.h"

arm_status dsp_fft_run(arm_rfft_fast_instance_f32 *instance, float32_t *input, float32_t *output) {
  // Output array for RFFT has to be of CONFIG_ADC_SAMPLES samples
  static float32_t tmp[CONFIG_ADC_SAMPLES];
  arm_rfft_fast_f32(instance, input, tmp, FFT_DIRECT);
  // First tmp bin is DC value
  output[0] = tmp[0];
  // Second tmp bin is Nyquist frequency value
  output[CONFIG_ADC_SAMPLES/2 - 1] = tmp[1];
  // Calculate bin magnitude from complex values
  for(uint32_t i = 1; i < CONFIG_ADC_SAMPLES / 2 - 1; i++) {
    output[i] = sqrtf(powf(tmp[2 * i], 2) + powf(tmp[2 * i + 1], 2));
  }
  // Normalize magnitudes
  for(uint32_t i = 0; i < CONFIG_ADC_SAMPLES / 2; i++) { output[i] /= CONFIG_ADC_SAMPLES; }
  return ARM_MATH_SUCCESS;
}