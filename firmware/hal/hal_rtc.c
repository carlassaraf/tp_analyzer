#include "hal_rtc.h"
#include "pico/aon_timer.h"
#include <time.h>

bool hal_rtc_get(hal_rtc_datetime_t *dt)
{
  if (!aon_timer_is_running()) { return false; }
  struct tm tm = {0};
  if (!aon_timer_get_time_calendar(&tm)) { return false; }
  dt->day   = (uint8_t)tm.tm_mday;
  dt->month = (uint8_t)(tm.tm_mon);
  dt->year  = (uint16_t)(tm.tm_year + 1900);
  dt->hour  = (uint8_t)tm.tm_hour;
  dt->min   = (uint8_t)tm.tm_min;
  return true;
}

bool hal_rtc_set(const hal_rtc_datetime_t *dt)
{
  struct tm tm = {0};
  tm.tm_mday = dt->day;
  tm.tm_mon  = dt->month - 1;
  tm.tm_year = dt->year - 1900;
  tm.tm_hour = dt->hour;
  tm.tm_min  = dt->min;
  tm.tm_sec  = 0;
  return aon_timer_start_calendar(&tm);
}