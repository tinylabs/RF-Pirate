#include "hw.h"
#include "ui.h"


int main (int argc, char **argv)
{
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
    // Add usb processing here

    // Update ui
    ui_process();
  }
}
