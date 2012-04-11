/**
 * UI events handlers
 * Elliot Buller 2012
 **/
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ssd1306.h"
#include "Menu.h"

#include "cmd_parser.h"
#include "timetick.h"
#include "keypad.h"
#include "hw.h"
#include "usb_serial.h"

// Apps
#include "rf_test.h"
#include "rf_debug.h"

// logo + icons
#include "logo.h"

// Include battery icons
#include "batt_0p0.h"
#include "batt_0p25.h"
#include "batt_0p5.h"
#include "batt_0p75.h"
#include "batt_1p0.h"

// pointer to battery icon
uint8_t *batt_icon = batt_1p0;
extern unsigned char font[];

/* Core ui objects */
ssd1306 oled;
EXPORT_MENU(m_root);

/* line  (Graphic LCD usage)
 * 0 - Header
 * 1 - Menu 1
 * 2 - ..
 * 3 - ..
 * 4 - ..
 * 5 - Menu 5
 * 6 - Text system status (yellow)
 * 7 - icons              (yellow)
 */
Menu menu(m_root, &oled, 1, 5);

#define VER_MAJOR 00
#define VER_MINOR 01

// Define bootloader in memory
void (*bootloader) (void) = (void (*)())0x3800;
//void (*bootloader) (void) = (void (*)())0x7000;

void shutdown ();
void fast_charge_on(void) { HIGH(usb_i_sel); oled.poweroff(); }
void jmp_bootloader(void) { 
  cli(); 
  // Make it look like an external reset
  MCUSR |= _BV(EXTRF);
  bootloader(); 
}

// Todo move to menu file
MenuEntry m_root[] = {
  MenuEntry ("RF", m_rf_root, &rf_event_notify),
  MenuEntry ("RF Debug", m_rf_debug, &rf_debug_notify),
  MenuEntry ("Bootloader", &jmp_bootloader),
  MenuEntry ("Shutdown", &shutdown),
  NULL
};

void shutdown ()
{
  // Turn off LCD
  oled.poweroff();
  // Release pshold
  LOW(pshold);
}


void led_keepalive (uint16_t ticks) {
  TOGGLE(led);
}

void UpdateStatus (const char *str) {
  oled.clearline(6);
  oled.drawstring(6, 0, str, 0);
  oled.display();
}


/* Choose icon based on vbatt level */
#define VBATT_1P0   160
#define VBATT_0P75  158
#define VBATT_0P5   150
#define VBATT_0P25  140

static void UpdateVbatt (uint8_t level)
{
  if (level >= VBATT_1P0) {
    batt_icon = batt_1p0;
  }
  else if (level >= VBATT_0P75) {
    batt_icon = batt_0p75;
  }
  else if (level >= VBATT_0P5) {
    batt_icon = batt_0p5;
  }
  else if (level >= VBATT_0P25) {
    batt_icon = batt_0p25;
  }
  else {
    batt_icon = batt_0p0;
  }
}

uint8_t *icon_cb (uint8_t idx)
{
  switch (idx) {
    case 0:
      return batt_icon;

    default:
      // oops someone forgot a callback
      return &font['$' * 5]; 
  }
  return NULL;
}


static uint8_t ui_event_notify (uint8_t event, uint16_t data)
{
  switch (event) {
    case EVENT_KEYPRESS:
      // Pass keypresses to menu
      menu.KeyHandler ((uint8_t)data);
      break;

    case EVENT_ICON_UPDT:
      //UpdateIcons((icon_data_t *)data);
      break;

    case EVENT_SD_DET:
      UpdateStatus ("SD Card detected");
      break;

    case EVENT_SD_RMV:
      UpdateStatus ("SD Card removed");
      break;

    case EVENT_VBATT:
      UpdateVbatt ((uint8_t)data);
      break;

    case EVENT_SERIAL_RECV:
      cmdp_parse_cmd((string_t *)data);
      break;

    default:
      // do nothing
      break;
  }

  // Always return true - consume all events
  return 1;
}

// TODO: Move to main
void process_usb (uint16_t ticks) {
  ser.process();
}

void ui_setup(void)
{
  // Register self as initial event handler, never gets popped
  evt_handler_addhandler(&ui_event_notify);

  // Init Timetick subsystem
  timetick_init();

  // Init command parser
  cmdp_init();

  // Start stillalive led 500ms
  OUTPUT(led);
  HIGH(led);
  timetick_register(&led_keepalive, 50);

  // process usb every 20ms
  timetick_register(&process_usb, 2);

  // Start keypad - sample at 70ms
  keypad_init(7);

  // LCD init + splashscreen
  oled.init(SSD1306_SWITCHCAPVCC);

  // Register battery icon
  oled.registericon(0, &icon_cb);
  
  // Show splashscreen
  oled.drawbitmap(logo, 128, 64);
  _delay_ms (2000);

  // Draw battery icon
  oled.drawicon(0, 7, 19, 2); /* vbatt, line=7, x=19, w=2 */
  menu.DrawMenu();  
}

void ui_process(void)
{
  // Dispatch any outstanding events
  // TODO: move somewhere else
  evt_handler_dispatch();
}
