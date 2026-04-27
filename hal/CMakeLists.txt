add_library(hal STATIC
  hal_spi.c
  hal_gpio.c
  hal_display.c
  hal_adc.c
)

target_include_directories(hal PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}
)

target_link_libraries(hal pico_stdlib hardware_spi hardware_gpio hardware_adc hardware_dma)
