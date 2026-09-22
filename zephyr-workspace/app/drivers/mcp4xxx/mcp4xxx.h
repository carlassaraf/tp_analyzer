/*
 * Copyright (c) 2026 Fabrizio Carlassara
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APP_DRIVERS_MCP4XXX_H_
#define APP_DRIVERS_MCP4XXX_H_

#include <zephyr/device.h>

/**
 * @file
 * Public API for the Microchip MCP41XX/MCP42XX (MCP4131/4151/4231/4251)
 * SPI digital potentiometer/rheostat driver (app/drivers/mcp4xxx/).
 *
 * Not to be confused with Microchip's *other*, older, differently
 * protocol'd MCP41XXX/MCP42XXX (Microchip DS11195) -- despite the
 * near-identical naming, that's a different chip this driver does not
 * support. This one is documented in Microchip DS22060B ("MCP413X/415X/
 * 423X/425X 7/8-Bit Single/Dual SPI Digital POT with Volatile Memory").
 *
 * There is no upstream Zephyr subsystem class for digital potentiometers
 * to conform to. The shape below -- a device handle plus an explicit
 * channel argument, mirroring zephyr/drivers/dac.h -- is this driver's
 * own. See docs/ROADMAP.md in this directory for the rest of the design
 * rationale and validation status.
 *
 * @c channel is 0 for the only channel on a microchip,mcp41xx instance,
 * or 0/1 for the two channels on a microchip,mcp42xx instance.
 *
 * Wiper codes run 0 to an instance's full-scale value: 0x80 for a
 * 7-bit part (`resolution = <7>`), 0x100 for an 8-bit part
 * (`resolution = <8>`) -- one more than fits in a uint8_t, which is why
 * these take/return uint16_t rather than the byte-sized codes
 * DS11195-family chips use.
 *
 * IMPORTANT for any board's SPI bus node this is attached to: use
 * `cs-gpios`, not the SPI controller's own dedicated hardware CS pin.
 * On hardware that pulses its dedicated CS line between every data
 * frame instead of holding it low across a multi-byte transfer (RP2350
 * confirmed to do this; likely others), every 2-byte Read Data command
 * -- mcp4xxx_get(), mcp4xxx_read_tcon(), mcp4xxx_read_status() -- comes
 * back wrong: the chip sees two isolated 8-bit commands instead of one
 * 16-bit Read, each echoing its own out-of-context response. Single-
 * frame Writes happen to survive this, so it can look like the driver
 * mostly works while every read is silently corrupted.
 */

/**
 * @brief Set a channel's wiper position.
 *
 * @param dev     mcp4xxx device.
 * @param channel Channel to update.
 * @param value   Wiper code: 0 moves the wiper fully to terminal B, the
 *                instance's full-scale value (0x80 or 0x100) fully to
 *                terminal A. Resets to mid-scale at power-up; this
 *                family has no software or pin-based reset command
 *                afterwards (no RS pin on any package in this family).
 * @return 0 on success, -EINVAL if @p channel doesn't exist on this
 *         instance or @p value exceeds its full-scale code, negative
 *         errno from the underlying spi_write() otherwise.
 */
int mcp4xxx_set(const struct device *dev, uint8_t channel, uint16_t value);

/**
 * @brief Set a channel's wiper to approximate a target resistance.
 *
 * Maps @p ohms proportionally onto the wiper's code range using the
 * instance's devicetree `ohms` (total end-to-end resistance) and
 * `resolution` (full-scale code) properties: this is rheostat-mode
 * arithmetic (resistance between the wiper and one terminal), not
 * appropriate for potentiometer/voltage-divider mode. It's an
 * approximation, not a calibrated value -- wiper resistance and
 * device-to-device matching aren't factored in, and are themselves
 * only "typical" datasheet specs wider than one code step at the low
 * end of the range.
 *
 * @param dev     mcp4xxx device.
 * @param channel Channel to update.
 * @param ohms    Target resistance; clamped to the instance's `ohms`
 *                devicetree property if it's exceeded.
 * @return 0 on success, -EINVAL if @p channel doesn't exist on this
 *         instance, negative errno from the underlying spi_write() otherwise.
 */
int mcp4xxx_set_ohms(const struct device *dev, uint8_t channel, uint32_t ohms);

/**
 * @brief Read a channel's current wiper position back.
 *
 * Unlike the DS11195-family chips, this part can report its own wiper
 * setting over SPI (the Read Data command) -- no caller-side tracking
 * needed.
 *
 * @param dev     mcp4xxx device.
 * @param channel Channel to read.
 * @param value   Set to the current wiper code on success.
 * @return 0 on success, -EINVAL if @p channel doesn't exist on this
 *         instance, negative errno from the underlying spi_transceive()
 *         otherwise.
 */
int mcp4xxx_get(const struct device *dev, uint8_t channel, uint16_t *value);

/**
 * @brief Nudge a channel's wiper up by one code, in hardware.
 *
 * A single 8-bit SPI command; the device does the +1 itself; there is
 * no client-side value to keep in sync. A no-op once the wiper is
 * already at full-scale (the device clamps, per the datasheet's
 * Increment Operation table).
 *
 * @param dev     mcp4xxx device.
 * @param channel Channel to increment.
 * @return 0 on success, -EINVAL if @p channel doesn't exist on this
 *         instance, negative errno from the underlying spi_write() otherwise.
 */
int mcp4xxx_increment(const struct device *dev, uint8_t channel);

/**
 * @brief Nudge a channel's wiper down by one code, in hardware.
 *
 * See mcp4xxx_increment() -- same shape, opposite direction, clamps at
 * zero-scale instead of full-scale.
 *
 * @param dev     mcp4xxx device.
 * @param channel Channel to decrement.
 * @return 0 on success, -EINVAL if @p channel doesn't exist on this
 *         instance, negative errno from the underlying spi_write() otherwise.
 */
int mcp4xxx_decrement(const struct device *dev, uint8_t channel);

/**
 * @brief Read the raw 9-bit Terminal Control (TCON) register.
 *
 * Bit layout (DS22060B Register 4-2), bit 8 reserved/always 1:
 * `D8 R1HW R1A R1W R1B R0HW R0A R0W R0B`. Each RxA/RxW/RxB bit
 * independently connects (1) or disconnects (0) that terminal of
 * channel x from its resistor network; RxHW clears (0) to make channel
 * x follow the SHDN pin, or sets (1) to exempt it. All bits reset to 1
 * (all terminals connected, no channel exempted) at power-up.
 *
 * There's no per-channel bit-set/clear helper (yet -- see
 * docs/ROADMAP.md): read this, modify the 4 bits for the channel you
 * care about, and write the whole register back with
 * mcp4xxx_write_tcon() to avoid disturbing the other channel's bits.
 *
 * @param dev   mcp4xxx device.
 * @param value Set to the current TCON register value on success.
 * @return 0 on success, negative errno from the underlying
 *         spi_transceive() otherwise.
 */
int mcp4xxx_read_tcon(const struct device *dev, uint16_t *value);

/** @brief Write the raw TCON register. See mcp4xxx_read_tcon(). */
int mcp4xxx_write_tcon(const struct device *dev, uint16_t value);

/**
 * @brief Read the raw 9-bit STATUS register.
 *
 * Only bit 1 (SHDN) is currently meaningful (DS22060B Register 4-1):
 * set when the hardware SHDN pin is presently low. All other bits are
 * reserved.
 *
 * @param dev   mcp4xxx device.
 * @param value Set to the current STATUS register value on success.
 * @return 0 on success, negative errno from the underlying
 *         spi_transceive() otherwise.
 */
int mcp4xxx_read_status(const struct device *dev, uint16_t *value);

/**
 * @brief Drive the hardware SHDN pin (microchip,mcp42xx with shutdown-gpios only).
 *
 * A single pin shared by both channels; which channels it actually
 * affects is filtered by each channel's TCON RxHW bit (see
 * mcp4xxx_read_tcon()). Per the datasheet, must not be toggled while CS
 * is low -- not a concern here, this driver never holds CS low outside
 * of a single spi transfer.
 *
 * @param dev      mcp4xxx device.
 * @param shutdown true to assert hardware shutdown, false to release it.
 * @return 0 on success, -ENOTSUP if this instance has no shutdown-gpios
 *         (always true for a microchip,mcp41xx instance -- that
 *         package has no SHDN pin at all).
 */
int mcp4xxx_hw_shutdown(const struct device *dev, bool shutdown);

#endif /* APP_DRIVERS_MCP4XXX_H_ */
