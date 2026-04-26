#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint8_t pin_a;
  uint8_t pin_b;
  uint8_t pin_btn;
} encoder_config_t;

void encoder_init(const encoder_config_t *config);
int32_t encoder_pop_delta(void);
bool encoder_btn_down(void);

#endif
