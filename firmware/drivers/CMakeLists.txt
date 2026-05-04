if(DISPLAY_DRIVER STREQUAL "ILI9486")
  set(DISPLAY_SRC display/ili9486.c)
  set(DISPLAY_DEF DISPLAY_DRIVER_ILI9486)
elseif(DISPLAY_DRIVER STREQUAL "ST7789")
  set(DISPLAY_SRC display/st7789.c)
  set(DISPLAY_DEF DISPLAY_DRIVER_ST7789)
else()
  message(FATAL_ERROR "Unknown DISPLAY_DRIVER '${DISPLAY_DRIVER}'. Valid values: ST7789, ILI9486")
endif()

add_library(drivers STATIC
  ${DISPLAY_SRC}
  encoder/encoder.c
)

target_compile_definitions(drivers PUBLIC ${DISPLAY_DEF})

target_include_directories(drivers PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(drivers
  hal
  pico_stdlib
)
