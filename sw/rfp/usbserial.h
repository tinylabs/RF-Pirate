#ifndef _USBSERIAL_H_
#define _USBSERIAL_H_
/**
 * Virtual serial driver over usb. Buffers input and output.
 * Wraps around Dean Camera LUFA lib to provide line buffering.
 *
 * Elliot Buller 2012
 */
#include "VirtualSerial.h"
#include <stdarg.h>
#include <stdint.h>

// defined in Descriptors.c
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

typedef void (*serial_recv_cb)(char *buf, uint8_t len);

#define USB_SERIAL_BUF_SZ 30
#define RX_DEPTH          2
#define TX_DEPTH          4

class UsbSerial {
 public:
  UsbSerial(void);                          // constructor
  void process(void);                       // process input and output
  void printf(const char *fmt, ...);        // buffered printf
  void register_recv_cb(serial_recv_cb cb); // cb when line is recvd

 private:
  void buffer_rx(char b);

  // Data members
  FILE stream;
  serial_recv_cb recv_cb;
  // Track head/tail of each buffer
  uint8_t rx_h, rx_t, rx_idx;
  uint8_t tx_h, tx_t;
  // Storage
  char rx_q[RX_DEPTH][USB_SERIAL_BUF_SZ];
  char tx_q[TX_DEPTH][USB_SERIAL_BUF_SZ];
};

// Extern global instance for all to use
extern UsbSerial ser;

#endif /* _USBSERIAL_H_ */
