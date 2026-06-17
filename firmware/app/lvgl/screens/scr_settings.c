#include "ui.h"
#include "lvgl/screens.h"

void scr_settings_prepare(void)
{
  lv_obj_add_flag(ui_scrSettings_cntDatetime, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrSettings_cntScreen, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrSettings_cntMeasure, LV_OBJ_FLAG_EVENT_TRICKLE);
  lv_obj_add_flag(ui_scrSettings_cntInfo, LV_OBJ_FLAG_EVENT_TRICKLE);
}

void scr_settings_init(void)
{
  SCR_ADD_TO_GROUP(ui_scrSettings_btnBack);
  SCR_ADD_TO_GROUP(ui_scrSettings_cntDatetime);
  SCR_ADD_TO_GROUP(ui_scrSettings_cntScreen);
  SCR_ADD_TO_GROUP(ui_scrSettings_cntMeasure);
  SCR_ADD_TO_GROUP(ui_scrSettings_cntInfo);
  // Update clock label in menu
  hal_rtc_datetime_t dt;
  hal_rtc_get(&dt);
  lv_label_set_text_fmt(ui_scrSettings_lblDatetimeBrief, "%02d/%02d/%02d - %02d:%02d", dt.day, dt.month, dt.year, dt.hour, dt.min);
}

void scr_settings_deinit(void)
{
  SCR_CLEAR_GROUP();
}

void scr_settings_step(void)
{

}