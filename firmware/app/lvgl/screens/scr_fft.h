#ifndef SCR_FFT_H_
#define SCR_FFT_H_

#include <stdint.h>

void scr_fft_prepare(void);
void scr_fft_init(void);
void scr_fft_deinit(void);
void scr_fft_step(void);

#endif