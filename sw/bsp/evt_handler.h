#ifndef _EVT_HANDLER_H_
#define _EVT_HANDLER_H_
/**
 * Event Handler - Pass events to registered handlers until event is handled.
 * Last registered handler (system handler) will consume all remaining events.
 * Elliot Buller 2012
 **/
#include <stdint.h>
#include "system.h"

typedef uint8_t (*event_notify_cb) (uint8_t event, uint16_t data);

void evt_handler_init(void);
void evt_handler_addhandler (event_notify_cb cb);
void evt_handler_pophandler (void);
void evt_handler_event (uint8_t event, uint16_t data);
void evt_handler_syncevent (uint8_t event, uint16_t data);
void evt_handler_dispatch (void);

#endif /* _EVT_HANDLER_H_ */
