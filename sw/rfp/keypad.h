#ifndef _KEYPAD_H_
#define _KEYPAD_H_
/**
 * Keypad driver - Sample joystick and produce key events.
 *
 * Elliot Buller 2012
 **/
#include "timetick.h"

// Public functions
void keypad_init(uint16_t ticks);
uint8_t keypad_lastkeycode(void);

#endif /* _KEYPAD_H_ */
