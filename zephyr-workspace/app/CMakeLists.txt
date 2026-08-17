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