/**
 * Driver for the SSD1306 (128 x 64) OLED display.
 * Based on ladyada's implementation (https://github.com/adafruit/SSD1306)
 * and possibly cstone@pobox.com.
 * Optimized for reduced memory footprint and applications utilizing a 
 * text menu. Still capable of 128 x 64 monochrome bitmaps and icons that
 * are multiples of 8x6 chars. Footprint should be < 200 bytes RAM
 *
 * Elliot Buller 2011
 **/

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "ssd1306.h"
#include "glcdfont.h"


//Array to hold icons references - Non member for c callbacks
static icon_cb_t icon[LCD_MAX_ICON];

/**
 * Draw bitmap to display. Will not cache it but instead write it directly
 * to display. Only supports native resolution of 128 x 64
 */
void ssd1306::drawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  if (w != LCD_WIDTH || h != LCD_HEIGHT)
    return;

  ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
  ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
  ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0

  for (uint16_t i=0; i<1024; i++) {
    ssd1306_data( pgm_read_byte(&bitmap[i]) );
  }
}

/**
 * Register icon with display driver. Icon must be 8 bits high and a multiple
 * if 6 bits wide. Icons can later be displayed using the drawicon fn.
 */
void ssd1306::registericon(uint8_t index, icon_cb_t cb) {
  if (index >= LCD_MAX_ICON)
    return;
  // Save cb
  icon[index] = cb;
}


/**
 * Place an icon into the character buffer. Will get displayed on next 
 * screen update
 */  
void ssd1306::drawicon(uint8_t idx, uint8_t line, uint8_t x, uint8_t width) {
  uint8_t i;

  // Check bounds on x
  if (x >= LCD_CHAR_PER_LINE)
    return;

  // Validate icon index
  if (idx >= LCD_MAX_ICON)
    return;

  // Make sure we have enough room to place the icon
  if ((width > 8) || (x + width - 1 >= LCD_CHAR_PER_LINE))
    return;

  // Set placeholders in char array
  for (i = 0; i < width; i++) {
    buf[line][x + i] = LCD_ICON_FLAG | (idx << 3) | i;

    // Clear invert flag
    invert_mask[x + i] &= ~(1 << line);
  }
}

void ssd1306::clearline(uint8_t line) {
  uint8_t i;
  memset(&buf[line], 0, LCD_CHAR_PER_LINE);

  /* Clear all invert bits for line */
  for (i = 0; i < LCD_CHAR_PER_LINE; i++) {
    invert_mask[i] &= ~(1 << line);
  }
}

/**
 * Draw a string into the character buffer
 */
void ssd1306::drawstring(uint8_t line, uint8_t x, const char *c, uint8_t invert) {
  if (x >= LCD_CHAR_PER_LINE)
    return;

  while (*c != 0) {
    buf[line][x] = *c;
    if (invert)
      invert_mask[x] |= (1 << line);
    c++; x++;

    // Truncate overflowing lines
    if (x >= LCD_CHAR_PER_LINE) {
      //x = 0;    // ran out of this line
      //line++;
      return;
    }
    if (line >= LCD_CHAR_LINES)
      return;        // ran out of space :(
  }
}



inline void ssd1306::spiwrite(uint8_t c) {
  // Add spi hw access here
  // Write byte out to SPI hardware
  SPDR = c;
  // Wait for transfer to complete
  while(!(SPSR & (1<<SPIF)))
    ;
}

void ssd1306::ssd1306_command(uint8_t c) { 
  LCD_CS_PORT |= _BV(LCD_CS_PIN);
  LCD_DC_PORT &= ~_BV(LCD_DC_PIN);
  LCD_CS_PORT &= ~_BV(LCD_CS_PIN);
  spiwrite(c);
  LCD_CS_PORT |= _BV(LCD_CS_PIN);
}

void ssd1306::ssd1306_data(uint8_t c) {
  LCD_CS_PORT |= _BV(LCD_CS_PIN);
  LCD_DC_PORT |= _BV(LCD_DC_PIN);
  LCD_CS_PORT &= ~_BV(LCD_CS_PIN);
  spiwrite(c);
  LCD_CS_PORT |= _BV(LCD_CS_PIN);
}

/**
 * Update the display with data in the character buffer
 */
void ssd1306::display(void) {
  uint8_t line, idx;

  // Set display to raster from top left
  ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
  ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
  ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0

  // Loop through the character buffer
  for (line = 0; line < LCD_CHAR_LINES; line++) {
    for (idx = 0; idx < LCD_CHAR_PER_LINE; idx++) {
      uint8_t c, i, icon_flag = 0;
      uint8_t *bm;

      // grab character
      c  = buf[line][idx];

      // if icon do character replacement
      if (c & LCD_ICON_FLAG) {
	uint8_t idx;

	// Check bounds on index;
	idx = (c >> 3) & 0xf;
	if ((idx >= LCD_MAX_ICON) || (icon[idx] == NULL))
	  bm = &font['$' * 5];
	else {
	  // Point to icon character
	  // Call icon callback to get icon
	  bm = (icon[idx] != NULL) ? icon[idx](idx) : &font['$' * 5];
	  bm = &bm[(c & 0x7) * 6];
	  icon_flag = 1;
	}
      }
      else {
	// Point to font character
	bm = &font[c * 5];
	icon_flag = 0;
      }

      // Draw character
      for (i = 0; i < 6; i++) {
	// get each character slice
	// ascii chars are only 5 bits wide
	c = ((i == 5) && !icon_flag) ? 0 : pgm_read_byte(&bm[i]);
	
	// Invert if necessary
	if (invert_mask[idx] & (1 << line))
	  c = ~c;

	// Write data out to display
	ssd1306_data(c);
      }
    }
    // 21 chars per line * 6bytes per character = 126
    // Need 2 null bytes to round out each line
    ssd1306_data(0);
    ssd1306_data(0);
  }
}

// clear everything
void ssd1306::clear(void) {
  uint8_t line;
  // Clear character buffer
  for (line = 0; line < LCD_CHAR_LINES; line++) {
    memset(buf[line], 0, LCD_CHAR_PER_LINE);
  }
  // clear invert mask
  memset(invert_mask, 0, sizeof(invert_mask));
}

void ssd1306::poweroff(void)
{
  // Turn off display
  ssd1306_command(SSD1306_DISPLAYOFF);
  // Turn off charge pump
  ssd1306_command(SSD1306_CHARGEPUMP);
  ssd1306_command(0x10);
}

void ssd1306::init(uint8_t vccstate) {
  // set pin directions
  LCD_CS_DDR |= _BV(LCD_CS_PIN);
  LCD_DC_DDR |= _BV(LCD_DC_PIN);
  LCD_RST_DDR |= _BV(LCD_RST_PIN);

  // Set SPI Directions
  DDRB |= (_BV(MOSI_PIN) | _BV(SCK_PIN));
  // Clear SPI pwr saving bit
  PRR0 &= ~(4);
  // Enable SPI
  //SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
  SPCR = (1<<SPE) | (1<<MSTR);  // 2Mhz clock

  // Reset LCD
  LCD_RST_PORT |= _BV(LCD_RST_PIN);
  // VDD (3.3V) goes high at start, lets just chill for a ms
  _delay_ms(1);
  LCD_RST_PORT &= ~_BV(LCD_RST_PIN);
  // wait 10ms
  _delay_ms(10);
  // bring out of reset
  LCD_RST_PORT |= _BV(LCD_RST_PIN);

  ssd1306_command(SSD1306_DISPLAYOFF);  // 0xAE
  ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
  ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0  
  ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0
  ssd1306_command(SSD1306_SETCONTRAST);  // 0x81

  if (vccstate == SSD1306_EXTERNALVCC) {
    ssd1306_command(0x9F);  // external 9V
  } else {
    ssd1306_command(0xCF);  // chargepump
  }
    
  ssd1306_command(0xa1);  // setment remap 95 to 0 (?)
  ssd1306_command(SSD1306_NORMALDISPLAY); // 0xA6
  ssd1306_command(SSD1306_DISPLAYALLON_RESUME); // 0xA4
  ssd1306_command(SSD1306_SETMULTIPLEX); // 0xA8
  ssd1306_command(0x3F);  // 0x3F 1/64 duty
  ssd1306_command(SSD1306_SETDISPLAYOFFSET); // 0xD3
  ssd1306_command(0x0); // no offset
  ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);  // 0xD5
  ssd1306_command(0x80);  // the suggested ratio 0x80
  ssd1306_command(SSD1306_SETPRECHARGE); // 0xd9

  if (vccstate == SSD1306_EXTERNALVCC) {
    ssd1306_command(0x22); // external 9V
  } else {
    ssd1306_command(0xF1); // DC/DC
  }
  
  ssd1306_command(SSD1306_SETCOMPINS); // 0xDA
  ssd1306_command(0x12); // disable COM left/right remap  
  ssd1306_command(SSD1306_SETVCOMDETECT); // 0xDB
  ssd1306_command(0x40); // 0x20 is default?
  ssd1306_command(SSD1306_MEMORYMODE); // 0x20
  ssd1306_command(0x00); // 0x0 act like ks0108
  
  // flip display, upside down on board
  ssd1306_command(SSD1306_SEGREMAP);
  ssd1306_command(SSD1306_COMSCANINC);
  ssd1306_command(SSD1306_CHARGEPUMP); //0x8D
  if (vccstate == SSD1306_EXTERNALVCC) {
    ssd1306_command(0x10);  // disable
  } else {
    ssd1306_command(0x14);  // disable    
  }
  ssd1306_command(SSD1306_DISPLAYON);//--turn on oled panel
}

/* Constructor */
ssd1306::ssd1306 (void)
{
  uint8_t i;

  // Clear all icons
  for (i = 0; i < LCD_MAX_ICON; i++)
    icon[i] = NULL;
}
