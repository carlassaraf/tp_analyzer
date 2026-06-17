#ifndef HAL_RTC_H
#define HAL_RTC_H

#include <stdint.h>
#include <stdbool.h>

/** @brief Datetime struct to use across application */
typedef struct { 
  uint8_t day, month, hour, min;
  uint16_t year;
} hal_rtc_datetime_t;

/**
 * @brief Gets current datetime from RTC
 * @param dt Pointer to a datetime struct
 * @return true on success
 */
bool hal_rtc_get(hal_rtc_datetime_t *dt);

/**
 * @brief Sets the current datetime to the RTC
 * @param dt Pointer to a datetime struct
 * @return true on success
 */
bool hal_rtc_set(const hal_rtc_datetime_t *dt);

#endif