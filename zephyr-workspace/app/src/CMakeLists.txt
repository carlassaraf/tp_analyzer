add_library(app_lib STATIC
  app.c
  dev_state/dev_state.c
  threads/ui_thread.c
  threads/ad_thread.c
  lvgl/screen_manager.c
  lvgl/screen_update.c
  lvgl/screens/scr_boot.c
  lvgl/screens/scr_datetime.c
  lvgl/screens/scr_fft.c
  lvgl/screens/scr_information.c
  lvgl/screens/scr_menu.c
  lvgl/screens/scr_oscilloscope.c
  lvgl/screens/scr_settings.c
  lvgl/screens/scr_timeout.c
  lvgl/helpers/animations.c
  lvgl/helpers/chart.c
  lvgl/helpers/components.c
)

target_include_directories(app_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(app_lib PUBLIC
  zephyr_interface
  ui
)

add_subdirectory(lvgl/ui)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/ui)
target_link_libraries(ui PUBLIC zephyr_interface)
