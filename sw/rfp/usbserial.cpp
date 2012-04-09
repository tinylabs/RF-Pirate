#include "usbserial.h"

#define WRAP(i, depth) (i = (i >= depth) ? 0: i++)

/* Constructor - setup usb stream and init USB stack */
UsbSerial::UsbSerial (void)
{
  uint8_t i;

  // init class data
  rx_h = rx_t = rx_idx = 0;
  tx_h = tx_t = 0;
  recv_cb = NULL;

  for (i = 0; i < RX_DEPTH; i++) {
    rx_q[i][0] = '\0';
  }
  for (i = 0; i < TX_DEPTH; i++) {
    tx_q[i][0] = '\0';
  }
  
  USB_Init();
  CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &stream);
}

void UsbSerial::buffer_rx(char b)
{
  // Check for line ending
  if (b == '\r' || b == '\n') {
    goto dispatch;
  }
    
  // echo back to host
  CDC_Device_SendByte (&VirtualSerial_CDC_Interface, b);

  // add byte to buffer if not backspace
  if (b == '\b' && rx_idx > 0)
    rx_idx--;
  else
    rx_q[rx_h][rx_idx++] = b;

  // Make sure we are not at the end
  if (rx_idx == USB_SERIAL_BUF_SZ - 1) {
    goto dispatch;
  }

  return;

 dispatch:
    rx_q[rx_h][rx_idx] = '\0';
    fprintf(&stream, "\r\n"); // send CR/LF
    // dispatch
    if (recv_cb != NULL) {
      recv_cb (&rx_q[rx_h][0], rx_idx);
    }
    rx_idx = 0;
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

void UsbSerial::register_recv_cb(serial_recv_cb cb)
{
  recv_cb = cb;
}

