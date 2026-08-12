/*
 * Copyright (c) 2026 Fabrizio Carlassara
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * Continuous free-running ADC + DMA acquisition for the RP2350.
 *
 * Shape mirrors two references — see ZEPHYR_MIGRATION.md Phase 1b:
 *  - main:firmware/hal/hal_adc.c (the FreeRTOS original): same free-run +
 *    ping-pong idea, same Pico-SDK adc_fifo_setup()/adc_set_clkdiv() calls.
 *  - drivers/adc/adc_stm32.c's CONFIG_ADC_STM32_DMA path (in-tree
 *    precedent for "this MCU's ADC subsystem driver has no DMA-streaming
 *    API of its own, drive the DMA side through zephyr/drivers/dma.h
 *    against a real DMA controller device instead of hand-rolling DMA
 *    register access" — see adc_stm32_dma_start() there).
 *
 * STATUS: untested skeleton laid down for Phase 1b hardware bring-up, not
 * yet validated on the board. Known TODOs are marked inline; the biggest
 * unverified piece is the ADC pin's pinctrl state (see the board overlay).
 */

#define DT_DRV_COMPAT raspberrypi_pico_adc_stream

#include <zephyr/device.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/drivers/reset.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <hardware/adc.h>

#include "adc_stream.h"

LOG_MODULE_REGISTER(adc_stream_rpi_pico, CONFIG_ADC_STREAM_RPI_PICO_LOG_LEVEL);

#define SAMPLE_SIZE_BYTES sizeof(uint16_t)
#define BLOCK_SIZE_BYTES  (CONFIG_ADC_STREAM_BUFFER_SIZE * SAMPLE_SIZE_BYTES)

struct adc_stream_config {
	const struct pinctrl_dev_config *pcfg;
	const struct device *clk_dev;
	clock_control_subsys_t clk_id;
	const struct reset_dt_spec reset;
	const struct device *dma_dev;
	uint32_t dma_channel;
	uint32_t dma_slot;
};

struct adc_stream_data {
	struct dma_config dma_cfg;
	struct dma_block_config dma_blk;
	uint16_t buf_a[CONFIG_ADC_STREAM_BUFFER_SIZE];
	uint16_t buf_b[CONFIG_ADC_STREAM_BUFFER_SIZE];
	uint16_t *active;	/* half the DMA channel is currently filling */
	struct k_msgq *msgq;	/* NULL when stopped */
};

static inline uint16_t *other_buf(struct adc_stream_data *data, uint16_t *buf)
{
	return (buf == data->buf_a) ? data->buf_b : data->buf_a;
}

/*
 * Fires from dma_rpi_pico's ISR context (see .dma_callback in dma_config
 * below) — keep this short, no blocking calls.
 */
static void adc_stream_dma_cb(const struct device *dma_dev, void *user_data,
			      uint32_t channel, int status)
{
	const struct device *dev = user_data;
	const struct adc_stream_config *config = dev->config;
	struct adc_stream_data *data = dev->data;
	uint16_t *completed;
	struct adc_stream_block block;
	int err;

	ARG_UNUSED(dma_dev);

	if (status < 0) {
		LOG_ERR("DMA error %d on channel %u", status, channel);
		return;
	}

	completed = data->active;
	data->active = other_buf(data, completed);

	/*
	 * Re-arm the just-vacated half before touching the completed one —
	 * mirrors hal_adc.c's dma_isr(), keeps the ADC FIFO draining with no
	 * gap between blocks. dma_rpi_pico implements .reload (see
	 * drivers/dma/dma_rpi_pico.c), so this is a register poke, not a
	 * full re-negotiation like dma_config().
	 */
	err = dma_reload(config->dma_dev, config->dma_channel,
			 (uint32_t)&adc_hw->fifo, (uint32_t)data->active,
			 BLOCK_SIZE_BYTES);
	if (err) {
		LOG_ERR("dma_reload failed: %d", err);
		return;
	}
	err = dma_start(config->dma_dev, config->dma_channel);
	if (err) {
		LOG_ERR("dma_start failed: %d", err);
		return;
	}

	if (data->msgq == NULL) {
		return;
	}

	block.samples = completed;
	block.count = CONFIG_ADC_STREAM_BUFFER_SIZE;

	/*
	 * K_NO_WAIT: a full msgq means the consumer fell behind. Drop and
	 * log rather than block the DMA-completion path.
	 * TODO(Phase 2/4): surface a drop counter to ad_task once it exists,
	 * instead of just a rate-limited log line.
	 */
	err = k_msgq_put(data->msgq, &block, K_NO_WAIT);
	if (err) {
		LOG_WRN("msgq full, dropped a block");
	}
}

int adc_stream_start(const struct device *dev, struct k_msgq *msgq)
{
	const struct adc_stream_config *config = dev->config;
	struct adc_stream_data *data = dev->data;
	int err;

	if (msgq->msg_size != sizeof(struct adc_stream_block)) {
		return -EINVAL;
	}

	data->msgq = msgq;
	data->active = data->buf_a;

	data->dma_blk = (struct dma_block_config){
		.source_address = (uint32_t)&adc_hw->fifo,
		.dest_address = (uint32_t)data->active,
		.block_size = BLOCK_SIZE_BYTES,
	};

	data->dma_cfg = (struct dma_config){
		.dma_slot = config->dma_slot,
		.channel_direction = PERIPHERAL_TO_MEMORY,
		.source_data_size = SAMPLE_SIZE_BYTES,
		.dest_data_size = SAMPLE_SIZE_BYTES,
		.source_burst_length = 1,
		.dest_burst_length = 1,
		.block_count = 1,
		.head_block = &data->dma_blk,
		.dma_callback = adc_stream_dma_cb,
		.user_data = (void *)dev,
		.complete_callback_en = 0,	/* callback at block completion only */
	};

	err = dma_config(config->dma_dev, config->dma_channel, &data->dma_cfg);
	if (err) {
		LOG_ERR("dma_config failed: %d", err);
		return err;
	}

	/*
	 * adc_fifo_setup(en, dreq_en, dreq_thresh, err_in_fifo, byte_shift):
	 * DREQ asserts once per FIFO entry (threshold 1), matching
	 * hal_adc.c. adc_set_clkdiv() mirrors board_config.h's
	 * ADC_SAMPLE_RATE derivation (48 MHz / (div + 1)).
	 * TODO: confirm on a scope that the resulting rate matches
	 * CONFIG_ADC_STREAM_SAMPLE_RATE_HZ closely enough for the FFT bin
	 * math in services/dsp to hold.
	 */
	adc_select_input(0);	/* TODO: drive from devicetree/Kconfig, not hardcoded — see below */
	adc_fifo_setup(true, true, 1, false, false);
	adc_set_clkdiv(48e6f / (float)CONFIG_ADC_STREAM_SAMPLE_RATE_HZ - 1.0f);

	err = dma_start(config->dma_dev, config->dma_channel);
	if (err) {
		LOG_ERR("dma_start failed: %d", err);
		return err;
	}

	adc_run(true);

	return 0;
}

int adc_stream_stop(const struct device *dev)
{
	const struct adc_stream_config *config = dev->config;
	struct adc_stream_data *data = dev->data;

	adc_run(false);
	dma_stop(config->dma_dev, config->dma_channel);
	adc_fifo_drain();
	data->msgq = NULL;

	return 0;
}

static int adc_stream_init(const struct device *dev)
{
	const struct adc_stream_config *config = dev->config;
	int err;

	err = pinctrl_apply_state(config->pcfg, PINCTRL_STATE_DEFAULT);
	if (err) {
		return err;
	}

	err = clock_control_on(config->clk_dev, config->clk_id);
	if (err) {
		return err;
	}

	err = reset_line_toggle_dt(&config->reset);
	if (err) {
		return err;
	}

	if (!device_is_ready(config->dma_dev)) {
		LOG_ERR("%s not ready", config->dma_dev->name);
		return -ENODEV;
	}

	/*
	 * Pico-SDK adc_init(): enables the ADC clock domain and brings the
	 * block out of reset internally too — redundant with the
	 * clock_control_on()/reset_line_toggle_dt() above in principle, but
	 * adc_rpi_pico.c's own init doesn't call it and instead open-codes
	 * adc_enable(); TODO: during bring-up, check with a debugger whether
	 * calling both here is actually necessary or whether it's one or
	 * the other.
	 */
	adc_init();

	return 0;
}

#define ADC_STREAM_RPI_PICO_INIT(idx)                                                        \
	PINCTRL_DT_INST_DEFINE(idx);                                                          \
	static const struct adc_stream_config adc_stream_config_##idx = {                    \
		.pcfg = PINCTRL_DT_INST_DEV_CONFIG_GET(idx),                                  \
		.clk_dev = DEVICE_DT_GET(DT_INST_CLOCKS_CTLR(idx)),                           \
		.clk_id = (clock_control_subsys_t)DT_INST_PHA_BY_IDX(idx, clocks, 0, clk_id), \
		.reset = RESET_DT_SPEC_INST_GET(idx),                                         \
		.dma_dev = DEVICE_DT_GET(DT_INST_DMAS_CTLR_BY_IDX(idx, 0)),                   \
		.dma_channel = DT_INST_DMAS_CELL_BY_IDX(idx, 0, channel),                     \
		.dma_slot = DT_INST_DMAS_CELL_BY_IDX(idx, 0, slot),                           \
	};                                                                                     \
	static struct adc_stream_data adc_stream_data_##idx;                                 \
	DEVICE_DT_INST_DEFINE(idx, adc_stream_init, NULL, &adc_stream_data_##idx,             \
			      &adc_stream_config_##idx, POST_KERNEL,                          \
			      CONFIG_ADC_STREAM_RPI_PICO_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(ADC_STREAM_RPI_PICO_INIT)
