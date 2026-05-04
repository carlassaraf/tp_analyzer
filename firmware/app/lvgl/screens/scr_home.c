#include "scr_home.h"
#include "lvgl.h"
#include "lvgl/screens.h"

static lv_obj_t *scr_home     = NULL;
static lv_obj_t *scr_home_btn = NULL;

void scr_home_init(void) {
  scr_home = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_home, lv_color_black(), LV_PART_MAIN);

  scr_home_btn = lv_button_create(scr_home);
  lv_obj_set_size(scr_home_btn, 120, 50);
  lv_obj_align(scr_home_btn, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *label = lv_label_create(scr_home_btn);
  lv_label_set_text(label, "Home");
  lv_obj_center(label);

  // Make this button reachable by the rotary encoder.
  // Add every focusable widget on this screen the same way.
  SCR_ADD_TO_GROUP(scr_home_btn);

  lv_screen_load_anim(scr_home, LV_SCR_LOAD_ANIM_FADE_IN, 300, 0, true);
}

void scr_home_deinit(void) {
  SCR_CLEAR_GROUP();
  scr_home     = NULL;
  scr_home_btn = NULL;
}

void scr_home_step(void) {
}
