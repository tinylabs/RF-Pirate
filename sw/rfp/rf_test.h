#ifndef _RFTEST_H_
#define _RFTEST_H_

#include "MenuEntry.h"

/* event handler */
uint8_t rf_event_notify (uint8_t event, uint16_t data);

/* Export rf_test menu entry */
EXPORT_MENU(m_rf_root);

#endif /* _RFTEST_H_ */
