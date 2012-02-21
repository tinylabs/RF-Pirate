#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "MenuEntry.h"
#include "rf_test.h"
#include "context.h"
#include "si4432.h"
#include "hw.h"

typedef struct {
  uint8_t addr;
  uint8_t val;
} rf_reg;

static context_t *sys;

static void print (uint8_t line, char *fmt, uint8_t data)
{
  char str[21];
  
  snprintf(str, sizeof(str), fmt, data);
  sys->disp->clearline(line);
  sys->disp->drawstring (line, 0, str, 0);
  sys->disp->display();
}

// Handle all events except back key to quit
uint8_t rf_event_notify (uint8_t event, uint16_t data)
{
  switch (event) {
    case EVENT_APP_START:
      sys = (context_t *)data;
      // Carve out some lcd space to display
      sys->menu->SetLineStartEnd(1, 2);
      // Initialize the rf card
      rf_probe();
      //if (!rf_probe())
      //sys->disp->drawstring(6, 0, " RF Init failed", 1);
      return true;
      break;

    case EVENT_APP_STOP:
      // Restore menu
      sys->menu->SetLineStartEnd(1, 5);
      // Move this into rf driver
      //HIGH(rf_sdn);
      HIGH(led);
      return true;
      break;

      // Ignore these events
    case EVENT_VBATT:
    case EVENT_KEYPRESS:
      //print (6, "keycode=%02x", data);
      break;

      // RF events
    case EVENT_ISR_FIFO_RXHI:
      print (7, "RX fifo hi", 0);
      break;
    case ISR_SYNC_DET:
      print (7, "SYNC det", 0);
      break;
    case ISR_VAL_PRM_DET:
      print (7, "PREAMBLE det", 0);
      break;
    case ISR_PKT_RCVD:
      print (7, "PKT received", 0);
      break;
    
    default:
      LOW(led); //debug
      print (6, "Event = %02x", event);
      break;
  }
  return false;  // Pass events to system
}

static void dump_rf_regs (uint8_t start, uint8_t end)
{
  uint8_t left, line = 3;
  uint8_t buf[7]; // reg buffer
  char str[21];
  
  end = end > 127 ? 127 : end;
  left = end - start;

  // Clear lines 3 - 6
  sys->disp->clearline(line);
  sys->disp->clearline(line+1);
  sys->disp->clearline(line+2);
  sys->disp->clearline(line+3);

  while (left > 0) {
    memset (buf, 0, sizeof(buf));
    // Read regs from si4432
    rf_spi_readm(end - left, buf, left < 7 ? left : 7);
    // Print regs into buffer
    snprintf (str, sizeof(str), "%02x %02x %02x %02x %02x %02x %02x",
	      buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
    // print to display
    sys->disp->drawstring (line, 0, str, 0);
    sys->disp->display();    
    left = (left < 7) ? 0 : left - 7;
    line++;
  }
}

void reg_0_27 (void) { dump_rf_regs(0, 28); }
void reg_28_55 (void) { dump_rf_regs(28, 56); }
void reg_56_83 (void) { dump_rf_regs(56, 84); }
void reg_84_111 (void) { dump_rf_regs(84, 112); }
void reg_112_127 (void) { dump_rf_regs(112, 128); }
void reg_exit(void) {
  // Clear lines 3 - 6
  sys->disp->clearline(3);
  sys->disp->clearline(4);
  sys->disp->clearline(5);
  sys->disp->clearline(6);
  // Generate left key
  evt_handler_event(EVENT_KEYPRESS, KEY_LEFT);
}


void keeloq_315_tx (void)
{
  uint8_t i;
  uint8_t fifo[32];
  
  // Setup packet handling
  rf_spi_write(0x32, 0);    // no broadcast check, no header check
  rf_spi_write(0x33, 0xa);  // hdr len=0, fixpktlen=1, synclen=2
  rf_spi_write(0x34, 0x3);  // preamble - 3 nibbles = 12 bits
  rf_spi_write(0x36, 0xff); // sync words
  rf_spi_write(0x37, 0x00); 
  rf_spi_write(0x3e, 8);    // packet length

  // data rate = 64kbps
  rf_spi_write (0x6e, 0x10);
  rf_spi_write (0x6f, 0x62);
  rf_spi_write (0x58, 0x80); //according to datasheet < 100kbps

  // Set tx power to +20dBm
  rf_set_power (RF_20_DBM);

  // Manchester disable
  rf_spi_write(0x70, 0x00);

  // Setup modulation
  //rf_set_mod_src(MOD_OOK, SRC_FIFO);
  rf_spi_write(0x71, 0x21);

  // Tune to 315 Mhz
  rf_set_freq(315.0);

  // Fill in fifo
  for (i = 0; i < 32; i++) {
    fifo[i] = 2;
  }

  // Write fifo to radio
  rf_fifo_write (fifo, sizeof(fifo));

  // Turn on tx_on
  rf_spi_write(0x7, 0x9);

  // Wait for packet to complete
  while (rf_spi_read(7) & 0x8);
}

void keeloq_315_rx (void)
{
  uint8_t i;
  uint8_t fifo[32];
  
  // Setup packet handling
  rf_spi_write(0x30, 0xd);  // disable packet handling
  rf_spi_write(0x32, 0);    // no broadcast check, no header check
  rf_spi_write(0x33, 0xa);  // hdr len=0, fixpktlen=1, synclen=2
  //rf_spi_write(0x34, 0x3);  // preamble - 3 nibbles = 12 bits
  rf_spi_write(0x35, 3<<3);  // preamble - 3 nibbles = 12 bits
  rf_spi_write(0x36, 0xff); // sync words
  rf_spi_write(0x37, 0x00); 
  //rf_spi_write(0x3e, 8);    // packet length

  // set invalid preamble threshold
  rf_spi_write (0x60, 0xf0);

  // data rate = 64kbps
  rf_spi_write (0x6e, 0x10);
  rf_spi_write (0x6f, 0x62);
  rf_spi_write (0x58, 0x80); //according to datasheet < 100kbps

  // Set tx power to +20dBm
  //rf_set_power (RF_20_DBM);

  // Manchester disable
  rf_spi_write(0x70, 0x00);

  // Setup modulation
  //rf_set_mod_src(MOD_OOK, SRC_FIFO);
  rf_spi_write(0x71, 0x21);

  // Tune to 315 Mhz
  rf_set_freq(315.0);

  // Setup IF bandwidth = 142.8kHz
  rf_spi_write(0x1c, 0x94);

  // Setup clock recovery
  rf_spi_write(0x20, 94);

  // ook counter 1
  rf_spi_write(0x2c, 0x30);

  // Clear RX fifo
  rf_spi_write(0x08, 2);
  rf_spi_write(0x08, 0);

  // Enable interrupts
  rf_spi_write(0x5, 2);
  rf_spi_write(0x6, 0xc0);
  PCMSK0 |= (1 << 4);
  PCICR = 1;
  sei();
  
  //rf_enable_isr(ISR_SYNC_DET | ISR_VAL_PRM_DET | ISR_PKT_RCVD);
  //rf_enable_isr(ISR_MASK_ALL);

  // Turn on reciever
  rf_spi_write(0x7, 0x4);
}

rf_reg rx_regs[] = {
  {0x1C, 0x81},  // IF filter bw
  {0x1D, 0x3C},  // AFC Loop Gearshift Override
  {0x1E, 0x02},  // AFC timing control
  {0x1F, 0x03},  // Clock recovery gearshift
  {0x20, 0xBC},  // Clock recovery oversampling ratio
  {0x21, 0x00},  // Clock recovery offset 2
  {0x22, 0xAE},  // Clock recovery offset 1
  {0x23, 0xC3},  // Clock recovery offset 0
  {0x24, 0x00},  // Clock recovery timing loop gain 1
  {0x25, 0xB0},  // Clock recovery timing loop gain 0
  {0x2A, 0xFF},  // AFC limiter
  {0x2C, 0x28},  // OOK counter val 1
  {0x2D, 0x13},  // OOK counter val 2
  {0x2E, 0x29},  // Slicer peak hold
  {0x30, 0x04},  // Data access control - autopkt=0 crc=0
  {0x32, 0x00},  // Header control 1 - No broadcast or header check
  {0x33, 0x8A},  // Header control 2 - no header, sync=2 bytes
  {0x34, 0x03},  // Preamble length
  {0x35, 3<<3},  // Preamble detection control
  {0x36, 0xFF},  // Sync word 3
  {0x37, 0x00},  // Sync word 2
  {0x38, 0x00},  // Sync word 1
  {0x39, 0x00},  // Sync word 0
  {0x58, 0x80},  // undocumented - >30kbps=0x80 <30kbps=0xC0 
  {0x69, 0x60},  // AGC override 1
  {0x6E, 0x10},  // TX data rate 1
  {0x6F, 0x62},  // TX data rate 2
  {0x70, 0x04},  // Modulation mode control 1
  {0x71, 0x21},  // Modulation mode control 2
  {0x72, 0x50},  // Frequency deviation
  {0x75, 0x47},  // Frequency band select
  {0x76, 0x7D},  // Nominal carrier freq 1
  {0x77, 0x00},  // Nominal carrier freq 0
};

void rx_315(void)
{
  uint8_t i;

  // Write to si4432
  for (i = 0; i < sizeof(rx_regs)/sizeof(rx_regs[0]); i++) {
    rf_spi_write(rx_regs[i].addr, rx_regs[i].val);
  }

  // Clear rx fifo
  rf_spi_write(0x8, 0x2);
  // Set rx threshold
  rf_spi_write(0x7e, 0x23);
  
  // Enable interrupts
  rf_spi_write(0x5, 0x92);
  rf_spi_write(0x6, 0xC0);
  PCMSK0 |= (1 << 4);
  PCICR = 1;
  sei();
  
  // Turn on reciever
  rf_spi_write(0x7, 0x4);
}

void dump_rx_fifo (void)
{
  uint8_t i, line = 3;
  char str[21];
  uint8_t byte[7];

  // Clear lines
  sys->disp->clearline(line);
  sys->disp->clearline(line+1);
  sys->disp->clearline(line+2);
  sys->disp->clearline(line+3);
  sys->disp->clearline(line+4);
  
  for (i = 0; i < 5; i++) {
    rf_fifo_read(byte, 7);
    snprintf (str, sizeof(str), "%02x %02x %02x %02x %02x %02x %02x",
	      byte[0], byte[1], byte[2], byte[3], byte[4], 
	      byte[5], byte[6]);
    sys->disp->drawstring (line, 0, str, 0);
    sys->disp->display();
    line++;
  }
}

MenuEntry m_rf_reg[] = {
  MenuEntry("Regs 0-27", &reg_0_27),
  MenuEntry("Regs 28-55", &reg_28_55),
  MenuEntry("Regs 56-83", &reg_56_83),
  MenuEntry("Regs 84-111", &reg_84_111),
  MenuEntry("Regs 112-127", &reg_112_127),
  MenuEntry("Exit", &reg_exit),
  NULL
};

/* Menu Entrys */
MenuEntry m_rf_root[] = {
  MenuEntry("Debug registers", m_rf_reg),
  MenuEntry("RX @ 315", &rx_315),
  MenuEntry("RX buffer", &dump_rx_fifo),
  NULL
};
