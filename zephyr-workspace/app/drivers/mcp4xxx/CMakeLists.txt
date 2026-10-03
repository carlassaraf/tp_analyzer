# Copyright (c) 2026 Fabrizio Carlassara
# SPDX-License-Identifier: Apache-2.0

add_library(mcp4xxx STATIC mcp4xxx.c)
target_include_directories(mcp4xxx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mcp4xxx PUBLIC zephyr_interface)
add_dependencies(mcp4xxx zephyr_generated_headers)
