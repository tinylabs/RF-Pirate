#ifndef _SSD1306_H_
#define _SSD1306_H_
/**
 * Driver for the SSD1306 (128 x 64) OLED display.
 * Based heavily on ladyada's implementation (https://github.com/adafruit/SSD1306)
 * Optimized for reduced memory footprint and applications utilizing a text menu.
 * Still capable of 128 x 64 monochrome bitmaps and icons that are multiples of 8x6 chars.
 * Footprint should be < 200 bytes RAM
 *
 * Elliot Buller 2011
 **/
#include <stdint.h>

// SSD1306 regs
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Geometry
#define LCD_CHAR_LINES    8
#define LCD_CHAR_PER_LINE 21
#define LCD_WIDTH         128
#define LCD_HEIGHT        64
#define LCD_MAX_ICON      5
#define LCD_ICON_FLAG     0x80

/* Define ports */
#define LCD_CS_PORT       PORTD
#define LCD_CS_PIN        7
#define LCD_CS_DDR        DDRD

#define LCD_DC_PORT       PORTB
#define LCD_DC_PIN        0
#define LCD_DC_DDR        DDRB

#define LCD_RST_PORT      PORTB
#define LCD_RST_PIN       7
#define LCD_RST_DDR       DDRB

#define MOSI_PIN          2
#define SCK_PIN           1

typedef uint8_t *(*icon_cb_t) (uint8_t idx);

class ssd1306 {

 public:
  ssd1306 (void);
  void init(uint8_t switchvcc);
  void clear();
  void display();
  void poweroff();

  // Draw functions
  void drawstring(uint8_t line, uint8_t x, const char *c, uint8_t invert);
  void drawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h);
  void drawicon(uint8_t index, uint8_t line, uint8_t x, uint8_t width);
  void registericon(uint8_t index, icon_cb_t cb);
  void clearline(uint8_t line);
  virtual void ssd1306_command(uint8_t c);
  virtual void ssd1306_data(uint8_t c);

 private:
  void spiwrite(uint8_t c);
  // character buffer
  uint8_t buf[LCD_CHAR_LINES][LCD_CHAR_PER_LINE];
  // mask of inverted characters
  uint8_t invert_mask[LCD_CHAR_PER_LINE];
};

#endif /* _SSD1306_H_ */
