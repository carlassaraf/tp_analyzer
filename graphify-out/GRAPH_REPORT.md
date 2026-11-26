# Graph Report - tp_analyzer  (2026-11-25)

## Corpus Check
- 183 files · ~209,478 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 92 file(s) not represented in the graph (top: (none) 19, .kicad_mod 12, .asc 10)
- WARNING: 92 of 275 files seen (33%) have no supported extractor and are missing from this graph entirely. It may look complete while representing only part of the codebase.

## Summary
- 717 nodes · 1057 edges · 42 communities (33 shown, 9 thin omitted)
- Extraction: 86% EXTRACTED · 14% INFERRED · 0% AMBIGUOUS · INFERRED: 143 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- LVGL Style & Anim Types
- LVGL Animations
- UI Helpers
- Analog PCB Revision
- Zephyr Subsystem Headers
- DSP & Math Includes
- Device State & App Run
- ADC Stream Bindings
- Product Block Architecture
- Firmware Threads & Peripherals
- CMake Libraries
- MCP42xx Bindings
- MCP41xx Bindings
- Components & Screen Manager
- ADC Pico Driver
- Boot Screen & Navigation
- UI Component Events
- AGC Design Decisions
- Screen Update Queue
- Architecture & Market Docs
- Voltage/Current Channels
- West Manifest
- BOM Export Script
- Gain Stage & LTspice
- RP2350B Devboard
- Image Descriptors
- DSP Algorithms & Competitors
- AGC Diagram
- Antialiasing Filter
- Powman RTC Binding
- Power Architecture
- Producer-Consumer Threads
- Digital Board & Peak Detectors
- Firmware Status & Risks
- Top Bar Hook
- Graphify Rules
- Power & SD Storage
- Block Diagram Fig.1

## God Nodes (most connected - your core abstractions)
1. `ui_object_set_themeable_style_property()` - 18 edges
2. `_ui_screen_change()` - 15 edges
3. `animation_move_to_side()` - 12 edges
4. `Digital board README (peak ADC and IO expander)` - 12 edges
5. `ui_contTopBar_create()` - 11 edges
6. `Analog front end PCB revision doc` - 10 edges
7. `screen_update_plot_data()` - 9 edges
8. `screen_update_fft_data()` - 9 edges
9. `ui_init()` - 9 edges
10. `ui_destroy()` - 9 edges

## Surprising Connections (you probably didn't know these)
- `Auto-rango en firmware (umbrales 1,3 V / 0,6 V)` --semantically_similar_to--> `Histeresis AGC y congelamiento de ganancia por bloque`  [INFERRED] [semantically similar]
  hardware/analog/canal_corriente.md → docs/architecture.md
- `Voltage channel (6x47k, ZMPT101B, burden 100 ohm, variable gain)` --semantically_similar_to--> `T1 ZMPT101B voltage transformer`  [INFERRED] [semantically similar]
  simulations/README.md → hardware/analog/revision_pcb.md
- `BAT54S protection diode` --semantically_similar_to--> `BAT54S ADC protection replaces Zener BZV55C3V3`  [INFERRED] [semantically similar]
  hardware/bom_pedido_elemon.md → simulations/README.md
- `Canal de corriente (burden 15 ohm, filtro RF, ganancia)` --implements--> `Sensado de corriente SCT-013-000`  [INFERRED]
  hardware/analog/calculos_front_end.md → docs/architecture.md
- `Etapa de ganancia MCP41010 como divisor (G=256/D)` --implements--> `Etapa de ganancia LM358 + MCP41010 (G=256/D)`  [INFERRED]
  hardware/analog/calculos_front_end.md → docs/architecture.md

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Cadena de senal del canal analogico (burden, filtro RF, ganancia, MFB, proteccion ADC)** — hardware_analog_calculos_front_end_current_channel, hardware_analog_calculos_front_end_gain_stage, hardware_analog_calculos_front_end_mfb_filter, hardware_analog_calculos_front_end_adc_protection [EXTRACTED 1.00]
- **Flujo productor-cola-consumidor del firmware** — docs_architecture_adc_stream_driver, docs_architecture_ad_thread, docs_architecture_screen_update_msgq, docs_architecture_ui_thread, docs_architecture_screen_manager [EXTRACTED 1.00]
- **Lazo AGC: etapa de ganancia, detectores de pico, MCP3204, MCP23017 y firmware** — docs_architecture_mcp41010_gain_stage, docs_architecture_peak_detectors, docs_architecture_mcp3204, docs_architecture_mcp23017, docs_architecture_hysteresis_block_freeze [EXTRACTED 0.95]
- **Digital board SPI and chip-select chain** — hardware_digital_readme_mcp23017, hardware_digital_readme_mcp3204, hardware_digital_readme_mcp41010, hardware_digital_readme_spi_series_resistors [EXTRACTED 1.00]
- **Simulated analog front end signal chain** — simulations_readme_current_channel, simulations_readme_voltage_channel, simulations_readme_peak_detector, simulations_readme_vbias_buffer, simulations_readme_bat54s_protection [EXTRACTED 1.00]
- **Devboard selectable power path** — hardware_devboard_readme_boost_module, hardware_devboard_readme_jp1_power_select, hardware_devboard_readme_ams1117 [EXTRACTED 1.00]
- **Optional driver/service libraries linked into app_lib** — zephyr_workspace_app_drivers_adc_stream_cmakelists_adc_stream, zephyr_workspace_app_drivers_powman_rtc_cmakelists_powman_rtc, zephyr_workspace_app_drivers_mcp4xxx_cmakelists_mcp4xxx, zephyr_workspace_app_services_dsp_cmakelists_dsp [EXTRACTED 1.00]
- **Producers feeding screen_update_msgq** — docs_assets_firmware_producer_consumer_ad_thread, docs_assets_firmware_producer_consumer_rtc_timer, docs_assets_firmware_producer_consumer_screen_update_msgq, docs_assets_firmware_producer_consumer_ui_thread [EXTRACTED 1.00]
- **Power chain from phases to system rail with battery backup** — docs_assets_power_architecture_phases, docs_assets_power_architecture_rectifier, docs_assets_power_architecture_smps, docs_assets_power_architecture_system_rail, docs_assets_power_architecture_battery [EXTRACTED 1.00]
- **AGC feedback loop components** — docs_assets_agc_diagram_lm358_mcp41010, docs_assets_agc_diagram_peak_detector, docs_assets_agc_diagram_mcp3204 [EXTRACTED 1.00]

## Communities (42 total, 9 thin omitted)

### Community 0 - "LVGL Style & Anim Types"
Cohesion: 0.05
Nodes (45): ui_contTopBar_create(), ui_scrBoot_screen_destroy(), ui_scrBoot_screen_init(), ui_event_scrDatetime_btnBack(), ui_scrDatetime_screen_destroy(), ui_scrDatetime_screen_init(), ui_event_scrFFT_cntBack(), ui_scrFFT_screen_destroy() (+37 more)

### Community 1 - "LVGL Animations"
Cohesion: 0.06
Nodes (46): animation_move_to_side(), ui_chart_bind_ext_array(), ui_chart_push_data(), ui_chart_push_float_data(), screen_update_fft_data(), screen_update_plot_data(), change_signal_cb(), change_span_cb() (+38 more)

### Community 3 - "UI Helpers"
Cohesion: 0.08
Nodes (39): scr_unloaded_delete_cb(), _ui_anim_callback_free_user_data(), _ui_anim_callback_get_height(), _ui_anim_callback_get_image_angle(), _ui_anim_callback_get_image_frame(), _ui_anim_callback_get_image_zoom(), _ui_anim_callback_get_opacity(), _ui_anim_callback_get_width() (+31 more)

### Community 4 - "Analog PCB Revision"
Cohesion: 0.08
Nodes (42): Analog front end PCB revision doc, Pre-assembly checklist and assembly order, Home fabrication (toner transfer, FeCl3, no plated holes), Mains (HV) zone with reinforced isolation, J10 SCT-013 jack CUI SJ1-3523N, J1 mains input solder pads, J2 analog board 2x7 IDC connector, T1 ZMPT101B voltage transformer (+34 more)

### Community 5 - "Zephyr Subsystem Headers"
Cohesion: 0.09
Nodes (22): device, mcp4xxx_decrement(), mcp4xxx_get(), mcp4xxx_hw_shutdown(), mcp4xxx_increment(), mcp4xxx_read_reg(), mcp4xxx_read_status(), mcp4xxx_read_tcon() (+14 more)

### Community 6 - "DSP & Math Includes"
Cohesion: 0.09
Nodes (12): dsp_fft_init(), dsp_fft_run(), dsp_get_frequency_bins(), datetime_click_cb(), datetime_field_for_obj(), datetime_field_update_labels(), datetime_lock_input_group(), datetime_pop_encoder_diff() (+4 more)

### Community 7 - "Device State & App Run"
Cohesion: 0.10
Nodes (14): app_run(), dev_state_get(), dev_state_init(), dev_state_kick_activity(), dev_state_set_off_timeout(), dev_state_set_screen_timeout(), dev_state_set_timeout_cb(), encoder_activity_cb() (+6 more)

### Community 8 - "ADC Stream Bindings"
Cohesion: 0.07
Nodes (27): const, child-binding, description, properties, required, compatible, description, description (+19 more)

### Community 10 - "Product Block Architecture"
Cohesion: 0.09
Nodes (22): Fig. 3 AGC control loop diagram, AGC block (8x MCP41010 + LM358, peak detectors, 2x MCP3204, MCP23017 CS), Universal analog frontend (4x ZMPT101B + 4x SCT-013), Local interface (TFT 3.5in ILI9486 + rotary encoder), Power supply block (3-phase rectifier + SMPS + Li-ion battery power path), RP2350B acquisition and processing (DMA ping-pong, CMSIS-DSP, topology detection, MCSA), Storage (microSD FATFS CSV logs + RTC), Three-phase grid under measurement (L1, L2, L3, N) (+14 more)

### Community 11 - "Firmware Threads & Peripherals"
Cohesion: 0.11
Nodes (18): ad_thread (priority 2), adc_msgq (k_msgq of adc_stream_block), adc_stream driver (free-run ADC + DMA, app-local), dev_state (screen_timeout / off_timeout k_timers), Display ILI9486 (mipi_dbi bitbang 480x320), Encoder (gpio-qdec + gpio-keys to LVGL indev), rtc_timer (k_timer 60 s), screen_update_msgq (depth 10, keeps last msg per command/channel) (+10 more)

### Community 12 - "CMake Libraries"
Cohesion: 0.13
Nodes (18): app CMakeLists (tp_analyzer project), adc_stream static library, mcp4xxx static library, powman_rtc static library, mcp4xxx_sample application, dsp static library, app_lib static library, ui static library (SquareLine generated) (+10 more)

### Community 13 - "MCP42xx Bindings"
Cohesion: 0.11
Nodes (17): compatible, description, include, description, enum, required, type, properties (+9 more)

### Community 15 - "MCP41xx Bindings"
Cohesion: 0.13
Nodes (14): compatible, description, include, description, enum, required, type, properties (+6 more)

### Community 16 - "Components & Screen Manager"
Cohesion: 0.22
Nodes (8): ui_topbar_update_datetime(), destroy_timer_cb(), screen_manager_get_active_screen(), screen_manager_step(), screen_manager_update_datetime(), screen_update_datetime(), scr_datetime_deinit(), scr_datetime_update_datetime()

### Community 17 - "ADC Pico Driver"
Cohesion: 0.18
Nodes (4): adc_stream_dma_cb(), adc_stream_start(), other_buf(), ad_thread()

### Community 18 - "Boot Screen & Navigation"
Cohesion: 0.18
Nodes (3): screen_manager_go_to(), timer_cb(), on_dev_state_timeout()

### Community 19 - "UI Component Events"
Cohesion: 0.20
Nodes (4): del_component_child_event_cb(), get_component_child_event_cb(), ui_comp_get_child(), ui_anim_user_data_t

### Community 20 - "AGC Design Decisions"
Cohesion: 0.22
Nodes (9): Driver adc_stream (DMA ping-pong), Control automatico de ganancia (AGC), Histeresis AGC y congelamiento de ganancia por bloque, MCP23017 (expansor de CS por I2C), Etapa de ganancia LM358 + MCP41010 (G=256/D), Driver app-local MCP4xxx (decision), RP2350B en SoM Core2350B0, PGA MCP6S21 (diseno original de AGC) (+1 more)

### Community 21 - "Screen Update Queue"
Cohesion: 0.27
Nodes (6): screen_manager_init(), screen_update(), screen_update_cmd_push(), screen_update_init(), rtc_timer_cb(), ui_thread()

### Community 22 - "Architecture & Market Docs"
Cohesion: 0.25
Nodes (9): Arquitectura del Proyecto - Analizador Trifasico (TP Analyzer), Conectores jack 3.5 mm / GX12 (seguridad TC), Sensado de corriente SCT-013-000, Estudio de Mercado y Diseno de Analizador Trifasico, Front end analogico: calculo y justificacion de componentes, Desacople y lista de componentes, Canal de medicion de corriente: diseno y justificacion, Placa analogica (README) (+1 more)

### Community 23 - "Voltage/Current Channels"
Cohesion: 0.22
Nodes (9): Neutro virtual resistivo (4x 1 MOhm), Sensado de tension ZMPT101B, Canal de corriente (burden 15 ohm, filtro RF, ganancia), Referencia VBIAS 1,65 V (divisor + seguidor U1A), Canal de tension (6x47k, burden 100 ohm, ZMPT101B), Burden RB = 15 ohm, Modelo del TC SCT-013-000 (Lm 5,5 H, Rw 100 ohm) y error de fase, Seguidor VBIAS U3 con 47 ohm + 1 uF (+1 more)

### Community 24 - "West Manifest"
Cohesion: 0.22
Nodes (8): name-allowlist, manifest, projects, version, import, name, revision, url

### Community 26 - "Gain Stage & LTspice"
Cohesion: 0.25
Nodes (8): Etapa de ganancia MCP41010 como divisor (G=256/D), Verificacion por simulacion LTspice, Tabla de desfasajes por armonica y calibracion, Presupuesto de error a 50 Hz y calibracion, Etapa de ganancia U1 + MCP41010 (no inversor, divisor a VBIAS), Limites del LM358 a 5 V (excursion 0,25-3,05 V, G max 32), Cuidados MCP41010 (D>=8, sin shutdown), Pull-downs 1 kOhm contra distorsion de cruce del LM358

### Community 27 - "RP2350B Devboard"
Cohesion: 0.29
Nodes (8): RP2350B Development Board README, AMS1117-3.3 regulator (U2), U5 PCBoard.ca mini boost converter 5V socket, Core2350B0 RP2350B module (U1), ILI9486 3.5in TFT shield with microSD (U4), JP1 VSYS source select jumper, Devboard power architecture (VBAT/USB/5V to VSYS to 3V3), SW1 rotary encoder

### Community 28 - "Image Descriptors"
Cohesion: 0.25
Nodes (6): _ui_anim_user_data_t, imgset, imgset_size, target, val, _ui_screen_delete()

### Community 29 - "DSP Algorithms & Competitors"
Cohesion: 0.29
Nodes (6): Algoritmos DSP (FFT, RMS, THD, MCSA), MCSA: analisis de firma de corriente del motor, Deteccion automatica de topologia estrella/triangulo, Fluke 1777, Hioki PQ3198, Megger MPQ1000

### Community 30 - "AGC Diagram"
Cohesion: 0.33
Nodes (5): LM358 + MCP41010 variable gain stage (G=256/D, SPI), 2x MCP3204 peak ADC (SPI), Passive peak detector (from secondary), RP2350B internal ADC (full waveform, 8 channels), TV signal / current clamp (voltage or current)

### Community 31 - "Antialiasing Filter"
Cohesion: 0.29
Nodes (6): Proteccion de ADC con BAT54S (no Zener), Analisis de pliegue desde 8690 Hz (fs 10240 Hz), Filtro antialiasing MFB 2do orden (f0 2,61 kHz, Q 0,755), Puntos abiertos del front end, Propuesta: muestrear a 20480 Hz y diezmar, Filtro MFB U2 de valores fijos

### Community 32 - "Powman RTC Binding"
Cohesion: 0.29
Nodes (6): compatible, description, include, properties, reg, required

### Community 33 - "Power Architecture"
Cohesion: 0.33
Nodes (6): Li-ion 18650 battery with continuous power path management, Phase inputs L1, L2, L3, Three-phase rectifier bridge (tolerates loss of 1 or 2 phases), Safe shutdown sequence (display off, close SD files, log to EVENTOS.CSV), Ultra-wide-input switching supply, System rail (MCU RP2350B + display + AGC frontend)

### Community 35 - "Producer-Consumer Threads"
Cohesion: 0.40
Nodes (5): ad_thread (prioridad 2), screen_manager (maquina de estados de pantallas), screen_update_msgq (coalescing por comando/canal), ui_thread (prioridad 1, LVGL), Interfaz de usuario (TFT ILI9486 + encoder)

### Community 36 - "Digital Board & Peak Detectors"
Cohesion: 0.50
Nodes (5): Placa digital (hub IDC 2x13 al RP2350B), MCP3204 (ADC SPI de detectores de pico), Detectores de pico pasivos, ADS1115 externo para detectores de pico (diseno original), Detector de pico pasivo de media onda (BAT54, 470 nF, 1 MOhm)

### Community 37 - "Firmware Status & Risks"
Cohesion: 0.67
Nodes (3): Estado de implementacion vs diseno objetivo, Riesgos y decisiones abiertas, Particion de firmware en dos nucleos del RP2350B

## Knowledge Gaps
- **74 isolated node(s):** `imgset`, `imgset_size`, `val`, `Conectores jack 3.5 mm / GX12 (seguridad TC)`, `RP2350B en SoM Core2350B0` (+69 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 278 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **9 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `_ui_anim_user_data_t` connect `Image Descriptors` to `UI Component Events`?**
  _High betweenness centrality (0.010) - this node is a cross-community bridge._
- **Are the 9 inferred relationships involving `ui_object_set_themeable_style_property()` (e.g. with `ui_contTopBar_create()` and `ui_scrBoot_screen_init()`) actually correct?**
  _`ui_object_set_themeable_style_property()` has 9 INFERRED edges - model-reasoned connections that need verification._
- **What connects `imgset`, `imgset_size`, `val` to the rest of the system?**
  _74 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `LVGL Style & Anim Types` be split into smaller, more focused modules?**
  _Cohesion score 0.05300207039337474 - nodes in this community are weakly interconnected._
- **Why does `_ui_screen_change()` connect `LVGL Style & Anim Types` to `UI Helpers`?**
  _High betweenness centrality (0.009) - this node is a cross-community bridge._
- **Are the 12 inferred relationships involving `_ui_screen_change()` (e.g. with `ui_event_scrDatetime_btnBack()` and `ui_event_scrFFT_cntBack()`) actually correct?**
  _`_ui_screen_change()` has 12 INFERRED edges - model-reasoned connections that need verification._
- **Should `LVGL Animations` be split into smaller, more focused modules?**
  _Cohesion score 0.06001984126984127 - nodes in this community are weakly interconnected._