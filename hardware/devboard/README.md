# RP2350B Development Board

**Rev 1** — 2026-08-18 · Fabrizio Carlassara · UTN FRA, Departamento de Ingeniería Electrónica, Cátedra de Proyecto Final

KiCad project: [`devboard.kicad_pro`](devboard.kicad_pro) · Schematic: [`devboard.kicad_sch`](devboard.kicad_sch) · PCB: [`devboard.kicad_pcb`](devboard.kicad_pcb)

## Overview

A minimal development/breakout board built around the **[Core2350B0](https://www.waveshare.com/wiki/Core2350B0)** module (an RP2350B‑QFN80 SoM), with a parallel-interface **[3.5″ ILI9486 TFT + SD card shield](https://www.lcdwiki.com/3.5inch_Arduino_Display-UNO)**, a rotary encoder for UI input, 8 analog input headers, an I²C header, an SWD debug header, and a selectable power path that can run from USB, an external 5V source/boost module, or a Li‑Po/Li-ion battery.

- **Board:** 2-layer, 55.45 mm × 64.95 mm
- **MCU module:** U1 — [Core2350B0](https://www.waveshare.com/wiki/Core2350B0) (RP2350B in QFN-80), custom project symbol/footprint
- **Display:** U4 — [ILI9486 3.5″ TFT shield](https://www.lcdwiki.com/3.5inch_Arduino_Display-UNO) (8-bit parallel/8080 interface) with onboard microSD slot (SPI)
- **Power:** Li‑Po/Li‑ion input, USB VBUS, or external 5V/boost module → selectable VSYS → AMS1117-3.3 → +3V3
- **Input:** SW1 rotary encoder with integrated push-button

## Components

| Ref | Part | Function |
|---|---|---|
| U1 | [Core2350B0](https://www.waveshare.com/wiki/Core2350B0) (RP2350B QFN‑80) | Main MCU module |
| U2 | [AMS1117CD‑3.3](https://www.advanced-monolithic.com/pdf/ds1117.pdf) (SOT‑223) | Linear regulator, VSYS → +3V3 |
| U4 | [ILI9486 3.5″ TFT Shield](https://www.lcdwiki.com/3.5inch_Arduino_Display-UNO) | Display + resistive-touch‑shield form factor, 8-bit parallel LCD bus, onboard microSD (SPI) |
| U5 | 3-pin header, socket for [PCBoard.ca Mini Boost Converter 5V](https://www.pcboard.ca/mini-boost-converter-5v) | Plug-in step-up module, boosts VBAT (~3.7V Li‑Po) to a regulated +5V |
| SW1 | Rotary encoder w/ push switch | UI input — quadrature A/B + click |
| R1, R2, R3 | 10 kΩ (0805) | Pull-ups to +3V3 for encoder A, B, and switch lines |
| C1, C5 | 10 µF MLCC | Decoupling on VSYS and +3V3 |
| JP1 | 3-pin jumper (bridged 1‑2 by default) | VSYS power-source select: **+5V** (external header/boost module) vs **VBUS** (USB) |
| J4 | 3-pin header | SWD debug (SWCLK, GND, SWDIO) |
| J5 | 4-pin header | I²C breakout (+3V3, GPIO8, GPIO9, GND) |
| J6–J13 | 2-pin headers | ADC0–ADC7 analog input breakouts (signal + GND) |
| J14 | 2-pin header | Battery input (VBAT, GND) — single-cell Li‑Po/Li‑ion, no onboard charge management |

> ⚠️ U5 and J14 use generic 2.54 mm pin-header footprints as placeholders — U5 is a socket for the plug-in boost module linked above (not a fixed regulator IC on this PCB), and J14 is a bare 2-pin header rather than a JST-PH battery connector. J4 is likewise a standard 2.54 mm SWD header (a schematic note calls it a "JST Debug Connector", but no JST part is actually used).

## Power architecture

```
Battery (Li-Po/Li-ion, ~3.7V) ──► J14 ──► U5 (external boost module) ──► +5V ─┐
                                                                               ├─► JP1 ──► VSYS ──► U2 (AMS1117-3.3) ──► +3V3
                                       USB VBUS (from U1's onboard USB) ──────┘
```

- **JP1** selects what feeds VSYS: bridge pins **1-2** to power VSYS from the external **+5V** rail (boost module output), or bridge **2-3** to power VSYS from USB **VBUS** instead.
- **VSYS** feeds the AMS1117-3.3 (U2), which produces the board's **+3V3** rail for the RP2350B and all logic.
- **VSYS** also feeds the TFT shield's 5V pin directly (U4 pad 19), so the display runs from the ~5V rail while its logic/control lines are driven at 3.3V from the RP2350B GPIOs.
- U1's own **USB_DP/USB_DM** pins are not routed to this board — the Core2350B0 module has its own USB connector; only its **VBUS** pin is brought out here for power-source selection.
- U1's **RUN**, **BOOTSEL**, **3V3_EN**, and **ADC_VREF** pins are also not routed on this board (available on the module for future use — e.g. reset/BOOTSEL buttons, external ADC reference).

## Core2350B0 (RP2350B) pin usage

| GPIO | Net | Connected to | Role |
|---|---|---|---|
| GPIO0 | /GPIO0 | U4 pad 8 (LCD_D0) | TFT parallel data bus |
| GPIO1 | /GPIO1 | U4 pad 7 (LCD_D1) | TFT parallel data bus |
| GPIO2 | /GPIO2 | U4 pad 14 (LCD_D2) | TFT parallel data bus |
| GPIO3 | /GPIO3 | U4 pad 13 (LCD_D3) | TFT parallel data bus |
| GPIO4 | /GPIO4 | U4 pad 12 (LCD_D4) | TFT parallel data bus |
| GPIO5 | /GPIO5 | U4 pad 11 (LCD_D5) | TFT parallel data bus |
| GPIO6 | /GPIO6 | U4 pad 10 (LCD_D6) | TFT parallel data bus |
| GPIO7 | /GPIO7 | U4 pad 9 (LCD_D7) | TFT parallel data bus |
| GPIO8 | /GPIO8 | J5 pad 2 | I²C header (SDA/SCL — function assigned in firmware) |
| GPIO9 | /GPIO9 | J5 pad 3 | I²C header (SDA/SCL — function assigned in firmware) |
| GPIO10 | /GPIO10 | *(none)* | Not routed off-module — reserved/available |
| GPIO11 | /GPIO11 | *(none)* | Not routed off-module — reserved/available |
| GPIO12 | /GPIO12 | U4 pad 4 (SD_D0) | microSD SPI — MISO |
| GPIO13 | /GPIO13 | U4 pad 6 (SD_SS) | microSD SPI — chip select |
| GPIO14 | /GPIO14 | U4 pad 3 (SD_SCK) | microSD SPI — clock |
| GPIO15 | /GPIO15 | U4 pad 5 (SD_DI) | microSD SPI — MOSI |
| GPIO16 | /GPIO16 | U4 pad 23 (LCD_RD) | TFT control — read strobe |
| GPIO17 | /GPIO17 | U4 pad 24 (LCD_WR) | TFT control — write strobe |
| GPIO18 | /GPIO18 | U4 pad 25 (LCD_RS) | TFT control — register/data select |
| GPIO19 | /GPIO19 | U4 pad 26 (LCD_CS) | TFT control — chip select |
| GPIO20 | /GPIO20 | U4 pad 27 (LCD_RST) | TFT control — reset |
| GPIO21–GPIO26 | /GPIO21…/GPIO26 | *(none)* | Not routed off-module — reserved/available |
| GPIO27 | /GPIO27 | R1 (10k → +3V3), SW1 pad A | Rotary encoder — channel A (pulled up) |
| GPIO28 | /GPIO28 | *(none)* | Not routed off-module — reserved/available |
| GPIO29 | /GPIO29 | R2 (10k → +3V3), SW1 pad B | Rotary encoder — channel B (pulled up) |
| GPIO30 | /GPIO30 | R3 (10k → +3V3), SW1 pad S1 | Rotary encoder — push switch (pulled up) |
| GPIO31–GPIO39 | /GPIO31…/GPIO39 | *(none)* | Not routed off-module — reserved/available |
| GPIO40 | /GPIO40_ADC0 | J6 | ADC0 analog input header |
| GPIO41 | /GPIO41_ADC1 | J7 | ADC1 analog input header |
| GPIO42 | /GPIO42_ADC2 | J8 | ADC2 analog input header |
| GPIO43 | /GPIO43_ADC3 | J9 | ADC3 analog input header |
| GPIO44 | /GPIO44_ADC4 | J10 | ADC4 analog input header |
| GPIO45 | /GPIO45_ADC5 | J11 | ADC5 analog input header |
| GPIO46 | /GPIO46_ADC6 | J12 | ADC6 analog input header |
| GPIO47 | /GPIO47_ADC7 | J13 | ADC7 analog input header |
| SWCLK | /SWCLK | J4 pad 1 | SWD debug clock |
| SWDIO | /SWD | J4 pad 3 | SWD debug data |
| VBUS | VBUS | JP1 pad 3 | USB 5V sense/power (module's own USB port) |
| 3V3 | +3V3 | U2 output, C5, R1–R3, J5 pad 1 | Regulated logic supply |
| GND | GND | Board ground | Common ground |
| RUN | *(unconnected)* | — | Reset — not routed on this board |
| BOOTSEL | *(unconnected)* | — | BOOTSEL — not routed on this board |
| 3V3_EN | *(unconnected)* | — | Regulator enable — not routed on this board |
| ADC_VREF | *(unconnected)* | — | External ADC reference — not routed on this board |
| USB_DP / USB_DM | *(unconnected)* | — | USB data — handled by module's onboard USB connector |

## Connector reference

| Connector | Pins | Notes |
|---|---|---|
| J4 (SWD) | 1: SWCLK · 2: GND · 3: SWDIO | Standard 2.54 mm 3-pin debug header |
| J5 (I2C) | 1: +3V3 · 2: GPIO8 · 3: GPIO9 · 4: GND | Pin function (SDA/SCL) set in firmware |
| J6–J13 (ADC0–ADC7) | 1: ADCx signal · 2: GND | One 2-pin header per analog channel |
| J14 (Battery) | 1: VBAT · 2: GND | Single-cell Li‑Po/Li‑ion, no charge management on-board |
| U5 (Boost) | 1: VBAT in · 2: GND · 3: +5V out | Socket for external boost module |
| JP1 (Power select) | 1: +5V · 2: VSYS · 3: VBUS | Bridge 1‑2: power from external +5V/boost; bridge 2‑3: power from USB |

## Open items / things to double check before fabrication

- No charge-management IC for the Li‑Po/Li‑ion battery on J14 — charging must be handled externally.
- Confirm current rating of the boost module against the board's total load (display backlight + MCU) before relying on it.
- GPIO10, 11, 21–26, 28, and 31–39 are unused on this revision — available for future expansion.
- U1's RUN/BOOTSEL are not brought out — flashing relies on the Core2350B0 module's own USB/BOOTSEL, or SWD via J4.
