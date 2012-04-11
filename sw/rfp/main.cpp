#include "hw.h"
#include "ui.h"

#include "usb_serial.h"

int main (int argc, char **argv)
{
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  // Assert pshold to stay powered on
  OUTPUT(pshold);
  HIGH(pshold);

  // En fast batt charging
  OUTPUT(usb_i_sel);
  LOW(usb_i_sel);

  // Init UI
  ui_setup();

  // do nothing
  while(1) {
    // Usb processing
    //ser.process(); done on timer

    // Update ui
    ui_process();
  }
}

