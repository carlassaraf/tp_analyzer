#include "ui.h"
#include "lvgl/screens.h"

void scr_datetime_prepare(void)
{
  lv_obj_add_flag(ui_scrDatetime_cntDay, LV_OBJ_FLAG_STATE_TRICKLE);
lv_obj_add_flag(ui_scrDatetime_cntMonth, LV_OBJ_FLAG_STATE_TRICKLE);
lv_obj_add_flag(ui_scrDatetime_cntYear, LV_OBJ_FLAG_STATE_TRICKLE);
lv_obj_add_flag(ui_scrDatetime_cntHour, LV_OBJ_FLAG_STATE_TRICKLE);
  lv_obj_add_flag(ui_scrDatetime_cntMin, LV_OBJ_FLAG_STATE_TRICKLE);
}

void scr_datetime_init(void)
{
  SCR_ADD_TO_GROUP(ui_scrDatetime_btnBack);
  SCR_ADD_TO_GROUP(ui_scrDatetime_cntDay);
}

void scr_datetime_deinit(void)
{
  SCR_CLEAR_GROUP();
}

void scr_datetime_step(void)
{

}