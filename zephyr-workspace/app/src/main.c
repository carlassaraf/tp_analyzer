#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>

#include "adc_stream.h"

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

/* Number of full-screen flushes to average over. */
#define FPS_TEST_FRAMES 20

/* Two alternating colors so a slow rate is visible on the panel too, not
 * just in the log.
 */
static const uint16_t fps_test_colors[] = {
	RGB565(255, 0, 0),
	RGB565(0, 255, 0),
};

static void run_fps_test(void)
{
	struct display_buffer_descriptor desc = {
		.width = DISPLAY_WIDTH,
		.pitch = DISPLAY_WIDTH,
	};
	uint32_t start_ms = k_uptime_get_32();

	for (int frame = 0; frame < FPS_TEST_FRAMES; frame++) {
		uint16_t color = fps_test_colors[frame % ARRAY_SIZE(fps_test_colors)];

		for (int i = 0; i < DISPLAY_WIDTH * CHUNK_LINES; i++) {
			line_buf[i] = color;
		}

		for (uint16_t row = 0; row < DISPLAY_HEIGHT; row += CHUNK_LINES) {
			uint16_t rows_this_write = MIN(CHUNK_LINES, DISPLAY_HEIGHT - row);

			desc.height = rows_this_write;
			desc.buf_size = DISPLAY_WIDTH * rows_this_write * sizeof(uint16_t);

			display_write(display_dev, 0, row, &desc, line_buf);
		}
	}

	uint32_t elapsed_ms = k_uptime_get_32() - start_ms;
	uint32_t avg_frame_ms = elapsed_ms / FPS_TEST_FRAMES;
	/* fps * 10, to get one decimal place without pulling in float printf */
	uint32_t fps_x10 = (FPS_TEST_FRAMES * 10000U) / elapsed_ms;

	LOG_INF("FPS test: %d full-screen (%ux%u) flushes in %u ms -> %u ms/frame, %u.%u fps",
		FPS_TEST_FRAMES, DISPLAY_WIDTH, DISPLAY_HEIGHT, elapsed_ms, avg_frame_ms,
		fps_x10 / 10, fps_x10 % 10);
}

static const struct device *adc = DEVICE_DT_GET(DT_NODELABEL(adc_stream));

/* Depth 4: enough slack for a couple of blocks to queue up if this thread
 * gets briefly preempted, without masking a real backpressure problem — see
 * adc_stream_rpi_pico.c's k_msgq_put(K_NO_WAIT) comment on drops.
 */
K_MSGQ_DEFINE(adc_msgq, sizeof(struct adc_stream_block), 4, sizeof(void *));

/* Phase 1b hardware bring-up smoke test (see adc_stream_rpi_pico.c's STATUS
 * comment): pull blocks straight off the driver and log min/avg/max per
 * block. No display/LVGL involved yet, so a failure here points at the ADC
 * DMA path specifically, not anything downstream.
 */
static void run_adc_stream_test(void)
{
	int err;

	if (!device_is_ready(adc)) {
		LOG_ERR("ADC stream device not ready");
		return;
	}

	err = adc_stream_start(adc, &adc_msgq);
	if (err) {
		LOG_ERR("adc_stream_start failed: %d", err);
		return;
	}
	LOG_INF("ADC stream started, waiting for blocks...");

	while (1) {
		struct adc_stream_block block;

		err = k_msgq_get(&adc_msgq, &block, K_SECONDS(2));
		if (err) {
			LOG_WRN("No ADC block in 2s - check DMA wiring/pinctrl");
			continue;
		}

		uint32_t sum = 0;
		uint16_t min = UINT16_MAX;
		uint16_t max = 0;

		for (size_t i = 0; i < block.count; i++) {
			uint16_t sample = block.samples[i];

			sum += sample;
			min = MIN(min, sample);
			max = MAX(max, sample);
		}

		LOG_INF("block: %u samples, min=%u avg=%u max=%u",
			block.count, min, sum / block.count, max);
	}
}

int main(void)
{
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device not ready");
		return 0;
	}

	run_adc_stream_test();

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

	run_fps_test();

	while (1) {
		k_msleep(1000);
	}

	return 0;
}
