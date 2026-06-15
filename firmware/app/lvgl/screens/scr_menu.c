#include "ui.h"
#include "scr_menu.h"
#include "lvgl.h"
#include "lvgl/screens.h"

void scr_menu_prepare(void) {
  lv_obj_add_flag(ui_scrMenu_cardOscilloscope, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrMenu_cardContRealTime, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrMenu_cardFFT, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrMenu_cardContFft, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrMenu_cardMediciones, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrMenu_cardContMediciones, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrMenu_cardAjustes, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrMenu_cardContAjustes, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrMenu_cardRec, LV_OBJ_FLAG_EVENT_TRICKLE);
}

void scr_menu_init(void) {
  // Make this button reachable by the rotary encoder.
  // Add every focusable widget on this screen the same way.
  SCR_ADD_TO_GROUP(ui_scrMenu_cardOscilloscope);
  SCR_ADD_TO_GROUP(ui_scrMenu_cardFFT);
  SCR_ADD_TO_GROUP(ui_scrMenu_cardMediciones);
  SCR_ADD_TO_GROUP(ui_scrMenu_cardAjustes);
}

void scr_menu_deinit(void) {
  SCR_CLEAR_GROUP();
}

void scr_menu_step(void) {
}
