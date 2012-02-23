#ifndef _RFDEBUG_H_
#define _RFDEBUG_H_

#include "MenuEntry.h"

/* event handler */
uint8_t rf_debug_notify (uint8_t event, uint16_t data);

/* Export rf_test menu entry */
EXPORT_MENU(m_rf_debug);

#endif /* _RFDEBUG_H_ */
