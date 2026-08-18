#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>

#include "app.h"
#include "adc_stream.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// #define DISPLAY_WIDTH  DT_PROP(DT_CHOSEN(zephyr_display), width)
// #define DISPLAY_HEIGHT DT_PROP(DT_CHOSEN(zephyr_display), height)

/* Rows written per display_write() call — keeps the static buffer small
 * while still writing in reasonably-sized chunks (LVGL's eventual partial
 * render buffer will look similar in spirit).
 */
// #define CHUNK_LINES 20

// static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
// static const struct device *adc = DEVICE_DT_GET(DT_NODELABEL(adc_stream));


int main(void)
{
	// if (!device_is_ready(display_dev)) {
	// 	LOG_ERR("Display device not ready");
	// 	return 0;
	// }

	// struct display_capabilities caps;

	// display_get_capabilities(display_dev, &caps);
	// LOG_INF("Display %ux%u, pixel format %u", caps.x_resolution, caps.y_resolution,
	// 	caps.current_pixel_format);

	/* Panel starts blanked (ili9xxx_init() leaves blanking on) — this is
	 * the point LVGL's flush would normally call it too, once per frame.
	 */
	// display_blanking_off(display_dev);

	// LOG_INF("Static pattern test done");

	if (!app_run()) {
		LOG_ERR("App failed to run");
		while(1) { k_msleep(1000); }
	}

	while (1) {
		k_msleep(1000);
	}

	return 0;
}
