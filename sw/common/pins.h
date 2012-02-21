// All pin definitions should be defined here
// This file shall never be #included'd in any source or header
// It get's preprocessed by perl into _pins.h which then is included in hw.h
// All code should include hw.h


// Tried to recursively define preprocessors but gcc preproc doesn't like
// that. ;( Instead we preprocess pins.h in perl to produce pins_auto.h
//#define PIN_DEFINE(name,port,pin)      #define name##_PORT PORT##port \
//                                       #define name##_PIN  _BV(pin)   \
//                                       #define name##_DIR  DDR##port

// Port B
PIN_DEFINE(lcd_dc, B, 0);
PIN_DEFINE(spi_sck, B, 1);
PIN_DEFINE(spi_mosi, B, 2);
PIN_DEFINE(spi_miso, B, 3);
PIN_DEFINE(rf_irq, B, 4);
PIN_DEFINE(rf_gpio, B, 5);
PIN_DEFINE(_spare, B, 6);
PIN_DEFINE(lcd_rst, B, 7);

// Port C
PIN_DEFINE(usb_i_sel, C, 6);
PIN_DEFINE(pshold, C, 7);

// Port D
PIN_DEFINE(scl, D, 0);
PIN_DEFINE(sda, D, 1);
PIN_DEFINE(rx, D, 2);
PIN_DEFINE(tx, D, 3);
PIN_DEFINE(rf_cs, D, 4);
PIN_DEFINE(xck, D, 5);
PIN_DEFINE(rf_sdn, D, 6);
PIN_DEFINE(lcd_cs, D, 7);

// Port E
PIN_DEFINE(hwb, E, 2);
PIN_DEFINE(sd_det, E, 6);

// Port F
PIN_DEFINE(led, F, 0);
PIN_DEFINE(sd_cs, F, 1);
PIN_DEFINE(sd_pwr, F, 4);
PIN_DEFINE(btn1, F, 5);
PIN_DEFINE(btn2, F, 6);
PIN_DEFINE(vbatt, F, 7);
