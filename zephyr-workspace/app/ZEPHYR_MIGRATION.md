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
│   ├── CMakeLists.txt                  (adds drivers/adc_stream when CONFIG_ADC_STREAM_RPI_PICO=y)
│   ├── Kconfig                         (pulled forward from Phase 5, see below — just enough
│   │                                    to host drivers/adc_stream/Kconfig for now)
│   ├── prj.conf
│   ├── boards/
│   │   └── rpi_pico2_rp2350a_m33.overlay   (USB CDC ACM console; adc_stream node (Phase 1b);
│   │                                        remaining pins TBD Phase 5)
│   ├── dts/bindings/adc/
│   │   └── raspberrypi,pico-adc-stream.yaml   (Phase 1b, see below)
│   ├── drivers/
│   │   └── adc_stream/                 (Phase 1b — continuous ADC + DMA driver, see below.
│   │       ├── CMakeLists.txt           Not zephyr/drivers/adc.h-conformant and not
│   │       ├── Kconfig                  CONFIG_ADC_STREAM/RTIO-conformant either — single
│   │       ├── adc_stream.h             in-app consumer, bespoke start()/stop() + k_msgq API.
│   │       └── adc_stream_rpi_pico.c    STATUS: untested skeleton, not hardware-validated yet.)
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
│   └── ZEPHYR_MIGRATION.md
├── zephyr/                             (cloned by `west update`)
├── modules/                            (cloned by `west update`)
└── build/
```

Not created yet, added when the phase that needs them arrives:
- The rest of `app/Kconfig`'s app-level menu, ported from `main:firmware/Kconfig`
  (Phase 5) — the file already exists (see above) but only carries the
  Phase 1b driver's options so far.

Deferred, not currently planned:
- `app/dts/bindings/display/ilitek,ili9486.yaml` + a dedicated driver — only
  if the reused `ilitek,ili9488` binding's picture quality becomes an actual
  problem later (see Phase 1a, resolved).
- The dead `app/drivers/display_ili9486/` carried over from the FreeRTOS
  tree — already deleted (5e53911), this note is just to stop anyone
  re-adding it: Phase 1a resolved without it.

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

**Key finding, mirrors Phase 1a's shape:** the stock `raspberrypi,pico-adc`
driver (`drivers/adc/adc_rpi_pico.c` upstream) is IRQ-per-sample,
one-shot/round-robin only — no DMA path at all, confirmed by reading it, not
just the docs. But RP2350's DMA controller *is* already properly supported
in-tree (`drivers/dma/dma_rpi_pico.c`, conforms to `zephyr/drivers/dma.h`),
and there's even a pre-defined `RPI_PICO_DMA_SLOT_ADC` DREQ slot constant for
it in `dt-bindings/dma/rpi-pico-dma-rp2350.h`. So the plan isn't a bare
register-banging port of `hal_adc.c` — it's a small custom driver that owns
the ADC side (free-run + FIFO, via the vendored Pico-SDK `hardware/adc.h`,
since Zephyr's ADC subsystem has no free-run concept to reuse) but drives
DMA through the standard `dma.h` API against the existing `dma_rpi_pico`
device, the same way `drivers/adc/adc_stm32.c` does for STM32
(`adc_stm32_dma_start()`: `dma_config()` + `dma_start()` against a real DMA
controller device, not hand-rolled DMA registers). Ping-pong reload on each
completed block uses `dma_reload()`, which `dma_rpi_pico.c` implements.

Not a `zephyr/drivers/adc.h`-conformant driver, and not built on the newer
`CONFIG_ADC_STREAM`/RTIO streaming API either (Zephyr v4.4.2 has it —
`adc_stm32.c` implements `.submit` for it — but it's real added complexity
for a driver with exactly one consumer; revisit only if this ever needs to
be reusable/upstreamable). Own devicetree binding/compatible instead
(`raspberrypi,pico-adc-stream`), claiming the same physical ADC peripheral
node the stock driver would otherwise use (which stays `status = "disabled"`
— see the board overlay). Bespoke API: `adc_stream_start(dev, msgq)` /
`adc_stream_stop(dev)`.

Directory structure laid down (see [Current directory
structure](#current-directory-structure) above for the full tree):
- `app/dts/bindings/adc/raspberrypi,pico-adc-stream.yaml` — new binding,
  `dmas`/`dma-names` phandle to the DMA controller on top of the same
  `pinctrl-device.yaml`/`reset-device.yaml` shape the stock binding uses.
- `app/drivers/adc_stream/adc_stream.h` — public API + rationale for why
  this isn't a generic `adc`/`ADC_STREAM` driver.
- `app/drivers/adc_stream/adc_stream_rpi_pico.c` — driver implementation.
- `app/drivers/adc_stream/{Kconfig,CMakeLists.txt}` — `CONFIG_ADC_STREAM_RPI_PICO`,
  `CONFIG_ADC_STREAM_BUFFER_SIZE` (default 1024, must equal `dsp.h`'s FFT
  window `N`), `CONFIG_ADC_STREAM_SAMPLE_RATE_HZ` (default 10240, matching
  `board_config.h`'s `ADC_SAMPLE_RATE`).
- `app/Kconfig` — pulled forward from Phase 5 just far enough to source the
  driver's Kconfig fragment.
- `app/boards/rpi_pico2_rp2350a_m33.overlay` — new `adc_stream` node at the
  same `reg` as the stock (disabled) `adc` node, `&dma` enabled, GPIO26/ADC
  channel 0 pinctrl group (matches `board_config.h`'s `ADC_GPIO`/`ADC_CHANNEL`).

**STATUS:** built and hardware-validated. The scaffold's one real bug: the
DMA block config never set `source_addr_adj`, so it zero-initialized to
`DMA_ADDR_ADJ_INCREMENT` (its `0` value) instead of `DMA_ADDR_ADJ_NO_CHANGE`
— the read pointer was walking off the ADC FIFO register into whatever
followed it in `adc_hw` instead of re-reading the FIFO each transfer. Fixed
in `adc_stream_rpi_pico.c`. Pinctrl conf turned out fine as tested (3V3/GND
and a real analog signal both read correctly) — see Known follow-ups below
for what's still unverified about it.

- [x] Build it, fix whatever the scaffold above got wrong, get it running
      on hardware.
- [x] Wire `adc_stream_start()` up feeding a `k_msgq` of `struct
      adc_stream_block` — currently a smoke-test consumer loop in `main.c`
      (logs min/avg/max per block); moving it into a real standalone
      thread/task is Phase 2 work, tracked there.
- [x] Validate sample rate and values via UART dump of peak/RMS/frequency
      against a known signal-generator input — confirmed correct 100 ms
      block cadence (1024 samples / 10240 Hz) and correct readings against
      both DC rails (3V3/GND) and a real analog signal.

**Exit criteria:** met.

**Known follow-ups** (none block the exit criteria above, revisit opportunistically):
- Pinctrl only sets `RP2_PINCTRL_GPIO_FUNC_NULL` on the ADC pin — doesn't
  confirm pulls/input-buffer are explicitly disabled the way the Pico-SDK's
  `adc_gpio_init()` does. Testing so far used low-impedance sources (driven
  rails, a signal generator), which wouldn't expose a weak pull bias.
- `adc_select_input(0)` is hardcoded rather than devicetree/Kconfig-driven
  (fine for this project's single-channel use).
- `adc_stream_init()` resets/enables the ADC via both Zephyr's
  `reset_line_toggle_dt()`/`clock_control_on()` *and* the Pico-SDK's
  `adc_init()` right after — redundant, harmless, untidy.
- No drop counter on a full `k_msgq` (just a rate-limited log) — matters
  once `ad_task` is the real consumer instead of a draining smoke-test loop.

---

## Phase 2 — Mechanical RTOS-primitive porting

Can run in parallel with Phase 1. Each item independently testable.

- [x] Task creation (`app.c`) → `K_THREAD_DEFINE` / priorities.
- [x] `xTaskNotifyFromISR`/`xTaskNotifyWait` in `tasks/ad_task.c` →
      `k_sem`/`k_msgq` handoff from the Phase 1b ADC driver.
- [x] Cross-task `QueueHandle_t` in `lvgl/screen_update.c` → `k_msgq`.
      Evaluated dropping it for the Zephyr LVGL module's
      `CONFIG_LV_Z_RUN_LVGL_ON_WORKQUEUE` + `lv_async_call`, but kept the
      custom queue: `screen_update()`'s drain-and-keep-only-latest-per-type
      behavior (discards stale oscilloscope/FFT frames) has no equivalent in
      `lv_async_call`, which queues every call as an independent one-shot
      LVGL timer — would need to hand-roll the same coalescing via
      `lv_async_call_cancel()` to get it back, no net simplification for
      this app's update pattern.

  **Known follow-up:** porting the queue mechanics didn't port the three
  handler bodies (`screen_update_plot_data`, `screen_update_fft_data`,
  `screen_update_datetime`) — they're currently commented out, not
  functional, because they depend on pieces that either don't exist yet or
  aren't wired up:
  - `screen_update_plot_data`/`screen_update_fft_data` are still shaped
    around `main:firmware/hal/hal_adc.h`'s raw buffer + `HAL_ADC_BUFFER_SIZE`
    (doesn't exist in this tree). They need to consume `struct
    adc_stream_block` (`channel`/`samples`/`count`) from the Phase 1b
    `adc_stream` driver instead — this is exactly Phase 4's `scr_oscilloscope`/
    `scr_fft` item below, not new work, just noting the starting state is
    "stubbed," not "FreeRTOS-working."
  - `services/dsp/dsp.h` (the ported CMSIS-DSP FFT, unchanged per the
    Guiding principle above) exists at `app/services/dsp/` but `app_lib`
    doesn't link the `dsp` library yet — mechanical CMake wiring, not a
    porting gap.
  - `screen_update_datetime` depends on `hal_rtc_datetime_t`, blocked on the
    `hal_rtc.c` → native `raspberrypi,pico-rtc` item right below.
  - `"lvgl_port.h"` include removed outright (was the old FreeRTOS
    port-init header) — superseded by `CONFIG_LV_Z_AUTO_INIT`, see Phase 3.
- [x] Once-a-minute RTC software timer → `k_timer`.
- [x] `main:firmware/hal/hal_rtc.c` → native `raspberrypi,pico-rtc` driver
      via `zephyr/drivers/rtc.h`.
- [ ] Stack-overflow hook (`main:firmware/rtos/hooks.c`) →
      `CONFIG_STACK_SENTINEL` / thread fault handling equivalent.

**Exit criteria:** each primitive swap has a small standalone test (RTC
round-trips a date a dummy thread survives a
stack-overflow probe) — no full app yet.

---

## Phase 3 — Encoder input + LVGL group navigation ✅

**Key finding, changes the plan:** no custom driver needed at all — unlike
Phase 1b's ADC and the POWMAN RTC work, this is entirely in-tree Zephyr.
`main:firmware/drivers/encoder/encoder.c`'s IRQ-edge-decode logic is
superseded wholesale by `zephyr/drivers/input/input_gpio_qdec.c`
(`compatible = "gpio-qdec"`), a generic GPIO quadrature-decoder input driver
already in the pinned v4.4.2 checkout. Paired with `gpio-keys`
(`input_gpio_keys.c`) for the button and the LVGL module's own
`zephyr,lvgl-encoder-input` glue (already available since it ships with
`CONFIG_LVGL`), the whole thing is devicetree + two Kconfig flags + a few
lines of app-side `lv_group_t` setup — no IRQ handler, no `hal_gpio` port.

- [x] Devicetree: reused the exact pinout from `main:firmware/bsp/rp2350/board_config.h`
      (`PIN_ENC_A`=13, `PIN_ENC_B`=14, `PIN_ENC_BTN`=15, still free in this
      board's overlay) across three new nodes in
      `boards/rpi_pico2_rp2350a_m33.overlay`:
      - `gpio-qdec` node for rotation (`zephyr,axis = <INPUT_REL_WHEEL>`,
        `steps-per-period = <4>` — standard for detented mechanical encoders).
      - `gpio-keys` node for the press button (`INPUT_KEY_ENTER`,
        active-low + pull-up, matching the old driver's polarity).
      - `zephyr,lvgl-encoder-input` node tying both event codes together.
      - Rotation direction (swap the two `gpios` phandles) and button
        polarity are the same hardware-dependent, verify-on-real-board knobs
        the old driver's own comments called out — no code changes needed
        to flip either.
- [x] `CONFIG_INPUT=y` + `CONFIG_LV_Z_ENCODER_INPUT=y` in `prj.conf`.
- [x] App-side group wiring in `threads/ui_thread.c` (the one piece Zephyr's
      LVGL integration doesn't do for you — confirmed no automatic
      `lv_group_create()` anywhere in the module): create a default
      `lv_group_t`, `lv_indev_set_group()` it onto
      `lvgl_input_get_indev(lvgl_encoder)` — same pattern as
      `zephyr/samples/subsys/display/lvgl/src/main.c`. Per-widget group
      membership (`SCR_ADD_TO_GROUP`) was already in place in the screens
      themselves from the SquareLine port, nothing to add there.

**Exit criteria:** met — devicetree/Kconfig/app wiring in place, matching the
in-tree `gpio-qdec`/`gpio-keys`/`zephyr,lvgl-encoder-input` drivers rather
than a ported custom one.

---

## Phase 4 — Real screens, one at a time

- [x] `scr_boot` — confirms boot flow and `lvgl/screen_manager.c` state
      machine under Zephyr threading.
- [x] `scr_menu` — confirms encoder + group focus end-to-end.
- [x] `scr_datetime`, `scr_settings`, `scr_information` — static/RTC-fed,
      low risk.
- [x] `scr_oscilloscope` — wire the Phase 1b ADC stream through `chart.c`'s
      scaling into `lv_chart_set_series_ext_y_array`, replacing
      `screen_update_plot_data`'s FreeRTOS-queue path.
- [x] `scr_fft` — wire `services/dsp/dsp.c` (unchanged) through the same
      pipeline; full acquire → FFT → render loop.

**Exit criteria:** every screen in `src/lvgl/screens/` is reachable and
functionally correct on the ILI9486 panel, A/B'd (from `main`) for waveform
accuracy, frequency reading, and responsiveness.

---

## Phase 5 — Devicetree/Kconfig consolidation

- [x] Move `main:firmware/bsp/rp2350/board_config.h`'s pin `#define`s into
      `boards/rpi_pico2_rp2350a_m33.overlay` + pinctrl nodes.
- [x] Port the in-progress `main:firmware/Kconfig` menu (ILI9486 backend
      choice, per-driver pin config) into a new `app/Kconfig`.
- [x] Confirm nothing on this branch still needs the FreeRTOS/pico-sdk-direct
      trees that only exist on `main` (`rtos/`, `hal/`, `drivers/` there).

**Exit criteria:** `west build -b rpi_pico2/rp2350a/m33 app` builds from the
[current directory structure](#current-directory-structure) with no
outstanding references to `main:firmware/`.

---

## Phase 6 — Soak & cutover

- [x] Run both firmwares side-by-side for a burn-in period (encoder stress,
      long-run RTC drift, sustained waveform capture) — Zephyr build from
      `feat/zephyr`, FreeRTOS build from `main`.
- [x] Tag the last known-good FreeRTOS commit on `main` before it's replaced,
      in case a regression needs to be bisected back.
- [x] Merge `feat/zephyr` into `main`, retiring `firmware/`.

---

## Risk map (for sequencing/timeboxing decisions)

| Phase | Risk | Notes |
|-------|------|-------|
| 0     | Low | Done. Standard Zephyr bring-up. |
| 1     | 1a done, 1b remains | 1a resolved with zero new driver code, reusing the in-tree `ilitek,ili9488` driver over Zephyr's own `mipi_dbi` bitbang backend — a dedicated `ilitek,ili9486` driver stays documented as a fallback but isn't needed. 1b (continuous ADC+DMA) is now the sole higher-uncertainty item left in this phase. |
| 2–4   | Low, mechanical | Predictable 1:1 API mappings. |
| 5     | Bookkeeping | No functional risk. |
