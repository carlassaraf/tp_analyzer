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

// ADC input
#define ADC_GPIO         26    // GPIO26 = ADC channel 0
#define ADC_CHANNEL      0
#define ADC_SAMPLE_RATE  10000 // Hz

// Rotary encoder
#define PIN_ENC_A    13
#define PIN_ENC_B    14
#define PIN_ENC_BTN  15

// ---- ILI9486 3.5" 480x320 8-bit parallel ----
// Data bus D0-D7 uses GPIO 0-7 (consecutive, required for gpio_put_masked).
#define ILI9486_DATA_BASE  0   // D0=GPIO0, D1=GPIO1, ..., D7=GPIO7
#define ILI9486_PIN_WR     17  // Write strobe (active low)
#define ILI9486_PIN_RD     16  // Read strobe  (active low, can be tied high)
#define ILI9486_PIN_RS     18  // Register select: 0=cmd, 1=data
#define ILI9486_PIN_CS     19  // Chip select (active low)
#define ILI9486_PIN_RST    20  // Reset (active low)
#define ILI9486_PIN_BL     -1  // No dedicated backlight pin on this display
#define ILI9486_WIDTH      480
#define ILI9486_HEIGHT     320

#endif
