#include "usb_serial.h"
#include "evt_handler.h"

#define WRAP(i, depth) (i = (i >= depth) ? 0: i++)

/* Single instance of serial usb device. Provides line buffering for input
 * and circular buffering for output */
UsbSerial ser;

/* Constructor - setup usb stream and init USB stack */
UsbSerial::UsbSerial (void)
{
  uint8_t i;

  // init class data
  rx_p = rx_idx = 0;
  tx_h = tx_t = 0;

  for (i = 0; i < RX_DEPTH; i++) {
    rx_q[i].data[0] = '\0';
  }
  for (i = 0; i < TX_DEPTH; i++) {
    tx_q[i][0] = '\0';
  }
  
  USB_Init();
  CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &stream);
}

/* We will overwrite any receive data if we run out of buffer space */
void UsbSerial::buffer_rx(char b)
{
  // Check for line ending
  if (b == '\r' || b == '\n') {
    goto dispatch;
  }

#ifdef ENABLE_ECHO
  // echo back to host
  CDC_Device_SendByte (&VirtualSerial_CDC_Interface, b);
#endif /* ENABLE_ECHO */

  // Check for overflows
  if ((rx_idx == 0) && (rx_q[rx_p].data[0] != '\0')) {
    fprintf(&stream, "RX OVFLW\r\n");    
  }

  // add byte to buffer if not backspace
  if (b == '\b' && rx_idx > 0)
    rx_idx--;
  else
    rx_q[rx_p].data[rx_idx++] = b;

  // Make sure we are not at the end
  if (rx_idx == USB_SERIAL_BUF_SZ - 1) {
    goto dispatch;
  }

  return;

  // Instead of dispatching now we post it to the event
  // q and continue to buffer the next command. That way the
  // latest event handler will be able to handle serial input

 dispatch:
    rx_q[rx_p].data[rx_idx] = '\0';
    rx_q[rx_p].len = rx_idx + 1; // include null
    fprintf(&stream, "\r\n"); // send CR/LF

    // dispatch event
    evt_handler_event(EVENT_SERIAL_RECV, (uint16_t)&rx_q[rx_p]);
    rx_idx = 0;
    WRAP(rx_p, RX_DEPTH);
}

void UsbSerial::process (void)
{
  int8_t b_avail;

  // buffer rx data
  b_avail = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);
  while (b_avail-- > 0) {
    char b = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    buffer_rx(b);
  }

  // process tx buffers
  if (tx_q[tx_t][0] != '\0') {
    fprintf(&stream, "%s", &tx_q[tx_t][0]);
    tx_q[tx_t++][0] = '\0';
    WRAP(tx_t, TX_DEPTH);
  }

  // call usb stack processing
  CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
  USB_USBTask();
}

void UsbSerial::printf (const char *fmt, ...)
{
  va_list args;
  
  // variable argument length
  va_start(args, fmt);

  // If no buffer is available just return
  if (tx_q[tx_h][0] != '\0')
    return;

  // print into buffer, truncate if necessary
  vsnprintf (&tx_q[tx_h++][0], USB_SERIAL_BUF_SZ, 
	     fmt, args);
  va_end(args);
  // increment head
  WRAP(tx_h, TX_DEPTH);
}
