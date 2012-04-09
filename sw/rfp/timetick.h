#ifndef _TIMETICK_H_
#define _TIMETICK_H_
/*
 * Timetick driver - Ticks once every 10 ms.
 * Callbacks are synchronous. Do not do any heavy lifting!! Instead post
 * an event to the event queue.
 *
 * Elliot Buller 2012
 */
#include <stdint.h>
#include <stdlib.h>

// Callback method type
typedef void (*timetick_cb_t)(uint16_t ticks);

// Timetick public methods
void timetick_init(void);
void timetick_register(timetick_cb_t cb, uint16_t ticks);
void timetick_deregister(timetick_cb_t cb);
uint16_t timetick_getcount(void);

#endif /* _TIMETICK_H_ */
