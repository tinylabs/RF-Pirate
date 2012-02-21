#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "ssd1306.h"
#include "Menu.h"

// System context obj for apps
typedef struct {
  // Handle to display
  ssd1306 *disp;
  // Handle to menu
  Menu *menu;
  // Handle to SD card
} context_t;


#endif /* _CONTEXT_H_ */
