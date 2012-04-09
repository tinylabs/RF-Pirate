#include "hw.h"
#include "ui.h"

#include "usbserial.h"

UsbSerial ser;

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
    // Add usb processing here
    ser.process();

    // Update ui
    ui_process();
  }
}

/*
 * USB EVENT CALLBACKS
 */

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
  //LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
  //LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
  bool ConfigSuccess = true;
  
  ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
  //LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Unhandled Control Request event. */
void EVENT_USB_Device_UnhandledControlRequest(void)
{
  CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

