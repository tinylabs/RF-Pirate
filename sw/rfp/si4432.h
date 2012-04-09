#ifndef _SI4432_H_
#define _SI4432_H_
/*
 * Driver for Si4432 rf transceiver. Tunable from 240-960MHz.
 * Capable of OOK, FSK, GFSK modulation.
 *
 * Elliot Buller 2012
 */
#include <stdint.h>

/* Tuning step size */
typedef enum {
  STEP_SZ_156HZ,
  STEP_SZ_312HZ,
  STEP_SZ_MAX
} rf_step_sz_t;

typedef enum {
  MODE_POWERON,
  MODE_SHUTDOWN,
  MODE_STANDBY,
  MODE_SLEEP,
  MODE_READY,
  MODE_TUNE,
  MODE_TX,
  MODE_RX,
  MODE_MAX
} rf_mode_t;

typedef enum {
  MOD_CARRIER = 0,
  MOD_OOK,
  MOD_FSK,
  MOD_GFSK,
  MOD_MAX
} rf_mod_t;

typedef enum {
  SRC_DIR_GPIO = 0,
  SRC_DIR_SDI,
  SRC_FIFO,
  SRC_PN9,
  SRC_MAX
} rf_src_t;

typedef enum {
  RF_1_DBM = 0,
  RF_2_DBM,
  RF_5_DBM,
  RF_8_DBM,
  RF_11_DBM,
  RF_14_DBM,
  RF_17_DBM,
  RF_20_DBM,
  RF_MAX_DBM
} rf_power_t;

// Init hw and verify comms
uint8_t rf_probe (void);

// Set mode
void rf_set_mode (rf_mode_t mode);

// Spi access functions
uint8_t rf_spi_read (uint8_t addr);
void rf_spi_readm (uint8_t addr, uint8_t *buf, uint8_t sz);
void rf_spi_write (uint8_t addr, uint8_t data);
void rf_spi_writem (uint8_t addr, uint8_t *buf, uint8_t sz);

// Generic RF
void rf_set_freq (float freq_mhz);
void rf_set_mod_src (rf_mod_t mod, rf_src_t src);

// RF TX control
void rf_set_tx_rate (uint16_t rate_kbps);
void rf_set_power (rf_power_t power);

// FIFO access
void rf_fifo_write(uint8_t *buf, uint8_t sz);
void rf_fifo_read(uint8_t *buf, uint8_t sz);

// ISR maskable bits
// ISR reg 5
#define ISR_FIFO_UNDOVR  0x8000  // fifo underflow/overflow
#define ISR_FIFO_TXHI    0x4000  // tx high watermark
#define ISR_FIFO_TXLO    0x2000  // tx low watermark
#define ISR_FIFO_RXHI    0x1000  // rx high watermark
#define ISR_EXT_INTEN    0x0800  // external int enabled
#define ISR_PKT_SENT     0x0400  // packet sent
#define ISR_PKT_RCVD     0x0200  // packet received
#define ISR_CRC_ERR      0x0100  // CRC error
// ISR reg 6
#define ISR_SYNC_DET     0x80    // sync word detected
#define ISR_VAL_PRM_DET  0x40    // valid preamble detected
#define ISR_INV_PRM_DET  0x20    // invalid preamble
#define ISR_RSSI         0x10    // rssi available
#define ISR_WAKE_TMR     0x08    // Wake up timer
#define ISR_LOW_BATT     0x04    // Low battery
#define ISR_XTAL_RDY     0x02    // XTAL ready
#define ISR_POR_EN       0x01    // Power on reset enable
// Mask for all valid
#define ISR_MASK_ALL     0xFFFA  // remove por and low battery

// Enable disable ISRs
void rf_enable_isr(uint16_t mask);
void rf_disable_isr(uint16_t mask);

typedef enum {
  ENCODE_KEELOQ_PCM,  // keeloq pulse coded modulation
  ENCODE_MAX
} data_encode_t;

// Data structure containing packet format
typedef struct {
  uint16_t bit_time_us;   // elementary bit time in us
  uint8_t preamble_hi_bc; // bit count of preamble high
  uint8_t preamble_lo_bc; // bit count of preamble low
  uint8_t preamble_len;   // Min length of preamble
  uint8_t header_hi_bc;   // bit count of header hi
  uint8_t header_lo_bc;   // bit count of header low
  data_encode_t encoding; // data encoding
  uint8_t data_bc;        // count of payload bits
} packet_format_t;

#if 0
packet_format_t keeloq_format = {
    400, // 400us bit time
    1,   // preamble hi = 1 bit
    2,   // preamble lo = 2 bit
    6,   // 6 preamble bits
    10,  // header hi = 4ms = 10 bits
    10,  // header lo = 4ms = 10 bits
    ENCODE_KEELOQ_PCM, // pulse coded modulation
    66   // 66 bits = 28(serial) + 32(ci) + 4(fn) + 2(chksum)
};
#endif

/** Not to self. Use preamble detect to scan channels and
 * quickly determine if the channel is being used. */

#endif /* _SI4432_H_ */
