add_library(services STATIC
  lvgl/lvgl_port.c
)

target_include_directories(services PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  lvgl
)

target_link_libraries(services
  bsp
  hal
  drivers
  lvgl
)
