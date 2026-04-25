add_library(bsp STATIC
  rp2350/bsp.c
)

target_include_directories(bsp PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/rp2350
)

target_link_libraries(bsp
  pico_stdlib
  hardware_spi
  drivers
  hal
)