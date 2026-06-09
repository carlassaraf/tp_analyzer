#include "ui.h"
#include "scr_menu.h"
#include "lvgl.h"
#include "lvgl/screens.h"

void scr_menu_prepare(void) {

}

void scr_menu_init(void) {
  // Make this button reachable by the rotary encoder.
  // Add every focusable widget on this screen the same way.
  SCR_ADD_TO_GROUP(ui_scrMenu_btnMenuPlotter);
  SCR_ADD_TO_GROUP(ui_scrMenu_btnMenuFFT);
}

void scr_menu_deinit(void) {
  SCR_CLEAR_GROUP();
}

void scr_menu_step(void) {
}
