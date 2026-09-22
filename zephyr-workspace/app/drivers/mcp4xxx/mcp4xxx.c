/*
 * Copyright (c) 2026 Fabrizio Carlassara
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mcp4xxx.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mcp4xxx, CONFIG_MCP4XXX_LOG_LEVEL);

/*
 * Command byte, DS22060B Figure 7-1: AD3 AD2 AD1 AD0 C1 C0 D9 D8.
 * AD3:AD0 addresses one of the 16 memory locations (Table 7-2); C1:C0
 * picks the command (Table 7-1); D9:D8 are the two data bits that live
 * in the command byte itself for a 16-bit Write/Read (D9 unused, D8 is
 * the wiper's 9th bit) and don't-care for the 8-bit Increment/Decrement.
 */
#define MCP4XXX_REG_WIPER0 0x0
#define MCP4XXX_REG_WIPER1 0x1
#define MCP4XXX_REG_TCON   0x4
#define MCP4XXX_REG_STATUS 0x5

#define MCP4XXX_CMD_WRITE 0x0
#define MCP4XXX_CMD_INCR  0x1
#define MCP4XXX_CMD_DECR  0x2
#define MCP4XXX_CMD_READ  0x3

#define MCP4XXX_CMD_BYTE(reg, cmd, d8) (((reg) << 4) | ((cmd) << 2) | ((d8) & 0x1))

struct mcp4xxx_config {
	struct spi_dt_spec bus;
	struct gpio_dt_spec shutdown_gpio; 	/* .port == NULL if shutdown-gpios absent */
	uint32_t ohms;                     	/* devicetree `ohms`, for mcp4xxx_set_ohms() */
	uint16_t full_scale;                /* 0x80 (7-bit) or 0x100 (8-bit) */
	uint8_t channels;                  	/* 1 (mcp41xx) or 2 (mcp42xx) */
};

static int mcp4xxx_wiper_reg(const struct mcp4xxx_config *config, uint8_t channel)
{
	if (channel >= config->channels) {
		return -EINVAL;
	}

	return MCP4XXX_REG_WIPER0 + channel;
}

int mcp4xxx_set(const struct device *dev, uint8_t channel, uint16_t value)
{
	const struct mcp4xxx_config *config = dev->config;
	uint8_t buf[2];
	struct spi_buf tx_buf = {.buf = buf, .len = sizeof(buf)};
	struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
	int reg = mcp4xxx_wiper_reg(config, channel);

	if (reg < 0) {
		return reg;
	}
	if (value > config->full_scale) {
		LOG_ERR("%s: value %u exceeds full-scale %u", dev->name, value,
			config->full_scale);
		return -EINVAL;
	}

	buf[0] = MCP4XXX_CMD_BYTE(reg, MCP4XXX_CMD_WRITE, value >> 8);
	buf[1] = value & 0xFF;

	return spi_write_dt(&config->bus, &tx);
}

int mcp4xxx_set_ohms(const struct device *dev, uint8_t channel, uint32_t ohms)
{
	const struct mcp4xxx_config *config = dev->config;
	uint32_t code;

	if (ohms >= config->ohms) {
		code = config->full_scale;
	} else {
		code = (uint32_t)(((uint64_t)ohms * config->full_scale) / config->ohms);
	}

	return mcp4xxx_set(dev, channel, (uint16_t)code);
}

static int mcp4xxx_read_reg(const struct device *dev, uint8_t reg, uint16_t *value)
{
	const struct mcp4xxx_config *config = dev->config;
	uint8_t tx_data[2] = {MCP4XXX_CMD_BYTE(reg, MCP4XXX_CMD_READ, 0), 0x00};
	uint8_t rx_data[2];
	struct spi_buf tx_buf = {.buf = tx_data, .len = sizeof(tx_data)};
	struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
	struct spi_buf rx_buf = {.buf = rx_data, .len = sizeof(rx_data)};
	struct spi_buf_set rx = {.buffers = &rx_buf, .count = 1};
	int ret;

	ret = spi_transceive_dt(&config->bus, &tx, &rx);
	if (ret != 0) {
		return ret;
	}

	/* Figure 7-1 / Table 7-2: SDO echoes "1111 111n nnnn nnnn" -- D8 is
	 * the low bit of the first byte, D7:D0 are the whole second byte.
	 */
	*value = ((uint16_t)(rx_data[0] & 0x01) << 8) | rx_data[1];

	return 0;
}

int mcp4xxx_get(const struct device *dev, uint8_t channel, uint16_t *value)
{
	const struct mcp4xxx_config *config = dev->config;
	int reg = mcp4xxx_wiper_reg(config, channel);

	if (reg < 0) {
		return reg;
	}

	return mcp4xxx_read_reg(dev, reg, value);
}

static int mcp4xxx_step(const struct device *dev, uint8_t channel, uint8_t cmd)
{
	const struct mcp4xxx_config *config = dev->config;
	uint8_t buf[1];
	struct spi_buf tx_buf = {.buf = buf, .len = sizeof(buf)};
	struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
	int reg = mcp4xxx_wiper_reg(config, channel);

	if (reg < 0) {
		return reg;
	}

	buf[0] = MCP4XXX_CMD_BYTE(reg, cmd, 0);

	return spi_write_dt(&config->bus, &tx);
}

int mcp4xxx_increment(const struct device *dev, uint8_t channel)
{
	return mcp4xxx_step(dev, channel, MCP4XXX_CMD_INCR);
}

int mcp4xxx_decrement(const struct device *dev, uint8_t channel)
{
	return mcp4xxx_step(dev, channel, MCP4XXX_CMD_DECR);
}

int mcp4xxx_read_tcon(const struct device *dev, uint16_t *value)
{
	return mcp4xxx_read_reg(dev, MCP4XXX_REG_TCON, value);
}

int mcp4xxx_write_tcon(const struct device *dev, uint16_t value)
{
	const struct mcp4xxx_config *config = dev->config;
	uint8_t buf[2];
	struct spi_buf tx_buf = {.buf = buf, .len = sizeof(buf)};
	struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};

	buf[0] = MCP4XXX_CMD_BYTE(MCP4XXX_REG_TCON, MCP4XXX_CMD_WRITE, value >> 8);
	buf[1] = value & 0xFF;

	return spi_write_dt(&config->bus, &tx);
}

int mcp4xxx_read_status(const struct device *dev, uint16_t *value)
{
	return mcp4xxx_read_reg(dev, MCP4XXX_REG_STATUS, value);
}

int mcp4xxx_hw_shutdown(const struct device *dev, bool shutdown)
{
	const struct mcp4xxx_config *config = dev->config;

	if (config->shutdown_gpio.port == NULL) {
		return -ENOTSUP;
	}

	return gpio_pin_set_dt(&config->shutdown_gpio, shutdown);
}

static int mcp4xxx_init(const struct device *dev)
{
	const struct mcp4xxx_config *config = dev->config;
	int ret;

	if (!spi_is_ready_dt(&config->bus)) {
		LOG_ERR("%s: SPI bus not ready", dev->name);
		return -ENODEV;
	}

	if (config->shutdown_gpio.port != NULL) {
		ret = gpio_pin_configure_dt(&config->shutdown_gpio, GPIO_OUTPUT_INACTIVE);
		if (ret != 0) {
			LOG_ERR("%s: failed to configure shutdown-gpios (%d)", dev->name, ret);
			return ret;
		}
	}

	return 0;
}

/*
 * SPI mode 0,0 (no CPOL/CPHA flags): the datasheet allows mode 0,0 or 1,1
 * indifferently, and the driver fixes one rather than making it a
 * devicetree property -- see docs/ROADMAP.md if 1,1 ever turns out to be
 * needed for a given board's SPI controller.
 */
#define MCP4XXX_SPI_OP (SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER)

#define MCP4XXX_FULL_SCALE(resolution) ((resolution) == 8 ? 0x100 : 0x80)

/*
 * `prefix` disambiguates config struct names between the two compatibles
 * below -- DT_INST_FOREACH_STATUS_OKAY_VARGS numbers instances per-compat
 * starting at 0, so mcp41xx instance 0 and mcp42xx instance 0 would
 * otherwise both expand to the same mcp4xxx_config_0 symbol.
 */
#define MCP4XXX_INIT(n, prefix, nchannels)                                                        \
	static const struct mcp4xxx_config prefix##_config_##n = {                               \
		.bus = SPI_DT_SPEC_INST_GET(n, MCP4XXX_SPI_OP),                                   \
		.shutdown_gpio = GPIO_DT_SPEC_INST_GET_OR(n, shutdown_gpios, {0}),                \
		.ohms = DT_INST_PROP(n, ohms),                                                    \
		.full_scale = MCP4XXX_FULL_SCALE(DT_INST_PROP(n, resolution)),                    \
		.channels = (nchannels),                                                          \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(n, mcp4xxx_init, NULL, NULL, &prefix##_config_##n, POST_KERNEL,     \
			      CONFIG_MCP4XXX_INIT_PRIORITY, NULL);

#define DT_DRV_COMPAT microchip_mcp41xx
DT_INST_FOREACH_STATUS_OKAY_VARGS(MCP4XXX_INIT, mcp41xx, 1)
#undef DT_DRV_COMPAT

#define DT_DRV_COMPAT microchip_mcp42xx
DT_INST_FOREACH_STATUS_OKAY_VARGS(MCP4XXX_INIT, mcp42xx, 2)
#undef DT_DRV_COMPAT
