#include "hal_display.h"

static hal_display_flush_fn   _flush_fn;
static const hal_display_info_t *_info;

void hal_display_set_backend(hal_display_flush_fn flush_fn, const hal_display_info_t *info) {
    _flush_fn = flush_fn;
    _info     = info;
}

const hal_display_info_t *hal_display_get_info(void) {
    return _info;
}

void hal_display_flush(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                       const uint8_t *buf, size_t len, void (*done_cb)(void)) {
    _flush_fn(x1, y1, x2, y2, buf, len, done_cb);
}
