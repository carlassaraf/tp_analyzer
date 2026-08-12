#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* RGB565: 5 bits red, 6 bits green, 5 bits blue, packed into a native uint16_t. */
#define RGB565(r, g, b) \
	(uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define DISPLAY_WIDTH  DT_PROP(DT_CHOSEN(zephyr_display), width)
#define DISPLAY_HEIGHT DT_PROP(DT_CHOSEN(zephyr_display), height)

/* Rows written per display_write() call — keeps the static buffer small
 * while still writing in reasonably-sized chunks (LVGL's eventual partial
 * render buffer will look similar in spirit).
 */
#define CHUNK_LINES 20

static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

static uint16_t line_buf[DISPLAY_WIDTH * CHUNK_LINES];

struct color_band {
	uint16_t color;
	const char *name;
};

/* Five solid bands top-to-bottom: primaries + white/black. Good first check —
 * wrong colors or a shifted/mirrored layout point straight at a devicetree
 * (polarity, rotation, pixel-format) or wiring problem rather than needing to
 * debug LVGL on top of an unproven display path.
 */
static const struct color_band bands[] = {
	{ RGB565(255, 0, 0),     "red" },
	{ RGB565(0, 255, 0),     "green" },
	{ RGB565(0, 0, 255),     "blue" },
	{ RGB565(255, 255, 255), "white" },
	{ RGB565(0, 0, 0),       "black" },
};

/* Bottom strip: a left-to-right gradient, to catch subtler issues (banding,
 * wrong bit packing) that solid colors alone wouldn't reveal.
 */
static void fill_gradient_row(uint16_t *row)
{
	for (int x = 0; x < DISPLAY_WIDTH; x++) {
		uint8_t level = (uint8_t)((x * 255) / (DISPLAY_WIDTH - 1));

		row[x] = RGB565(level, level, level);
	}
}

static int write_band(uint16_t y, uint16_t height, uint16_t color)
{
	struct display_buffer_descriptor desc = {
		.width = DISPLAY_WIDTH,
		.pitch = DISPLAY_WIDTH,
	};
	int ret;

	for (int i = 0; i < DISPLAY_WIDTH * CHUNK_LINES; i++) {
		line_buf[i] = color;
	}

	for (uint16_t row = 0; row < height; row += CHUNK_LINES) {
		uint16_t rows_this_write = MIN(CHUNK_LINES, height - row);

		desc.height = rows_this_write;
		desc.buf_size = DISPLAY_WIDTH * rows_this_write * sizeof(uint16_t);

		ret = display_write(display_dev, 0, y + row, &desc, line_buf);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

static int write_gradient_strip(uint16_t y, uint16_t height)
{
	struct display_buffer_descriptor desc = {
		.width = DISPLAY_WIDTH,
		.pitch = DISPLAY_WIDTH,
		.height = 1,
		.buf_size = DISPLAY_WIDTH * sizeof(uint16_t),
	};

	fill_gradient_row(line_buf);

	for (uint16_t row = 0; row < height; row++) {
		int ret = display_write(display_dev, 0, y + row, &desc, line_buf);

		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

int main(void)
{
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device not ready");
		return 0;
	}

	struct display_capabilities caps;

	display_get_capabilities(display_dev, &caps);
	LOG_INF("Display %ux%u, pixel format %u", caps.x_resolution, caps.y_resolution,
		caps.current_pixel_format);

	/* Reserve the last band's worth of rows for the gradient strip instead
	 * of a solid color.
	 */
	uint16_t band_height = DISPLAY_HEIGHT / (ARRAY_SIZE(bands) + 1);
	uint16_t y = 0;

	for (size_t i = 0; i < ARRAY_SIZE(bands); i++) {
		int ret = write_band(y, band_height, bands[i].color);

		if (ret < 0) {
			LOG_ERR("Failed to write %s band: %d", bands[i].name, ret);
			return 0;
		}
		LOG_INF("Wrote %s band at y=%u", bands[i].name, y);
		y += band_height;
	}

	if (write_gradient_strip(y, DISPLAY_HEIGHT - y) < 0) {
		LOG_ERR("Failed to write gradient strip");
		return 0;
	}
	LOG_INF("Wrote gradient strip at y=%u", y);

	/* Panel starts blanked (ili9xxx_init() leaves blanking on) — this is
	 * the point LVGL's flush would normally call it too, once per frame.
	 */
	display_blanking_off(display_dev);

	LOG_INF("Static pattern test done");

	while (1) {
		k_msleep(1000);
	}

	return 0;
}
