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
add_subdirectory_ifdef(CONFIG_ADC_STREAM_RPI_PICO drivers/adc_stream)
target_include_directories_ifdef(CONFIG_ADC_STREAM_RPI_PICO app_lib PRIVATE drivers/adc_stream)
target_link_libraries_ifdef(CONFIG_ADC_STREAM_RPI_PICO app_lib PRIVATE adc_stream)
# Link POWMAN RTC driver to application
add_subdirectory_ifdef(CONFIG_POWMAN_RTC_RPI_PICO drivers/powman_rtc)
target_include_directories_ifdef(CONFIG_POWMAN_RTC_RPI_PICO app_lib PRIVATE drivers/powman_rtc)
target_link_libraries_ifdef(CONFIG_POWMAN_RTC_RPI_PICO app_lib PRIVATE powman_rtc)
# Link MCP4XXX digital potentiometer driver to application. Also exposed to
# the `app` target (not just app_lib) so main.c can `#include "mcp4xxx.h"`
# directly, the same way drivers/adc_stream is above.
add_subdirectory_ifdef(CONFIG_MCP4XXX drivers/mcp4xxx)
target_include_directories_ifdef(CONFIG_MCP4XXX app PRIVATE drivers/mcp4xxx)
target_include_directories_ifdef(CONFIG_MCP4XXX app_lib PRIVATE drivers/mcp4xxx)
target_link_libraries_ifdef(CONFIG_MCP4XXX app_lib PRIVATE mcp4xxx)

# CMSIS-DSP FFT wrapper (RTOS-agnostic, ported unchanged from the FreeRTOS
# tree — see ZEPHYR_MIGRATION.md's Guiding principle)
add_subdirectory_ifdef(CONFIG_CMSIS_DSP services/dsp)
target_include_directories_ifdef(CONFIG_CMSIS_DSP app_lib PRIVATE services/dsp)
target_link_libraries_ifdef(CONFIG_CMSIS_DSP app_lib PRIVATE dsp)