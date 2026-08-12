/*
 * Copyright (c) 2026 Fabrizio Carlassara
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APP_DRIVERS_ADC_STREAM_H_
#define APP_DRIVERS_ADC_STREAM_H_

#include <zephyr/device.h>
#include <zephyr/kernel.h>

/**
 * @file
 * Public API for the app-local continuous ADC + DMA streaming driver.
 *
 * See ZEPHYR_MIGRATION.md, Phase 1b, for why this exists instead of the
 * stock ADC subsystem API (zephyr/drivers/adc.h): that API is built around
 * one-shot/repeated *sequences*, not a free-running producer, and the stock
 * raspberrypi,pico-adc driver has no DMA path to begin with.
 *
 * This is intentionally NOT a zephyr/drivers/adc.h-conformant driver, and
 * doesn't use the newer CONFIG_ADC_STREAM/RTIO API either — there is
 * exactly one consumer (tasks/ad_task.c) so the generic sequence/channel
 * mask contract and RTIO mempool machinery a general-purpose upstream
 * driver would need just adds risk for this phase. Revisit only if this
 * ever needs to be reusable/upstreamable.
 */

/** One completed ping-pong half-buffer, handed off via k_msgq. */
struct adc_stream_block {
	/** Points into the driver's internal ping-pong buffer — valid until
	 *  the *next* block for this stream is posted, not copied.
	 */
	const uint16_t *samples;
	size_t count;
};

/**
 * @brief Start free-running acquisition.
 *
 * Samples are pushed to @p msgq as they complete, one @ref adc_stream_block
 * per DMA half-transfer (CONFIG_ADC_STREAM_BUFFER_SIZE samples). The msgq's
 * message size must be sizeof(struct adc_stream_block) — block data itself
 * is not copied, so the consumer must be done with a block before the next
 * one lands (2 blocks in flight is the natural depth for ping-pong).
 *
 * @param dev  adc_stream device, e.g. DEVICE_DT_GET(DT_NODELABEL(adc_stream))
 * @param msgq Queue the driver pushes completed blocks onto.
 * @return 0 on success, -EINVAL if msgq's message size doesn't match,
 *         negative errno from the underlying dma_config()/dma_start() otherwise.
 */
int adc_stream_start(const struct device *dev, struct k_msgq *msgq);

/** @brief Stop acquisition. Safe to call whether or not it was started. */
int adc_stream_stop(const struct device *dev);

#endif /* APP_DRIVERS_ADC_STREAM_H_ */
