#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// TFT display SPI bus
#define PIN_TFT_SCK   18
#define PIN_TFT_MOSI  19
#define PIN_TFT_CS    17
#define PIN_TFT_DC    16
#define PIN_TFT_RST   20
#define PIN_TFT_BL    21

#define TFT_SPI_NUM   0
#define TFT_SPI_BAUD  62500000

// Display dimensions (landscape: 320 wide x 172 tall)
#define TFT_WIDTH    320
#define TFT_HEIGHT   172

// ST7789 framebuffer is 240x320; the 1.47" panel is 172x320.
// In landscape mode (MADCTL=0x60) the 172-row panel sits at row offset 34
// inside the controller's 240-row space.
#define TFT_COL_OFFSET  0
#define TFT_ROW_OFFSET  34

// Rotary encoder
#define PIN_ENC_A    6
#define PIN_ENC_B    7
#define PIN_ENC_BTN  8

#endif
