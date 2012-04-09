/*
 * Driver for Si4432 rf transceiver. Tunable from 240-960MHz.
 * Capable of OOK, FSK, GFSK modulation.
 *
 * Elliot Buller 2012
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#include "si4432.h"
#include "evt_handler.h"
#include "system.h"
#include "hw.h"

#define SPI_BAUD_4MHZ    0
#define SPI_BAUD_2MHZ    1
#define SPI_BAUD_1p3MHZ  2
#define SPI_BAUD_1MHZ    3

// Spi access functions
static inline void _spi_write(uint8_t data)
{
  uint8_t dummy;
  /* Wait for empty transmit buffer */
  while ( !( UCSR1A & (1<<UDRE1)) );
  UDR1 = data;
    /* Wait for transmit to complete */
  while ( !( UCSR1A & (1<<TXC1)) );
  /* Wait for data to be received */
  while ( !(UCSR1A & (1<<RXC1)) );
  // Throw byte away
  dummy = UDR1;
}

// Spi access functions
static inline uint8_t _spi_read(uint8_t data)
{
  uint8_t d;
  /* Wait for empty transmit buffer */
  while ( !( UCSR1A & (1<<UDRE1)) );
  UDR1 = data;
    /* Wait for transmit to complete */
  while ( !( UCSR1A & (1<<TXC1)) );
  /* Wait for data to be received */
  while ( !(UCSR1A & (1<<RXC1)) );
  // Return byte
  d = UDR1;
  return d;
}

void rf_spi_writem (uint8_t addr, uint8_t *buf, uint8_t sz)
{
  uint8_t i;

  // Asset cs
  HIGH(rf_cs);
  LOW(rf_cs);
  /* Write address */
  _spi_write(addr | 0x80);
  
  // Send all data
  for (i = 0; i < sz; i++) {
    _spi_write(*buf);
    buf++;
  }
  // De-assert cs
  HIGH(rf_cs);
}

void rf_spi_readm (uint8_t addr, uint8_t *buf, uint8_t sz)
{
  uint8_t i;

  // Assert cs
  HIGH(rf_cs);
  LOW(rf_cs);
  // Write address
  _spi_write(addr & 0x7f);
  
  // Loop through all bytes to receives
  for (i = 0; i < sz; i++) {
    // Send dummy byte to generate clock
    *buf = _spi_read (0);
    buf++;
  }
  // De-assert cs
  HIGH(rf_cs);
}

void rf_spi_write (uint8_t addr, uint8_t data)
{
  rf_spi_writem (addr, &data, 1);
}

uint8_t rf_spi_read (uint8_t addr)
{
  uint8_t d;
  rf_spi_readm (addr, &d, 1);
  return d;
}

/* Initialize MSPIM */
static void rf_spi_init(uint16_t baud)
{
  UBRR1 = 0;
  /* Setting the XCKn port pin as output, enables master mode. */
  OUTPUT(xck);
  /* Not using interrupts */
  UCSR1A = 0;
  /* Set MSPI mode of operation and SPI data mode 0. */
  UCSR1C = (1<<UMSEL11)|(1<<UMSEL10);//|(0<<UCPHA1)|(0<<UCPOL1);
  /* Enable receiver and transmitter. */
  UCSR1B = (1<<RXEN1)|(1<<TXEN1);
  /* Set baud rate. */
  /* IMPORTANT: The Baud Rate must be set after the transmitter is
     enabled */
  UBRR1 = baud;  
}

static void rf_config_gpios (void)
{
  // Setup IOs
  OUTPUT(rf_sdn);
  OUTPUT(rf_cs);
  INPUT(rf_irq);
  HIGH(rf_irq);  // Configure pullup
  INPUT(rf_gpio); // debug rx data
  HIGH(rf_cs);
  // Force device out of shutdown
  LOW(rf_sdn);
  // Wait for device to stabilize
  _delay_ms(10);
}

static uint8_t rf_initialized = 0;

/* Init hardware and verify communications */
uint8_t rf_probe(void)
{
  if (rf_initialized)
    return 1;
  
  // Configure gpios
  rf_config_gpios();

  // Init spi hw
  rf_spi_init(8);

  // Clear any interrupts
  rf_spi_read(3);
  rf_spi_read(4);

  // Force software reset
  rf_spi_write(7, 0x80);

  // Wait until interrupt is asserted
  while (READ(rf_irq));

  // Setup gpio tx/rx switch selects
  rf_spi_write(0x0b, 0x92);  //gpio0=tx_state
  rf_spi_write(0x0c, 0x95);  //gpio1=rx_state
  rf_spi_write(0x0d, 0x14);  //gpio2=rx data

  // Read type + version reg
  if (rf_spi_read(0) != 8) // silicon B1
    return 0;
  if (rf_spi_read(1) != 6)
    return 0;

  // Si4432 detected
  rf_initialized = 1;
  return 1;
}


// Set mode
void rf_set_mode (rf_mode_t mode)
{

}

// Generic RF
void rf_set_freq (float freq_mhz)
{
  uint8_t hbsel;
  uint16_t fb, fc;

  if (freq_mhz < 240.0 || freq_mhz > 960.0)
    return;

  // Are we in the high band or low band?
  hbsel = (freq_mhz >= 480.0) ? 1 : 0;
  
  // Calculate fb[4:0]
  fb = hbsel ? (uint16_t)(freq_mhz/2) : (uint16_t)freq_mhz;
  fb = (fb - 240) / 10;

  // clear frequency offset
  rf_spi_write(0x73, 0);
  rf_spi_write(0x74, 0);

  // Program band select
  rf_spi_write(0x75, (uint8_t)fb | (hbsel << 5));

  // Calculate fc[15:0]
  fc = (uint16_t)(((freq_mhz/((float)10 * (hbsel + 1))) 
		   - fb - 24) * (float)64000);
  // Program nominal carrier
  rf_spi_writem(0x76, (uint8_t *)&fc, 2);

  // No freq hopping
  rf_spi_write(0x79, 0);
}


void rf_set_mod_src (rf_mod_t mod, rf_src_t src)
{
  uint8_t tmp;
  if (mod >= MOD_MAX || src >= SRC_MAX)
    return;
  
  tmp = rf_spi_read(0x71);
  tmp &= ~0x33;
  tmp = (src << 4) | mod;
  rf_spi_write(0x71, tmp);
}

// RF TX control
void rf_set_tx_rate (uint16_t rate_kbps)
{
  uint8_t scale, tmp;
  uint16_t dr;

  if (rate_kbps > 1000)
    return;
  
  // calc reg value
  scale = (rate_kbps <= 31) ? 1 : 0;
  if (scale)
    dr = (uint16_t)(rate_kbps * 2097.152f);
  else
    dr = (uint16_t)(rate_kbps * 65.536f);

  // Program data rate
  rf_spi_writem(0x6e, (uint8_t *)&dr, 2);

  // program scale flag
  tmp = rf_spi_read(0x70);
  tmp &= ~(1 << 5);
  tmp |= (scale << 5);
  rf_spi_write(0x70, tmp);
}

void rf_set_power (rf_power_t pwr)
{
  if (pwr >= RF_MAX_DBM)
    return;

  rf_spi_write (0x6d, pwr);
}

// FIFO access
void rf_fifo_write(uint8_t *buf, uint8_t sz)
{
  rf_spi_writem(0x7f, buf, sz);
}
void rf_fifo_read(uint8_t *buf, uint8_t sz)
{
  rf_spi_readm(0x7f, buf, sz);
}

// ISR control fuctions
void rf_enable_isr(uint16_t mask)
{
  uint16_t tmp;
  rf_spi_readm(5, (uint8_t *)&tmp, 2);
  tmp |= mask;
  rf_spi_writem(5, (uint8_t *)&tmp, 2);

  // Setup PCINT4 for ISR
  PCMSK0 |= (1 << 4);
  PCICR = 1;
  sei();
}

void rf_disable_isr(uint16_t mask)
{
  uint16_t tmp;
  rf_spi_readm(5, (uint8_t *)&tmp, 2);
  tmp &= ~mask;
  rf_spi_writem(5, (uint8_t *)&tmp, 2);

  PCMSK0 &= ~(1 << 4);
  PCICR = 0;
}

// Map irqs to events
static const uint8_t rf_irq_evt[] = {
  EVENT_ISR_FIFO_UNDOVR,
  EVENT_ISR_FIFO_TXHI,
  EVENT_ISR_FIFO_TXLO,
  EVENT_ISR_FIFO_RXHI,
  EVENT_ISR_EXT_INTEN,
  EVENT_ISR_PKT_SENT,
  EVENT_ISR_PKT_RCVD,
  EVENT_ISR_CRC_ERR,
  EVENT_ISR_SYNC_DET,
  EVENT_ISR_VAL_PRM_DET,
  EVENT_ISR_INV_PRM_DET,
  EVENT_ISR_RSSI,
  EVENT_ISR_WAKE_TMR,
  EVENT_ISR_LOW_BATT,
  EVENT_ISR_XTAL_RDY,
  EVENT_ISR_POR_EN,
};


/* RF IRQ interrupt */
ISR(PCINT0_vect)
{
  uint8_t i, s3, s4;
  uint16_t irq;

  // Make sure RF_IRQ is low
  //if (!READ(rf_irq)) {
    // Read RF irq events
    rf_spi_readm(3, (uint8_t *)&irq, 2);
    // Generate async event for each rf irq
    for (i = 0; irq && (i < 16); i++) {
      if (irq & 0x8000) {
	evt_handler_event(rf_irq_evt[i], 0);
	irq <<= 1;
      }
    }// end for
    //}// end if
}
