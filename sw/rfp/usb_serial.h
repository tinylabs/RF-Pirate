#ifndef _USBSERIAL_H_
#define _USBSERIAL_H_
/**
 * Virtual serial wrapper over usb. Buffers input and output.
 * Wraps around Dean Camera LUFA lib to provide line buffering.
 *
 * Elliot Buller 2012
 */
#include <stdarg.h>
#include <stdint.h>

#include "usb_vserial.h"

// defined in Descriptors.c
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;
typedef void (*serial_recv_cb)(char *buf, uint8_t len);

// Used by clients to release buffer after processing
#define RELEASE_BUF(x) (x[0]='\0')

/**
 * Configuration Parameters
 */
#define USB_SERIAL_BUF_SZ 30
#define RX_DEPTH          2
#define TX_DEPTH          5
#define ENABLE_ECHO

// Receive buffer data structure
typedef struct {
  char    data[USB_SERIAL_BUF_SZ];
  uint8_t len;
} string_t;

class UsbSerial {
 public:
  UsbSerial(void);                          // constructor
  void process(void);                       // process input and output
  void printf(const char *fmt, ...);        // buffered printf

 private:
  void buffer_rx(char b);

  // Data members
  FILE stream;
  // Track head/tail of each buffer
  uint8_t rx_p, rx_idx;
  uint8_t tx_h, tx_t;
  // Storage
  char tx_q[TX_DEPTH][USB_SERIAL_BUF_SZ];
  string_t rx_q[RX_DEPTH];
};

// Extern global instance for all to use
extern UsbSerial ser;

#endif /* _USBSERIAL_H_ */
