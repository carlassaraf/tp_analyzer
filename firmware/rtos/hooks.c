#include <stdio.h>
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#if(configCHECK_FOR_STACK_OVERFLOW > 0)
/**
 * @brief Stack overflow handler for debugging purposes
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  stdio_init_all();
  printf("ERROR: Stackoverflow en %s\n", pcTaskName);
  while(1);
}
#endif