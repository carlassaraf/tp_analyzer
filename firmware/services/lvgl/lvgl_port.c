#include "lvgl.h"
#include "drivers/encoder/encoder.h"
#include "board_config.h"
#include "FreeRTOS.h"
#include "task.h"

#if defined(DISPLAY_DRIVER_ILI9486)
#include "drivers/display/ili9486.h"
#define DISP_WIDTH    ILI9486_WIDTH
#define DISP_HEIGHT   ILI9486_HEIGHT
#define DISP_X_OFF    0
#define DISP_Y_OFF    0
static ili9486_t *display_ctx;
#elif defined(DISPLAY_DRIVER_ST7789)
#include "drivers/display/st7789.h"
#define DISP_WIDTH    TFT_WIDTH
#define DISP_HEIGHT   TFT_HEIGHT
#define DISP_X_OFF    TFT_COL_OFFSET
#define DISP_Y_OFF    TFT_ROW_OFFSET
static st7789_t *display_ctx;
#else
#error "No display driver selected. Set DISPLAY_DRIVER to ST7789 or ILI9486."
#endif

#define DISPLAY_BUFFER_LINES 20

static lv_display_t *disp;

static void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *color_p) {
  uint16_t x1 = (uint16_t)area->x1 + DISP_X_OFF;
  uint16_t x2 = (uint16_t)area->x2 + DISP_X_OFF;
  uint16_t y1 = (uint16_t)area->y1 + DISP_Y_OFF;
  uint16_t y2 = (uint16_t)area->y2 + DISP_Y_OFF;

  uint32_t pixels = (uint32_t)(area->x2 - area->x1 + 1) *
                    (uint32_t)(area->y2 - area->y1 + 1);

#if defined(DISPLAY_DRIVER_ILI9486)
  ili9486_set_window(display_ctx, x1, y1, x2, y2);
  ili9486_send_pixels(display_ctx, color_p, pixels * 2);
#else
  st7789_set_window(display_ctx, x1, y1, x2, y2);
  st7789_send_pixels(display_ctx, color_p, pixels * 2);
#endif

  lv_display_flush_ready(display);
}

static void encoder_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
  (void)indev;
  data->enc_diff = (int16_t)encoder_pop_delta();
  data->state    = encoder_btn_down() ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

int32_t lvgl_port_init(void) {
  lv_init();

#if defined(DISPLAY_DRIVER_ILI9486)
  static const ili9486_config_t disp_config = {
    .data_pin_base = ILI9486_DATA_BASE,
    .pin_wr  = ILI9486_PIN_WR,
    .pin_rd  = ILI9486_PIN_RD,
    .pin_rs  = ILI9486_PIN_RS,
    .pin_cs  = ILI9486_PIN_CS,
    .pin_rst = ILI9486_PIN_RST,
    .pin_bl  = ILI9486_PIN_BL,
    .width   = ILI9486_WIDTH,
    .height  = ILI9486_HEIGHT,
    .rotation = 1,
  };
  display_ctx = ili9486_init(&disp_config);
#else
  static const st7789_config_t disp_config = {
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
  display_ctx = st7789_init(&disp_config);
#endif

  if (!display_ctx) {
    return -1;
  }

  static uint8_t buf[DISP_WIDTH * DISPLAY_BUFFER_LINES * 2] __attribute__((aligned(4)));

  disp = lv_display_create(DISP_WIDTH, DISP_HEIGHT);
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565_SWAPPED);
  lv_display_set_flush_cb(disp, flush_cb);
  lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_tick_set_cb((lv_tick_get_cb_t)xTaskGetTickCount);

  // static const encoder_config_t enc_config = {
  //   .pin_a   = PIN_ENC_A,
  //   .pin_b   = PIN_ENC_B,
  //   .pin_btn = PIN_ENC_BTN,
  // };
  // encoder_init(&enc_config);

  // lv_indev_t *enc_indev = lv_indev_create();
  // lv_indev_set_type(enc_indev, LV_INDEV_TYPE_ENCODER);
  // lv_indev_set_read_cb(enc_indev, encoder_read_cb);

  // lv_group_t *group = lv_group_create();
  // lv_group_set_default(group);
  // lv_indev_set_group(enc_indev, group);

  return 0;
}
