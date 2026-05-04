# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(firmware C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

set(DISPLAY_DRIVER "ST7789" CACHE STRING "Display driver: ST7789 or ILI9486")
set_property(CACHE DISPLAY_DRIVER PROPERTY STRINGS ST7789 ILI9486)

# add_subdirectory(domain)
add_subdirectory(third_party/lvgl)
add_subdirectory(hal)
add_subdirectory(drivers)
add_subdirectory(services)
add_subdirectory(bsp)
add_subdirectory(rtos)
add_subdirectory(app)

target_link_libraries(lvgl PUBLIC freertos)
add_executable(firmware main.c)

pico_set_program_name(firmware "firmware")
pico_set_program_version(firmware "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(firmware 0)
pico_enable_stdio_usb(firmware 1)

# Add libraries
target_link_libraries(firmware
  app
  services
  drivers
  hal
  bsp
  pico_stdlib
)

pico_add_extra_outputs(firmware)