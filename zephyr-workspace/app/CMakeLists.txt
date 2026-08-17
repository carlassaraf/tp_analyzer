cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED)
project(tp_analyzer)

target_sources(app PRIVATE
  src/main.c
)

add_subdirectory(src)
target_link_libraries(app PRIVATE app_lib)

# So main.c can `#include "adc_stream.h"` for the app-local driver's public
# API (adc_stream_start()/adc_stream_stop()) without reaching into
# drivers/adc_stream/ with a relative path.
target_include_directories(app PRIVATE drivers/adc_stream)

# Phase 1b (see ZEPHYR_MIGRATION.md): app-local continuous ADC + DMA driver.
# See drivers/adc_stream/CMakeLists.txt for why the target_link_libraries()
# below is required, not just add_subdirectory().
add_subdirectory_ifdef(CONFIG_ADC_STREAM_RPI_PICO drivers/adc_stream)
target_link_libraries_ifdef(CONFIG_ADC_STREAM_RPI_PICO app PRIVATE adc_stream)