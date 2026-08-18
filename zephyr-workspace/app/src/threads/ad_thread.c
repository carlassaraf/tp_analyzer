#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "lvgl/screen_update.h"
#include "ad_thread.h"
#include "adc_stream.h"

/* Custom ADC device from devicetree */
static const struct device *adc = DEVICE_DT_GET(DT_NODELABEL(adc_stream));
/* Message queue used to get data from DMA */
K_MSGQ_DEFINE(adc_msgq, sizeof(struct adc_stream_block), 2 * ADC_STREAM_MAX_CHANNELS, sizeof(void *));

/* Per-channel storage this thread owns, indexed by ADC channel number */
static struct adc_stream_block s_channel_blocks[ADC_STREAM_MAX_CHANNELS];

LOG_MODULE_REGISTER(ad_thread, LOG_LEVEL_INF);

/** @brief AD consumer thread */
void ad_thread(void *param1, void *param2, void *param3)
{
  ARG_UNUSED(param1);
  ARG_UNUSED(param2);
  ARG_UNUSED(param3);

	if (!device_is_ready(adc)) {
		LOG_ERR("ADC stream device not ready");
		return;
	}

	int err = adc_stream_start(adc, &adc_msgq);
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
		if (block.channel >= ADC_STREAM_MAX_CHANNELS) {
			LOG_WRN("Unexpected ADC channel %u", block.channel);
			continue;
		}
		// Copy into this channel's persistent slot before handing a pointer to it off to another thread
		s_channel_blocks[block.channel] = block;

		// Pass on data to screens
		screen_update_cmd_push(SCREEN_UPDATE_OSC_DATA, (void *)&s_channel_blocks[block.channel]);
		screen_update_cmd_push(SCREEN_UPDATE_FFT_DATA,  (void *)&s_channel_blocks[block.channel]);
  }
}
