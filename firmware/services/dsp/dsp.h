#ifndef _DSP_H_
#define _DSP_H_

#include <math.h>
#include "arm_math.h"
#include <stdlib.h>

// Number of samples — must match HAL_ADC_BUFFER_SIZE
#define N   2048
// Sampling frequency in Hz — must match ADC_SAMPLE_RATE in board_config.h
#define FS  10000.0f

// Derived display constants (10 harmonics of 50 Hz visible at once)
#define VISIBLE_BINS  ((uint16_t)((10U * 50U * N) / 10000U))  // = 102 bins
#define SCROLL_BINS   ((uint16_t)((50U * N) / 10000U))         // = 10 bins per step

/**
 * @brief FFT directions
 */
typedef enum {
  FFT_DIRECT = 0,
  FFT_INVERSE = 1
} fft_direction_t;

/**
 * @brief Handles CMSIS DSP RFFT initialization
 * @param data pointer to FFT struct
 * @return initialization status ARM_MATH_SUCCESS if was possible
 */
static inline arm_status dsp_fft_init(arm_rfft_fast_instance_f32 *instance) {
#if(N == 256)
  return arm_rfft_fast_init_256_f32(instance);
#elif(N == 512)
  return arm_rfft_fast_init_512_f32(instance);
#elif(N == 1024)
  return arm_rfft_fast_init_1024_f32(instance);
#elif(N == 2048)
  return arm_rfft_fast_init_2048_f32(instance);
#elif(N == 4096)
  return arm_rfft_fast_init_4096_f32(instance);
#endif
}

/**
 * @brief Performs a RFFT and returns normalized bin values
 * @param data pointer to FFT struct
 * @return ARM_MATH_SUCCESS if was possible
 */
arm_status dsp_fft_run(arm_rfft_fast_instance_f32 *instance, float32_t *input, float32_t *output);

/**
 * @brief Creates a frequencies array
 * @param bins destination array of n elements
 * @param fs sampling frequency in Hz
 * @param n number of samples
 * @return ARM_MATH_SUCCESS if was possible
 */
static inline arm_status dsp_get_frequency_bins(float32_t *bins, float32_t fs, uint32_t n) {

  const float32_t inc = fs / n;
  for(uint32_t i = 0; i < n/2; i++) { bins[i] = i * inc; }
  return ARM_MATH_SUCCESS;
}

// Prototypes

#endif