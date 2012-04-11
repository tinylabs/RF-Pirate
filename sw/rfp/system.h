#ifndef _SYSTEM_H_
#define _SYSTEM_H_
/**
 * System definitions
 * Elliot Buller 2012
 **/

// Keycodes
#define KEY_UP                 0x80
#define KEY_DOWN               0x81
#define KEY_LEFT               0x82
#define KEY_RIGHT              0x83
#define KEY_SELECT             0x84

// System events
#define EVENT_KEYPRESS         0x10
#define EVENT_VBATT            0x11
#define EVENT_SD_DET           0x12
#define EVENT_SD_RMV           0x13
#define EVENT_ICON_UPDT        0x14

// App events
#define EVENT_APP_START        0x20
#define EVENT_APP_STOP         0x21

// Serial events
#define EVENT_SERIAL_RECV      0x30

// RF IRQ events
#define EVENT_ISR_FIFO_UNDOVR  0x40
#define EVENT_ISR_FIFO_TXHI    0x41
#define EVENT_ISR_FIFO_TXLO    0x42
#define EVENT_ISR_FIFO_RXHI    0x43
#define EVENT_ISR_EXT_INTEN    0x44
#define EVENT_ISR_PKT_SENT     0x45
#define EVENT_ISR_PKT_RCVD     0x46
#define EVENT_ISR_CRC_ERR      0x47
#define EVENT_ISR_SYNC_DET     0x48
#define EVENT_ISR_VAL_PRM_DET  0x49
#define EVENT_ISR_INV_PRM_DET  0x4A
#define EVENT_ISR_RSSI         0x4B
#define EVENT_ISR_WAKE_TMR     0x4C
#define EVENT_ISR_LOW_BATT     0x4D
#define EVENT_ISR_XTAL_RDY     0x4E
#define EVENT_ISR_POR_EN       0x4F

#endif /* _EVENTS_H_ */
