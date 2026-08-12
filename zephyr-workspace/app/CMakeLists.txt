cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED)
project(tp_analyzer)

target_sources(app PRIVATE
  src/main.c
  src/task
)

# Phase 1b (see ZEPHYR_MIGRATION.md): app-local continuous ADC + DMA driver.
add_subdirectory_ifdef(CONFIG_ADC_STREAM_RPI_PICO drivers/adc_stream)