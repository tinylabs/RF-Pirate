#include <stdio.h>
#include <string.h>

#include <avr/io.h>

#include "context.h"
#include "si4432.h"
#include "hw.h"

static context_t *sys;

// address of first reg
static uint8_t reg_addr;
#define REG_PER_LINE  6

static void dump_rf_regs (uint8_t start);

// Handle all events except back key to quit
uint8_t rf_debug_notify (uint8_t event, uint16_t data)
{
  uint8_t rv;

  switch (event) {
    case EVENT_APP_START:
      sys = (context_t *)data;
      // No menu
      sys->menu->Enable(0);
      reg_addr = 0;
      rf_probe();
      // Update display
      dump_rf_regs(reg_addr);
      rv = true;
      break;

    case EVENT_APP_STOP:
      // nothing to do
      sys->menu->Enable(1);
      rv = true;
      break;

    case EVENT_KEYPRESS:
      // Trap all keypress events except left
      switch (data) {
        case KEY_UP:
	  if (reg_addr >= 6) reg_addr -= REG_PER_LINE;
	  dump_rf_regs(reg_addr);
	  break;
        
      case KEY_DOWN: // Last line should be 0x7e
	  if (reg_addr <= 126 - (5 * 6)) reg_addr += REG_PER_LINE;
	  dump_rf_regs(reg_addr);
	  break;

        case KEY_LEFT:					
	  rv = false; // Pass on to menu system
	  break;
        
        default:
	  rv = true; // ignore all other keys
	  break;
      }
      break; /* end keypress */
    
    default:
      // Don't recognize this event return false
      rv = false;
      break;
  }
  return rv;  // Pass event on if unregonized
}

static void dump_rf_regs (uint8_t start)
{
  uint8_t line;
  uint8_t buf[6]; // reg buffer
  char str[22];
  
  // Clear lcd lines 1-5
  sys->disp->clearline(1);
  sys->disp->clearline(2);
  sys->disp->clearline(3);
  sys->disp->clearline(4);
  sys->disp->clearline(5);

  // Dump all registers to display
  for (line = 1; line < 6; line++) {
    // Clear buffer
    memset(buf, 0, sizeof(buf));
    // Read Si4432 registers
    rf_spi_readm(start, buf, start < 126 ? sizeof(buf) : 2);
    // Write to buffer
    snprintf (str, sizeof(str), "%02x: %02x %02x %02x %02x %02x %02x",
	      start, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);    
    // Write to display
    sys->disp->drawstring (line, 0, str, 0);
    start += REG_PER_LINE;
  }
  // Display
  sys->disp->display();
}

/* Dummy menu */
MenuEntry m_rf_debug[] = {
  NULL
};
