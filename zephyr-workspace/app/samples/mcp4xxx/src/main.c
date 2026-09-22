/*
 * Copyright (c) 2026 Fabrizio Carlassara
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include "mcp4xxx.h"

/*
 * Bench bring-up sample for app/drivers/mcp4xxx/ -- see this directory's
 * boards/ subdirectory for the wiring overlay, and docs/ROADMAP.md (in
 * the driver's own directory) for what's been validated against real
 * hardware so far.
 *
 * Deliberately shell-driven rather than an automatic sweep: each command
 * does exactly one thing and holds the result, so a multimeter/scope
 * reading can be taken at a steady value instead of chasing a moving one.
 */

static const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(mcp42xx0));

static int cmd_set(const struct shell *sh, size_t argc, char **argv)
{
	uint8_t channel = (uint8_t)strtoul(argv[1], NULL, 0);
	uint16_t value = (uint16_t)strtoul(argv[2], NULL, 0);
	int ret = mcp4xxx_set(dev, channel, value);

	if (ret != 0) {
		shell_error(sh, "mcp4xxx_set(%u, %u) failed: %d", channel, value, ret);
		return ret;
	}
	shell_print(sh, "ch%u <- %u", channel, value);
	return 0;
}

static int cmd_get(const struct shell *sh, size_t argc, char **argv)
{
	uint8_t channel = (uint8_t)strtoul(argv[1], NULL, 0);
	uint16_t value;
	int ret = mcp4xxx_get(dev, channel, &value);

	if (ret != 0) {
		shell_error(sh, "mcp4xxx_get(%u) failed: %d", channel, ret);
		return ret;
	}
	shell_print(sh, "ch%u -> %u", channel, value);
	return 0;
}

static int cmd_inc(const struct shell *sh, size_t argc, char **argv)
{
	uint8_t channel = (uint8_t)strtoul(argv[1], NULL, 0);
	int ret = mcp4xxx_increment(dev, channel);

	if (ret != 0) {
		shell_error(sh, "mcp4xxx_increment(%u) failed: %d", channel, ret);
		return ret;
	}
	shell_print(sh, "ch%u incremented", channel);
	return 0;
}

static int cmd_dec(const struct shell *sh, size_t argc, char **argv)
{
	uint8_t channel = (uint8_t)strtoul(argv[1], NULL, 0);
	int ret = mcp4xxx_decrement(dev, channel);

	if (ret != 0) {
		shell_error(sh, "mcp4xxx_decrement(%u) failed: %d", channel, ret);
		return ret;
	}
	shell_print(sh, "ch%u decremented", channel);
	return 0;
}

static int cmd_ohms(const struct shell *sh, size_t argc, char **argv)
{
	uint8_t channel = (uint8_t)strtoul(argv[1], NULL, 0);
	uint32_t ohms = (uint32_t)strtoul(argv[2], NULL, 0);
	int ret = mcp4xxx_set_ohms(dev, channel, ohms);

	if (ret != 0) {
		shell_error(sh, "mcp4xxx_set_ohms(%u, %u) failed: %d", channel, ohms, ret);
		return ret;
	}
	shell_print(sh, "ch%u <- ~%u ohms", channel, ohms);
	return 0;
}

static int cmd_shdn(const struct shell *sh, size_t argc, char **argv)
{
	bool assert_shdn = strcmp(argv[1], "on") == 0;
	int ret = mcp4xxx_hw_shutdown(dev, assert_shdn);

	if (ret != 0) {
		shell_error(sh, "mcp4xxx_hw_shutdown(%d) failed: %d", assert_shdn, ret);
		return ret;
	}
	shell_print(sh, "SHDN %s", assert_shdn ? "asserted" : "released");
	return 0;
}

static int cmd_tcon_read(const struct shell *sh, size_t argc, char **argv)
{
	uint16_t value;
	int ret = mcp4xxx_read_tcon(dev, &value);

	if (ret != 0) {
		shell_error(sh, "mcp4xxx_read_tcon() failed: %d", ret);
		return ret;
	}
	shell_print(sh, "TCON = 0x%03x", value);
	return 0;
}

static int cmd_tcon_write(const struct shell *sh, size_t argc, char **argv)
{
	uint16_t value = (uint16_t)strtoul(argv[1], NULL, 0);
	int ret = mcp4xxx_write_tcon(dev, value);

	if (ret != 0) {
		shell_error(sh, "mcp4xxx_write_tcon(0x%03x) failed: %d", value, ret);
		return ret;
	}
	shell_print(sh, "TCON <- 0x%03x", value);
	return 0;
}

static int cmd_status(const struct shell *sh, size_t argc, char **argv)
{
	uint16_t value;
	int ret = mcp4xxx_read_status(dev, &value);

	if (ret != 0) {
		shell_error(sh, "mcp4xxx_read_status() failed: %d", ret);
		return ret;
	}
	shell_print(sh, "STATUS = 0x%03x (SHDN pin %s)", value,
		    (value & BIT(1)) ? "low" : "high");
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(mcp4xxx_cmds,
	SHELL_CMD_ARG(set, NULL, "<channel> <value>", cmd_set, 3, 0),
	SHELL_CMD_ARG(get, NULL, "<channel>", cmd_get, 2, 0),
	SHELL_CMD_ARG(inc, NULL, "<channel>", cmd_inc, 2, 0),
	SHELL_CMD_ARG(dec, NULL, "<channel>", cmd_dec, 2, 0),
	SHELL_CMD_ARG(ohms, NULL, "<channel> <ohms>", cmd_ohms, 3, 0),
	SHELL_CMD_ARG(shdn, NULL, "<on|off>", cmd_shdn, 2, 0),
	SHELL_CMD_ARG(tcon_read, NULL, "", cmd_tcon_read, 1, 0),
	SHELL_CMD_ARG(tcon_write, NULL, "<value>", cmd_tcon_write, 2, 0),
	SHELL_CMD_ARG(status, NULL, "", cmd_status, 1, 0),
	SHELL_SUBCMD_SET_END
);
SHELL_CMD_REGISTER(mcp4xxx, &mcp4xxx_cmds, "mcp4xxx driver bring-up commands", NULL);

int main(void)
{
	if (!device_is_ready(dev)) {
		return -ENODEV;
	}
	return 0;
}
