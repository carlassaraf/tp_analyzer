# Copyright (c) 2026 Fabrizio Carlassara
# SPDX-License-Identifier: Apache-2.0

add_library(adc_stream STATIC adc_stream_rpi_pico.c)
target_include_directories(adc_stream PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(adc_stream PUBLIC zephyr_interface)
add_dependencies(adc_stream zephyr_generated_headers)