#include "animations.h"

void animation_move_to_side(lv_obj_t * obj, int32_t start_x, int32_t end_x, uint32_t duration) {
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, obj);
  lv_anim_set_values(&a, start_x, end_x);
  lv_anim_set_duration(&a, duration);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out); // Smooth easing
  lv_anim_start(&a);
}