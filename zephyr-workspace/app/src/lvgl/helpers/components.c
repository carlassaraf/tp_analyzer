#include "components.h"

#define COMP_TOPBAR_TIME_IDX  3
#define COMP_TOPBAR_DATE_IDX  4

void ui_topbar_update_datetime(lv_obj_t *parent,  struct rtc_time *dt)
{
  static const char month_str[12][4] = {
    "ENE", "FEB", "MAR", "ABR", "MAY", "JUN",
    "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"
  };
  lv_obj_t *time_lbl = lv_obj_get_child(parent, COMP_TOPBAR_TIME_IDX);
  lv_obj_t *date_lbl = lv_obj_get_child(parent, COMP_TOPBAR_DATE_IDX);
  lv_label_set_text_fmt(time_lbl, "%02d:%02d", dt->tm_hour, dt->tm_min);
  lv_label_set_text_fmt(date_lbl, "%02d %s", dt->tm_mday, month_str[dt->tm_mon]);
}