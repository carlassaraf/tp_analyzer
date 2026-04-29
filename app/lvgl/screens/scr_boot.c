#include "lvgl.h"
#include "ui/screen_manager.h"

// Time to wait in the boot screen
#define SCR_BOOT_TIME_WAIT_MS 5000

static lv_obj_t *scr_boot                    = NULL;
static lv_obj_t *scr_boot_label_welcome      = NULL;
static lv_obj_t *scr_boot_spinner_welcome    = NULL;

static uint32_t timestamp_start_boot = 0;

void scr_boot_init(void) {
  timestamp_start_boot = lv_tick_get();

  scr_boot = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_boot, lv_color_black(), LV_PART_MAIN);

  scr_boot_label_welcome = lv_label_create(scr_boot);
  lv_obj_set_pos(scr_boot_label_welcome, 0, -20);
  lv_obj_set_size(scr_boot_label_welcome, 280, 30);
  lv_obj_set_align(scr_boot_label_welcome, LV_ALIGN_CENTER);
  lv_obj_set_style_text_align(scr_boot_label_welcome, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_font(scr_boot_label_welcome, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_text_color(scr_boot_label_welcome, lv_color_white(), LV_PART_MAIN);
  lv_label_set_text(scr_boot_label_welcome, "Analizador Trifasico");

  scr_boot_spinner_welcome = lv_spinner_create(scr_boot);
  lv_obj_set_pos(scr_boot_spinner_welcome, 0, 40);
  lv_obj_set_size(scr_boot_spinner_welcome, 50, 50);
  lv_obj_set_align(scr_boot_spinner_welcome, LV_ALIGN_CENTER);
  lv_obj_set_style_arc_width(scr_boot_spinner_welcome, 6, LV_PART_MAIN);
  lv_obj_set_style_arc_width(scr_boot_spinner_welcome, 6, LV_PART_INDICATOR);
  lv_obj_set_style_arc_opa(scr_boot_spinner_welcome, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_spinner_set_anim_params(scr_boot_spinner_welcome, 10000, 200);

  lv_screen_load_anim(scr_boot, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

void scr_boot_deinit(void) {
  // lv_screen_load_anim was called with auto_del=true, so LVGL deletes
  // this screen object after the next screen's transition animation completes.
  // Just clear the pointers here.
  scr_boot                  = NULL;
  scr_boot_label_welcome    = NULL;
  scr_boot_spinner_welcome  = NULL;
}

void scr_boot_step(void) {
  if (lv_tick_elaps(timestamp_start_boot) >= SCR_BOOT_TIME_WAIT_MS) {
    screen_manager_go_to(SCREEN_HOME);
  }
}
