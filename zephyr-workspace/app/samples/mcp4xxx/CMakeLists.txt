# Copyright (c) 2026 Fabrizio Carlassara
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# This sample is its own application root (built via `-s
# app/samples/mcp4xxx`), so DTS_ROOT defaults to *this* directory, not
# app/ -- without this, the devicetree bindings at app/dts/bindings/
# (microchip,mcp41xx.yaml / microchip,mcp42xx.yaml) are never found, the
# mcp42xx0 node below never resolves against a binding, and
# DT_HAS_MICROCHIP_MCP42XX_ENABLED silently never becomes true, which
# silently keeps CONFIG_MCP4XXX at n regardless of anything in prj.conf.
# Must be set before find_package(Zephyr) picks DTS_ROOT up.
list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

find_package(Zephyr REQUIRED)
project(mcp4xxx_sample)

target_sources(app PRIVATE src/main.c)

# Reach into the app-local driver this sample exists to exercise -- same
# relative layout app/CMakeLists.txt uses for the real firmware, just two
# levels further down since this sample lives at app/samples/mcp4xxx/.
set(MCP4XXX_DRIVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../drivers/mcp4xxx)
target_include_directories(app PRIVATE ${MCP4XXX_DRIVER_DIR})
add_subdirectory_ifdef(CONFIG_MCP4XXX ${MCP4XXX_DRIVER_DIR} ${CMAKE_CURRENT_BINARY_DIR}/mcp4xxx)
target_link_libraries_ifdef(CONFIG_MCP4XXX app PRIVATE mcp4xxx)
