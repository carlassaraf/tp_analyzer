add_library(services STATIC
  lvgl/lvgl_port.c
  dsp/dsp.c
)

target_include_directories(services PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  lvgl
  dsp
)

target_link_libraries(services
  bsp
  hal
  drivers
  lvgl
  CMSISDSP 
)