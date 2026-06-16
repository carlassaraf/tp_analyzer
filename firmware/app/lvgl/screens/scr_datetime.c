#include "ui.h"
#include "lvgl/screens.h"

void scr_datetime_prepare(void)
{

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