if(DISPLAY_DRIVER STREQUAL "ILI9486")
  set(DISPLAY_SRC display/ili9486.c)
  set(DISPLAY_DEF DISPLAY_DRIVER_ILI9486)
  set(DISPLAY_LIBS hardware_pio hardware_dma)
elseif(DISPLAY_DRIVER STREQUAL "ST7789")
  set(DISPLAY_SRC display/st7789.c)
  set(DISPLAY_DEF DISPLAY_DRIVER_ST7789)
  set(DISPLAY_LIBS "")
else()
  message(FATAL_ERROR "Unknown DISPLAY_DRIVER '${DISPLAY_DRIVER}'. Valid values: ST7789, ILI9486")
endif()

add_library(drivers STATIC
  ${DISPLAY_SRC}
  encoder/encoder.c
)

if(DISPLAY_DRIVER STREQUAL "ILI9486")
  pico_generate_pio_header(drivers ${CMAKE_CURRENT_SOURCE_DIR}/display/ili9486_write.pio)
endif()

target_compile_definitions(drivers PUBLIC ${DISPLAY_DEF})

target_include_directories(drivers PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(drivers
  hal
  pico_stdlib
  ${DISPLAY_LIBS}
)
