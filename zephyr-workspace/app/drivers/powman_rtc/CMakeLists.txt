# Copyright (c) 2026 Fabrizio Carlassara
# SPDX-License-Identifier: Apache-2.0

add_library(powman_rtc STATIC powman_rtc.c)
target_include_directories(powman_rtc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(powman_rtc PUBLIC zephyr_interface)
add_dependencies(powman_rtc zephyr_generated_headers)

target_sources(powman_rtc PRIVATE
  ${ZEPHYR_HAL_RPI_PICO_MODULE_DIR}/src/rp2_common/hardware_powman/powman.c
  ${ZEPHYR_HAL_RPI_PICO_MODULE_DIR}/src/rp2_common/pico_aon_timer/aon_timer.c
  ${ZEPHYR_HAL_RPI_PICO_MODULE_DIR}/src/common/pico_util/datetime.c
)
target_include_directories(powman_rtc PRIVATE
  ${ZEPHYR_HAL_RPI_PICO_MODULE_DIR}/src/rp2_common/hardware_powman/include
  ${ZEPHYR_HAL_RPI_PICO_MODULE_DIR}/src/rp2_common/pico_aon_timer/include
  ${ZEPHYR_HAL_RPI_PICO_MODULE_DIR}/src/common/pico_util/include
)