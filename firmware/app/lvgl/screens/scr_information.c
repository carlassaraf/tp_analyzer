#include "ui.h"
#include "lvgl/screens.h"

void scr_information_prepare(void)
{

}

void scr_information_init(void)
{
  SCR_ADD_TO_GROUP(ui_scrInformation_btnBack);
}

void scr_information_deinit(void)
{
  SCR_CLEAR_GROUP();
}

void scr_information_step(void)
{
  uint32_t uptime_seconds = lv_tick_get() / 1000;
  uint8_t hours = uptime_seconds / 3600;
  uint8_t minutes = (uptime_seconds % 3600) / 60;
  uint8_t seconds = (uptime_seconds % 3600) % 60; 
  lv_label_set_text_fmt(ui_scrInformation_lblActiveTimeV, "%02d:%02d:%02d", hours, minutes, seconds);
}