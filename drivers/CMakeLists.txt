add_library(drivers STATIC
  display/st7789.c
)

target_include_directories(drivers PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(drivers
  hal
  pico_stdlib
)
