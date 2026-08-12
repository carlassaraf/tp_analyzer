# Zephyr Migration Roadmap

Companion to [CONTEXT.md](CONTEXT.md), which describes the current FreeRTOS
architecture. This document tracks the plan to re-platform the same
application onto Zephyr, on the same target (Raspberry Pi Pico 2 / RP2350,
`rpi_pico2/rp2350a/m33`).

Display target is the **ILI9486 8080-parallel panel only** — the ST7789/SPI
fallback path is dropped for this migration. The ILI9486 driver's pixel-push
backend (PIO state machine vs. plain SIO bit-banging) is left open; Phase 1
resolves it with a hardware spike before anything else depends on it.

## Guiding principle

De-risk the two highest-uncertainty pieces — the parallel display and the
streaming ADC — **first, in isolation**, before porting any application logic
on top of them. Everything RTOS-mechanical (tasks, sync primitives, RTC, PWM)
is low-risk and can be ported in parallel or after.

## Working method

- Do this work on a separate branch / west workspace (this branch:
  `feat/zephyr`). Keep the existing FreeRTOS tree in [firmware/](.) shippable
  and flashable throughout — always have a known-good `.uf2` to A/B against.
- Reuse RTOS-agnostic sources as-is rather than duplicating them:
  [services/dsp/dsp.c](services/dsp/dsp.c),
  [app/lvgl/helpers/chart.c](app/lvgl/helpers/chart.c), the `screens/` and
  generated `ui/` code.
- Each phase below ends with hardware-verified exit criteria before moving to
  the next. Tag known-good checkpoints in git.

---

## Target directory structure

The Zephyr tree lives alongside the FreeRTOS one during the migration
(`firmware/` keeps both until Phase 6). Proposed layout for the new
application, following Zephyr's freestanding-app conventions:

```
firmware/
├── west.yml                     # west manifest (Zephyr + module versions pinned)
├── CMakeLists.txt                # top-level Zephyr app CMake
├── prj.conf                      # base Kconfig fragment (LVGL, RTC, ADC stream, ...)
├── Kconfig                       # app-level Kconfig menu (ported from current Kconfig)
├── boards/
│   └── rpi_pico2_rp2350a_m33.overlay   # pin/peripheral devicetree overlay
├── dts/
│   └── bindings/
│       ├── display/
│       │   └── vendor,ili9486-parallel.yaml   # custom devicetree binding
│       └── adc/
│           └── vendor,adc-stream.yaml          # custom binding, if driver-shaped
├── src/
│   ├── main.c
│   ├── app/
│   │   ├── app.c / app.h
│   │   └── lvgl/
│   │       ├── screen_manager.c/h
│   │       ├── screen_update.c/h
│   │       ├── helpers/          # chart.c/h, unchanged
│   │       ├── screens/          # unchanged
│   │       └── ui/               # generated UI code, unchanged
│   └── services/
│       └── dsp/                  # unchanged
├── drivers/                      # out-of-tree Zephyr drivers, app-local
│   ├── display_ili9486/
│   │   ├── CMakeLists.txt
│   │   ├── Kconfig
│   │   ├── display_ili9486.c     # Zephyr `display` API, PIO or SIO backend (TBD)
│   │   ├── display_ili9486_pio.c     # built only if PIO backend selected
│   │   ├── display_ili9486_sio.c     # built only if SIO backend selected
│   │   └── ili9486_write.pio.h   # pre-assembled PIO program (pioasm output)
│   └── adc_stream/
│       ├── CMakeLists.txt
│       ├── Kconfig
│       └── adc_stream.c          # continuous ADC + DMA ping-pong acquisition
└── ZEPHYR_MIGRATION.md
```

Notes:
- `drivers/` stays app-local (not a redistributable west module) — simplest
  option for a single-product firmware. Revisit only if the drivers need to
  be shared across boards/projects.
- `display_ili9486.c` exposes the Zephyr `display` API and delegates pixel
  push to whichever backend file is compiled in, selected by a Kconfig choice
  (`DISPLAY_ILI9486_BACKEND_PIO` / `_SIO`) set in Phase 1a.
- The FreeRTOS-era [rtos/](rtos), [hal/](hal) and [drivers/](drivers) (current,
  pico-sdk-direct versions) stay untouched until Phase 6 deletes them.

---

## Phase 0 — Toolchain & skeleton bring-up

**Goal:** an empty Zephyr app boots and prints over USB on the real board.

- [x] Install `west` + Zephyr SDK.
- [x] Build & flash `samples/hello_world` for `rpi_pico2/rp2350a/m33` on
      actual hardware.
- [x] Confirm flashing workflow (UF2 drag-drop or `picotool`/SWD).
- [x] Confirm USB CDC console/logging, matching current [main.c](main.c)
      behavior.
- [ ] Lay down the [target directory structure](#target-directory-structure)
      (`west.yml`, `CMakeLists.txt`, `prj.conf`, empty `src/`, `drivers/`,
      `dts/bindings/`, `boards/` overlay skeleton).

**Exit criteria:** "Hello world" runs on the real board from the new
directory layout, console visible, no FreeRTOS involved.

---

## Phase 1 — De-risk the hard drivers, standalone, no UI

### 1a. ILI9486 custom display driver (PIO or SIO — decide here)

- [ ] Spike both pixel-push backends against a minimal `display` driver
      (static fill / gradient, no LVGL yet) and pick one before building
      anything on top:
  - **PIO backend:** pre-assemble `ili9486_write.pio` offline with `pioasm`
    and embed the instruction words (Zephyr does not compile `.pio` files at
    build time the way `pico_generate_pio_header()` does today). Reconcile
    the PIO↔SIO GPIO handoff trick (`hal_pio_release_gpio`/`claim_gpio`) with
    Zephyr's pinctrl ownership of those pins.
  - **SIO backend:** plain bit-banged GPIO writes (as in
    [drivers/display/ili9486.c](drivers/display/ili9486.c)'s
    `ili9486_send_pixels` fallback), no PIO/pinctrl handoff complexity, but
    slower — measure actual frame time against the PIO path before ruling
    it out.
- [ ] Record the decision and rationale in this file once made.
- [ ] Implement the chosen backend as a proper Zephyr `display` API driver
      wrapping the protocol logic in
      [drivers/display/ili9486.c](drivers/display/ili9486.c).
- [ ] Static-pattern/gradient test on the real 480×320 panel.

**Exit criteria:** backend decided; frame time comparable to (or better than)
the current FreeRTOS build; no visual artifacts during window-set /
pixel-push handoff.

### 1b. Continuous ADC + DMA acquisition

- [ ] Standalone thread that free-runs the ADC into ping-pong DMA buffers at
      10 kHz, bypassing the stock `adc_rpi_pico` driver (it is
      one-shot/round-robin only, no continuous DMA streaming), mirroring
      [hal/hal_adc.c](hal/hal_adc.c)'s ISR logic.
- [ ] Hand completed buffers to a `k_msgq`.
- [ ] Validate sample rate and values via UART dump of peak/RMS/frequency
      against a known signal-generator input.

**Exit criteria:** correct sample rate and values on hardware, no display
needed yet.

---

## Phase 2 — Mechanical RTOS-primitive porting

Can run in parallel with Phase 1. Each item independently testable.

- [ ] Task creation ([app/app.c](app/app.c)) → `K_THREAD_DEFINE` / priorities.
- [ ] `xTaskNotifyFromISR`/`xTaskNotifyWait` in
      [app/tasks/ad_task.c](app/tasks/ad_task.c) → `k_sem`/`k_msgq` handoff
      from the Phase 1b ADC driver.
- [ ] Cross-task `QueueHandle_t` in
      [app/lvgl/screen_update.c](app/lvgl/screen_update.c) → `k_msgq`, or
      drop it in favor of the Zephyr LVGL module's workqueue-driven
      `lv_timer_handler`/`lv_async_call` (re-evaluate whether a custom queue
      is still needed).
- [ ] Once-a-minute RTC software timer → `k_timer`.
- [ ] [hal/hal_rtc.c](hal/hal_rtc.c) → native `raspberrypi,pico-rtc` driver
      via `zephyr/drivers/rtc.h`.
- [ ] PWM backlight in [main.c](main.c) → `raspberrypi,pico-pwm` + Zephyr PWM
      API.
- [ ] Stack-overflow hook in [rtos/hooks.c](rtos/hooks.c) →
      `CONFIG_STACK_SENTINEL` / thread fault handling equivalent.

**Exit criteria:** each primitive swap has a small standalone test (RTC
round-trips a date, PWM backlight dims, a dummy thread survives a
stack-overflow probe) — no full app yet.

---

## Phase 3 — Encoder input + LVGL group navigation

- [ ] Port [drivers/encoder/encoder.c](drivers/encoder/encoder.c) (GPIO IRQ
      edge-detect) onto Zephyr GPIO callbacks.
- [ ] Wire it as an `LV_INDEV_TYPE_ENCODER` input device via
      `CONFIG_LV_Z_ENCODER_INPUT`, replacing the manual
      `lv_indev_create`/`encoder_read_cb` in
      [services/lvgl/lvgl_port.c](services/lvgl/lvgl_port.c).

**Exit criteria:** on the Phase 1a ILI9486 driver, an LVGL demo screen is
navigable with the physical encoder — turn, press, select.

---

## Phase 4 — Real screens, one at a time

- [ ] `scr_boot` — confirms boot flow and
      [app/lvgl/screen_manager.c](app/lvgl/screen_manager.c) state machine
      under Zephyr threading.
- [ ] `scr_menu` — confirms encoder + group focus end-to-end.
- [ ] `scr_datetime`, `scr_settings`, `scr_information` — static/RTC-fed,
      low risk.
- [ ] `scr_oscilloscope` — wire the Phase 1b ADC stream through `chart.c`'s
      scaling into `lv_chart_set_series_ext_y_array`, replacing
      `screen_update_plot_data`'s FreeRTOS-queue path.
- [ ] `scr_fft` — wire [services/dsp/dsp.c](services/dsp/dsp.c) (unchanged)
      through the same pipeline; full acquire → FFT → render loop.

**Exit criteria:** every screen in
[app/lvgl/screens/](app/lvgl/screens) is reachable and functionally
correct on the ILI9486 panel, A/B'd against the FreeRTOS firmware for
waveform accuracy, frequency reading, and responsiveness.

---

## Phase 5 — Devicetree/Kconfig consolidation

- [ ] Move [bsp/rp2350/board_config.h](bsp/rp2350/board_config.h)'s pin
      `#define`s into the `boards/*.overlay` + pinctrl nodes.
- [ ] Port the in-progress [Kconfig](Kconfig) menu (ILI9486 backend choice,
      per-driver pin config) into the app's native Zephyr `Kconfig`; retire
      [cmake/Kconfig.cmake](cmake/Kconfig.cmake).
- [ ] Delete the FreeRTOS kernel tree ([rtos/freertos/](rtos/freertos)) and
      the pico-sdk-direct [hal/](hal) / [drivers/](drivers) trees once
      nothing references them.

**Exit criteria:** `west build -DBOARD=rpi_pico2/rp2350a/m33` builds from the
[target directory structure](#target-directory-structure) with no leftover
FreeRTOS sources in the tree.

---

## Phase 6 — Soak & cutover

- [ ] Run both firmwares side-by-side for a burn-in period (encoder stress,
      long-run RTC drift, sustained waveform capture).
- [ ] Tag the last known-good FreeRTOS commit before removing it, in case a
      regression needs to be bisected back.
- [ ] Remove the FreeRTOS path from `main`.

---

## Risk map (for sequencing/timeboxing decisions)

| Phase | Risk | Notes |
|-------|------|-------|
| 0     | Low | Standard Zephyr bring-up. |
| 1     | High, unpredictable | Timebox as spikes. 1a resolves PIO-vs-SIO before anything else depends on the display; if PIO proves too costly, SIO is the built-in fallback, not an afterthought. |
| 2–4   | Low, mechanical | Predictable 1:1 API mappings. |
| 5     | Bookkeeping | No functional risk. |
