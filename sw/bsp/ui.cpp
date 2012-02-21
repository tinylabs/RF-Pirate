/**
 * UI events handlers
 * Elliot Buller 2012
 **/
#include <string.h>
#include <stdio.h>

#include <util/delay.h>

#include "ssd1306.h"
#include "Menu.h"

#include "timetick.h"
#include "keypad.h"
#include "hw.h"

// Apps
#include "rf_test.h"

// logo + icons
#include "logo.h"

// Include battery icons
#include "batt_0p0.h"
#include "batt_0p25.h"
#include "batt_0p5.h"
#include "batt_0p75.h"
#include "batt_1p0.h"

// pointer to battery icon
uint8_t *batt_icon;

/* Core objects */
ssd1306 oled;
EXPORT_MENU(m_root);
Menu menu(m_root, &oled, 1, 5);

// Jump to the bootloader
typedef void (*remote_call_t) (void);
remote_call_t bootloader = (remote_call_t)0x3800;


void shutdown ();
void fast_charge_on(void) { HIGH(usb_i_sel); oled.poweroff(); }

MenuEntry m_settings[] = {
  MenuEntry ("Batt charge mode", &fast_charge_on),
  NULL
};

MenuEntry m_root[] = {
  MenuEntry ("RF", m_rf_root, &rf_event_notify),
  MenuEntry ("Settings", m_settings),
  MenuEntry ("Bootloader", bootloader),
  MenuEntry ("Shutdown", &shutdown),
  NULL
};



void led_keepalive (uint16_t ticks)
{
  TOGGLE(led);
}

void shutdown ()
{
  // Turn off LCD
  oled.poweroff();
  // Release pshold
  LOW(pshold);
}

#if 0
void debug (char *fmt, uint16_t data)
{
  char str[21];
  static uint8_t line = 0;
  if (line > 6)
    line = 0;
  snprintf (str, sizeof(str), fmt, data);
  oled.drawstring(line, 0, str, 0);
  oled.display ();
  line++;
}
#endif

void UpdateStatus (const char *str)
{
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
  else { //empty
    batt_icon = batt_0p0;
  }
}

uint8_t *icon_cb (uint8_t idx)
{
  switch (idx) {
    case 0:
      return batt_icon;

    default:
      break;
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

    default:
      // do nothing
      break;
  }

  // Always return true - consume all events
  return 1;
}

void ui_setup(void)
{
  // Register self as initial event handler, never gets popped
  evt_handler_addhandler(&ui_event_notify);

  // Init Timetick subsystem
  timetick_init();

  // Start stillalive led 500ms
  OUTPUT(led);
  HIGH(led);
  //timetick_register(&led_keepalive, 100);

  // Start keypad - sample at 60ms
  keypad_init(8);

  // LCD init + splashscreen
  oled.init(SSD1306_SWITCHCAPVCC);

  // Register battery icon
  oled.registericon(0, &icon_cb);
  
  oled.drawbitmap(logo, 128, 64);
  _delay_ms (2000);

  // Draw battery icon
  oled.drawicon(0, 7, 19, 2); /* vbatt, line=7, x=19, w=2 */
  menu.DrawMenu();  
}

void ui_process(void)
{
  // Dispatch any outstanding events
  evt_handler_dispatch();
}
