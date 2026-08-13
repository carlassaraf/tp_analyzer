# Copyright (c) 2026 Fabrizio Carlassara
# SPDX-License-Identifier: Apache-2.0

# Named explicitly (rather than the default zephyr_library() derived name)
# so app/CMakeLists.txt has a stable target to target_link_libraries()
# against. This is added via add_subdirectory() from app/CMakeLists.txt, i.e.
# in Zephyr CMake "application mode" — by this point Zephyr's own ZEPHYR_LIBS
# bookkeeping (used to build the final link line) is already finalized, so
# this library does NOT get auto-linked into the app the way an in-tree
# driver would. The explicit target_link_libraries(app PRIVATE adc_stream)
# in app/CMakeLists.txt is required, not optional.
zephyr_library_named(adc_stream)
zephyr_library_sources(adc_stream_rpi_pico.c)
zephyr_library_include_directories(.)

# This library is added from app/CMakeLists.txt, i.e. in Zephyr CMake
# "application mode" (after find_package(Zephyr) already ran the loop
# that wires zephyr_generated_headers into in-tree libraries). Without
# this, generated headers like <zephyr/heap_constants.h> may not exist
# yet when this file is compiled.
zephyr_library_add_dependencies(zephyr_generated_headers)
