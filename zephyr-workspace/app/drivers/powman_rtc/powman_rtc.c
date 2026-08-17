/*
 * Copyright (c) 2026 Fabrizio Carlassara
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT raspberrypi_pico_powman_rtc

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <pico/aon_timer.h>

static int powman_rtc_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	if (!aon_timer_is_running()) {
		/* First ever boot */
		struct tm default_tm = {0};
    default_tm.tm_mday = 1;
		aon_timer_start_calendar(&default_tm);
	}
	return 0;
}

static int powman_rtc_set_time(const struct device *dev, const struct rtc_time *timeptr)
{
	ARG_UNUSED(dev);
	return aon_timer_set_time_calendar((const struct tm *)timeptr) ? 0 : -EIO;
}

static int powman_rtc_get_time(const struct device *dev, struct rtc_time *timeptr)
{
	ARG_UNUSED(dev);
	return aon_timer_get_time_calendar((struct tm *)timeptr) ? 0 : -EIO;
}

// Driver API wired with custom functions
static DEVICE_API(rtc, powman_rtc_driver_api) = {
	.set_time = powman_rtc_set_time,
	.get_time = powman_rtc_get_time,
};

DEVICE_DT_INST_DEFINE(0, &powman_rtc_init, NULL, NULL, NULL,
          POST_KERNEL, CONFIG_RTC_INIT_PRIORITY,
          &powman_rtc_driver_api);