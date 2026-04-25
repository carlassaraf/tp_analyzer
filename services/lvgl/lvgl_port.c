#include "lvgl.h"
#include "drivers/display/st7789.h"
#include "board_config.h"

#define DISPLAY_BUFFER_LINES 20

static lv_display_t *disp;
static st7789_t *st7789_ctx;

static void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *color_p) {
    // Apply controller offset: the 172-row panel sits at TFT_ROW_OFFSET inside
    // the ST7789's 240-row framebuffer when using landscape MADCTL=0x60.
    uint16_t x1 = (uint16_t)area->x1 + TFT_COL_OFFSET;
    uint16_t x2 = (uint16_t)area->x2 + TFT_COL_OFFSET;
    uint16_t y1 = (uint16_t)area->y1 + TFT_ROW_OFFSET;
    uint16_t y2 = (uint16_t)area->y2 + TFT_ROW_OFFSET;

    uint32_t pixels = (uint32_t)(area->x2 - area->x1 + 1) *
                      (uint32_t)(area->y2 - area->y1 + 1);

    st7789_set_window(st7789_ctx, x1, y1, x2, y2);
    st7789_send_pixels(st7789_ctx, color_p, pixels * 2);

    lv_display_flush_ready(display);
}

int32_t lvgl_port_init(void) {
    lv_init();

    static const st7789_config_t st7789_config = {
        .spi_config = {
            .spi_num  = TFT_SPI_NUM,
            .baudrate = TFT_SPI_BAUD,
            .sck_pin  = PIN_TFT_SCK,
            .mosi_pin = PIN_TFT_MOSI,
            .miso_pin = -1,
            .cs_pin   = PIN_TFT_CS,
        },
        .pin_dc  = PIN_TFT_DC,
        .pin_rst = PIN_TFT_RST,
        .pin_bl  = PIN_TFT_BL,
        .width   = TFT_WIDTH,
        .height  = TFT_HEIGHT,
        .rotation = 0,
    };

    st7789_ctx = st7789_init(&st7789_config);
    if (!st7789_ctx) {
        return -1;
    }

    // 4-byte aligned draw buffer: TFT_WIDTH x DISPLAY_BUFFER_LINES x 2 bytes/pixel
    static uint8_t buf[TFT_WIDTH * DISPLAY_BUFFER_LINES * 2] __attribute__((aligned(4)));

    disp = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    return 0;
}
