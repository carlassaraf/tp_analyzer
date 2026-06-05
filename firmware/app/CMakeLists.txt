add_library(app STATIC
  app.c
  tasks/ui_task.c
  tasks/ad_task.c
  lvgl/screen_manager.c
  lvgl/screen_update.c
  lvgl/screens/scr_boot.c
  lvgl/screens/scr_home.c
  lvgl/screens/scr_plotter.c
  lvgl/helpers/chart.c
)

target_include_directories(app PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(app
  services
  freertos
)

add_subdirectory(lvgl/ui)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/ui)
target_link_libraries(ui lvgl)
target_link_libraries(app ui)
