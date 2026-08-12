# Zephyr Migration Roadmap

Tracks re-platforming the TP Analyzer firmware from FreeRTOS onto Zephyr, on
the same target (Raspberry Pi Pico 2 / RP2350, `rpi_pico2/rp2350a/m33`).

The FreeRTOS reference implementation (companion `CONTEXT.md`, `hal/`,
`rtos/`, `drivers/`, `bsp/`, `cmake/Kconfig.cmake`, ...) lives on **`main`**
under `firmware/` — this branch (`feat/zephyr`) dropped that tree when it
restructured into the Zephyr west workspace below, so file references to it
in this document are paths on `main:firmware/`, not paths in this tree.
Check out `main` any time you need to build a known-good `.uf2` to A/B
against.

Display target is the **ILI9486 8080-parallel panel only** — the ST7789/SPI
fallback path is dropped for this migration.

**Key finding (confirmed against our pinned v4.4.2 checkout, not just docs):**
Zephyr already ships a generic MIPI DBI (Display Bus Interface) subsystem
with two ready-made 8080-parallel bus backends — `zephyr,mipi-dbi-bitbang`
(GPIO bit-bang, our "SIO" plan) and `raspberrypi,pico-mipi-dbi-pio`
(RP2350-specific PIO, our "PIO" plan) — both already upstream. A display
controller driver written against the generic `mipi_dbi` API doesn't care
which backend is underneath it, so **PIO vs. SIO becomes a one-line
devicetree `compatible` swap on the bus node**, not two driver
implementations. There's no `ilitek,ili9486` compatible/driver yet, but the
closely related `ilitek,ili9488` one (`display_ili9xxx.c` +
`display_ili9488.c`) is in-tree, and
[zephyrproject-rtos/zephyr#99540](https://github.com/zephyrproject-rtos/zephyr/issues/99540)
shows a real ILI9486 panel brought up through it — command set is close
enough, gamma/power tables are 9488-tuned. See Phase 1a.

## Guiding principle

De-risk the two highest-uncertainty pieces — the parallel display and the
streaming ADC — **first, in isolation**, before porting any application logic
on top of them. Everything RTOS-mechanical (tasks, sync primitives, RTC, PWM)
is low-risk and can be ported in parallel or after.

## Working method

- Work happens in the `zephyr-workspace/` west workspace (topdir), with
  `app/` as both the west manifest repo and the Zephyr application root —
  this file lives at `app/ZEPHYR_MIGRATION.md`, so paths below are relative
  to `app/` unless marked as `main:firmware/...`.
- Reuse RTOS-agnostic logic by porting it as-is rather than reinventing it:
  `main:firmware/services/dsp/dsp.c`,
  `main:firmware/app/lvgl/helpers/chart.c`, the `screens/` and generated
  `ui/` code.
- Each phase below ends with hardware-verified exit criteria before moving to
  the next. Tag known-good checkpoints in git.

---

## Current directory structure

```
zephyr-workspace/                      (west topdir)
├── .west/config                       (manifest.path = app)
├── app/                                (west manifest repo == Zephyr app root)
│   ├── west.yml                        (zephyr v4.4.2, name-allowlist: cmsis_6, hal_rpi_pico, lvgl)
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── boards/
│   │   └── rpi_pico2_rp2350a_m33.overlay   (USB CDC ACM console; pins TBD Phase 5)
│   ├── src/
│   │   ├── CMakeLists.txt
│   │   ├── main.c
│   │   ├── app.c / app.h
│   │   ├── tasks/
│   │   │   ├── ad_task.c/h
│   │   │   └── ui_task.c
│   │   └── lvgl/
│   │       ├── screen_manager.c/h
│   │       ├── screen_update.c/h
│   │       ├── helpers/
│   │       ├── screens/
│   │       └── ui/                     (generated UI code, unchanged)
│   ├── services/
│   │   └── dsp/                        (CMSIS-DSP FFT, unchanged, RTOS-agnostic)
│   ├── drivers/
│   │   └── display_ili9486/            (dead — carried over from the
│   │       ├── ili9486.c                FreeRTOS tree, never wired into any
│   │       └── ili9486.h                CMakeLists, not part of the build.
│   │                                    Phase 1a resolved without it — see
│   │                                    below. Not yet deleted.)
│   └── ZEPHYR_MIGRATION.md
├── zephyr/                             (cloned by `west update`)
├── modules/                            (cloned by `west update`)
└── build/
```

Not created yet, added when the phase that needs them arrives:
- `app/Kconfig` — app-level Kconfig menu ported from `main:firmware/Kconfig`
  (Phase 5).
- `app/drivers/adc_stream/` — continuous ADC + DMA driver (Phase 1b).

Deferred, not currently planned:
- `app/dts/bindings/display/ilitek,ili9486.yaml` + a dedicated driver — only
  if the reused `ilitek,ili9488` binding's picture quality becomes an actual
  problem later (see Phase 1a, resolved).

---

## Phase 0 — Toolchain & skeleton bring-up ✅

**Goal:** an empty Zephyr app boots and prints over USB on the real board.

- [x] Install `west` + Zephyr SDK.
- [x] Build for `rpi_pico2/rp2350a/m33` and flash on actual hardware.
- [x] Confirm flashing workflow (UF2 drag-drop).
- [x] Confirm USB CDC ACM console/logging.
- [x] Lay down the [directory structure](#current-directory-structure) above.

**Decisions made:**
- USB console uses the **new** USB device stack
  (`CONFIG_USB_DEVICE_STACK_NEXT` + `CONFIG_CDC_ACM_SERIAL_INITIALIZE_AT_BOOT`),
  not the legacy `usb_enable()`/`CONFIG_USB_DEVICE_STACK` API (deprecated,
  removal targeted for Zephyr v4.5.0; we're pinned to v4.4.2). Required a
  `cdc_acm_uart0` node under `&zephyr_udc0` in the board overlay — it isn't
  present by default even though the `zephyr_udc0` controller itself is
  already `status = "okay"` on `rpi_pico2`.

**Exit criteria:** met — "Hello world" runs on the real board, console
visible over USB CDC ACM, no FreeRTOS involved.

---

## Phase 1 — De-risk the hard drivers, standalone, no UI

### 1a. ILI9486 display, on top of Zephyr's MIPI DBI subsystem ✅

No bus-level bit-banging or PIO programming had to be written — `zephyr,mipi-dbi-bitbang`
(SIO) and `raspberrypi,pico-mipi-dbi-pio` (PIO) both already implement the
generic `mipi_dbi` API upstream (confirmed present in our pinned v4.4.2
checkout).

- [x] Probed with the existing `ilitek,ili9488` binding — no new driver code:
  - `mipi_dbi` bus node (`compatible = "zephyr,mipi-dbi-bitbang"`) in
    `boards/rpi_pico2_rp2350a_m33.overlay`, wired to the known-good pinout
    from `main:firmware/bsp/rp2350/board_config.h` (D0–D7 = GPIO0–7,
    WR=17, RD=16, RS/DC=18, CS=19, RST=20).
  - Polarity per [issue #99540](https://github.com/zephyrproject-rtos/zephyr/issues/99540):
    `cs-gpios`/`reset-gpios` = `GPIO_ACTIVE_LOW`, `dc-gpios`/`wr-gpios`/`rd-gpios`
    = `GPIO_ACTIVE_HIGH` — got it right first try, no white-screen debugging.
  - Child node `compatible = "ilitek,ili9488"`, 480×320,
    `mipi-mode = "MIPI_DBI_MODE_8080_BUS_8_BIT"`,
    `pixel-format = <PANEL_PIXEL_FORMAT_RGB_565>` (not `<1>` — that's
    `PANEL_PIXEL_FORMAT_RGB_888`, a copy-paste trap from the issue's example
    overlay), `rotation = <90>` (landscape, matching the FreeRTOS driver's
    `rotation = 1`).
  - `CONFIG_MIPI_DBI=y`, `CONFIG_DISPLAY=y`, `CONFIG_ILI9488=y` in `prj.conf`.
  - Static-pattern/gradient test via the raw `display` API (color bands +
    gradient strip, no LVGL) — correct on real hardware.
- [x] Picture is correct and acceptable → **no dedicated `ilitek,ili9486`
      driver needed.** The app's carried-over `drivers/display_ili9486/`
      files are dead code as a result (see directory structure above) —
      not yet deleted, but no longer part of the plan.
- [x] Frame time measured on real hardware (bitbang backend, 20-frame
      average, `src/main.c`'s `run_fps_test()`): **312.6 ms for a full
      480×320 flush (~3.1 fps)**, ≈0.98 ms/row. Full-screen redraw isn't
      the metric that matters for this app though — LVGL will only flush
      dirty regions, and the FreeRTOS driver's own partial buffer was 40
      lines tall. Extrapolated to that same chunk size: **~39 ms per
      40-line partial flush**, up to ~25 flushes/sec back-to-back. Judged
      acceptable to proceed on; revisit only if Phase 4's real chart/FFT
      redraw rate feels sluggish once LVGL is actually driving it.

**PIO attempted and reverted:** swapping the bus node to
`raspberrypi,pico-mipi-dbi-pio` built and flashed but produced an all-white
screen. Root-caused (by reading `mipi_dbi_rpi_pico_pio.c`, not confirmed on
a scope) to our exact pin numbers — WR=17, DC=18, CS=19 are 3 consecutive
GPIOs, which trips the driver's "consecutive control pins" fast path: CS/DC/WR
get driven through hardcoded PIO side-set instructions instead of through
`gpio_pin_set_dt()` (which is what respects our devicetree `GPIO_ACTIVE_LOW`/
`HIGH` flags, and what the bitbang backend and the driver's *non*-consecutive
path both use). That side-set logic couldn't be verified by static reading
alone — would need a scope on WR/CS/DC or upstream input to pin down further.
Since 1a's bitbang numbers above are judged acceptable, PIO stays parked as
optional future work, not a blocker.

**Note:** this is the `ilitek,ili9488` driver's power/gamma tuning, not a
native ILI9486 driver — Zephyr still has no `ilitek,ili9486` compatible.
It's "close enough to work," per the linked issue's precedent, not "ILI9486
ships out of the box." If picture quality (contrast/gamma/viewing angle)
becomes a real problem later — e.g. once real waveform content is on
screen under LVGL — the fallback is still to port the known-good register
tables from `main:firmware/drivers/display/ili9486.c` (`PWCTRL1/2/3`,
`VMCTRL1`, `DFUNCTR`, 15-byte `PGAMCTRL`/`NGAMCTRL`) into a small driver
against the public `mipi_dbi.h` + `display.h` APIs. Not needed for now.

**PIO later, if needed:** swap the bus node's `compatible` from
`zephyr,mipi-dbi-bitbang` to `raspberrypi,pico-mipi-dbi-pio` (adjusting its
`data-pin-splits`/`pio-clock-div` properties) — the `ilitek,ili9488` display
node needs zero changes, since it only ever talks to the abstract `mipi_dbi`
device.

**Exit criteria:** met — panel renders correctly on real hardware through
the reused `ilitek,ili9488` binding over the bitbang `mipi_dbi` backend,
correct orientation, no visual artifacts during window-set/pixel-push
sequences, frame time measured and judged acceptable (~39 ms/40-line
partial flush, extrapolated from a 312.6 ms full-screen benchmark).

### 1b. Continuous ADC + DMA acquisition

- [ ] Standalone thread that free-runs the ADC into ping-pong DMA buffers at
      10 kHz, bypassing the stock `adc_rpi_pico` driver (it is
      one-shot/round-robin only, no continuous DMA streaming), mirroring
      `main:firmware/hal/hal_adc.c`'s ISR logic.
- [ ] Hand completed buffers to a `k_msgq`.
- [ ] Validate sample rate and values via UART dump of peak/RMS/frequency
      against a known signal-generator input.

**Exit criteria:** correct sample rate and values on hardware, no display
needed yet.

---

## Phase 2 — Mechanical RTOS-primitive porting

Can run in parallel with Phase 1. Each item independently testable.

- [ ] Task creation (`app.c`) → `K_THREAD_DEFINE` / priorities.
- [ ] `xTaskNotifyFromISR`/`xTaskNotifyWait` in `tasks/ad_task.c` →
      `k_sem`/`k_msgq` handoff from the Phase 1b ADC driver.
- [ ] Cross-task `QueueHandle_t` in `lvgl/screen_update.c` → `k_msgq`, or
      drop it in favor of the Zephyr LVGL module's workqueue-driven
      `lv_timer_handler`/`lv_async_call` (re-evaluate whether a custom queue
      is still needed).
- [ ] Once-a-minute RTC software timer → `k_timer`.
- [ ] `main:firmware/hal/hal_rtc.c` → native `raspberrypi,pico-rtc` driver
      via `zephyr/drivers/rtc.h`.
- [ ] PWM backlight in `main.c` → `raspberrypi,pico-pwm` + Zephyr PWM API.
- [ ] Stack-overflow hook (`main:firmware/rtos/hooks.c`) →
      `CONFIG_STACK_SENTINEL` / thread fault handling equivalent.

**Exit criteria:** each primitive swap has a small standalone test (RTC
round-trips a date, PWM backlight dims, a dummy thread survives a
stack-overflow probe) — no full app yet.

---

## Phase 3 — Encoder input + LVGL group navigation

- [ ] Port `main:firmware/drivers/encoder/encoder.c` (GPIO IRQ edge-detect)
      onto Zephyr GPIO callbacks.
- [ ] Wire it as an `LV_INDEV_TYPE_ENCODER` input device via
      `CONFIG_LV_Z_ENCODER_INPUT`, replacing the manual
      `lv_indev_create`/`encoder_read_cb` from
      `main:firmware/services/lvgl/lvgl_port.c`.

**Exit criteria:** on the Phase 1a ILI9486 driver, an LVGL demo screen is
navigable with the physical encoder — turn, press, select.

---

## Phase 4 — Real screens, one at a time

- [ ] `scr_boot` — confirms boot flow and `lvgl/screen_manager.c` state
      machine under Zephyr threading.
- [ ] `scr_menu` — confirms encoder + group focus end-to-end.
- [ ] `scr_datetime`, `scr_settings`, `scr_information` — static/RTC-fed,
      low risk.
- [ ] `scr_oscilloscope` — wire the Phase 1b ADC stream through `chart.c`'s
      scaling into `lv_chart_set_series_ext_y_array`, replacing
      `screen_update_plot_data`'s FreeRTOS-queue path.
- [ ] `scr_fft` — wire `services/dsp/dsp.c` (unchanged) through the same
      pipeline; full acquire → FFT → render loop.

**Exit criteria:** every screen in `src/lvgl/screens/` is reachable and
functionally correct on the ILI9486 panel, A/B'd (from `main`) for waveform
accuracy, frequency reading, and responsiveness.

---

## Phase 5 — Devicetree/Kconfig consolidation

- [ ] Move `main:firmware/bsp/rp2350/board_config.h`'s pin `#define`s into
      `boards/rpi_pico2_rp2350a_m33.overlay` + pinctrl nodes.
- [ ] Port the in-progress `main:firmware/Kconfig` menu (ILI9486 backend
      choice, per-driver pin config) into a new `app/Kconfig`.
- [ ] Confirm nothing on this branch still needs the FreeRTOS/pico-sdk-direct
      trees that only exist on `main` (`rtos/`, `hal/`, `drivers/` there).

**Exit criteria:** `west build -b rpi_pico2/rp2350a/m33 app` builds from the
[current directory structure](#current-directory-structure) with no
outstanding references to `main:firmware/`.

---

## Phase 6 — Soak & cutover

- [ ] Run both firmwares side-by-side for a burn-in period (encoder stress,
      long-run RTC drift, sustained waveform capture) — Zephyr build from
      `feat/zephyr`, FreeRTOS build from `main`.
- [ ] Tag the last known-good FreeRTOS commit on `main` before it's replaced,
      in case a regression needs to be bisected back.
- [ ] Merge `feat/zephyr` into `main`, retiring `firmware/`.

---

## Risk map (for sequencing/timeboxing decisions)

| Phase | Risk | Notes |
|-------|------|-------|
| 0     | Low | Done. Standard Zephyr bring-up. |
| 1     | 1a done, 1b remains | 1a resolved with zero new driver code, reusing the in-tree `ilitek,ili9488` driver over Zephyr's own `mipi_dbi` bitbang backend — a dedicated `ilitek,ili9486` driver stays documented as a fallback but isn't needed. 1b (continuous ADC+DMA) is now the sole higher-uncertainty item left in this phase. |
| 2–4   | Low, mechanical | Predictable 1:1 API mappings. |
| 5     | Bookkeeping | No functional risk. |
