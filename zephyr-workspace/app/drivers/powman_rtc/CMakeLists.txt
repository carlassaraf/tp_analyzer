# Copyright (c) 2026 Fabrizio Carlassara
# SPDX-License-Identifier: Apache-2.0

add_library(powman_rtc STATIC powman_rtc.c)
target_include_directories(powman_rtc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(powman_rtc PUBLIC zephyr_interface)
add_dependencies(powman_rtc zephyr_generated_headers)

# picolibc only declares localtime_r()/gmtime_r() (used by the vendored
# pico_util/datetime.c below, via pico_localtime_r()) when POSIX visibility
# is >= 200809 — off by default under Zephyr, unlike whatever toolchain the
# original Pico-SDK build used. Scoped to this target only, not the whole app.
target_compile_definitions(powman_rtc PRIVATE _POSIX_C_SOURCE=200809L)

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