add_library(hal STATIC
  hal_spi.c
  hal_gpio.c
  hal_display.c
  hal_adc.c
  hal_pio.c
  hal_dma.c
  hal_rtc.c
)

pico_generate_pio_header(hal ${CMAKE_CURRENT_SOURCE_DIR}/hal_8080_write.pio)

target_include_directories(hal PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}
)

target_link_libraries(hal pico_stdlib hardware_spi hardware_gpio hardware_adc hardware_dma hardware_pio pico_aon_timer)
