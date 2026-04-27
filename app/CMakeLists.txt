add_library(app STATIC
  app.c
  tasks/ui_task.c
  tasks/ad_task.c
  ui/screen_manager.c
  ui/screen_update.c
  ui/screens/scr_boot.c
  ui/screens/scr_home.c
  ui/screens/scr_plot.c
)

target_include_directories(app PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(app
  services
  freertos
)
