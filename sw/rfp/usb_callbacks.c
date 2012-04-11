#include "usb_vserial.h"

// defined in usb_desc.c
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

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

