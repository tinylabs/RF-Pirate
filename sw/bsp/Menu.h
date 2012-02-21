#ifndef _MENU_H_
#define _MENU_H_

#include <stdint.h>
#include "MenuEntry.h"
#include "ssd1306.h"
#include "evt_handler.h"

#define UP_ICON       "\x01" // Stored in font map
#define DOWN_ICON     "\x02" // Stored in font map
#define DIR_ICON      "\x03" // Stored in font map

#define ICON_DRAW_BIT   0x80
#define ROOT_HEADER     "MENU"
#define MAX_MENU_DEPTH  10

struct menu_save_t {
  MenuEntry *menu;
  uint8_t   flags;
  uint8_t   num_entries;
  struct {
    uint8_t evt_hdl_set : 1;
    uint8_t selected    : 7;
  } flag;
};

// To be filled in by callback
struct icon_data_t {
  uint8_t index;
  uint8_t line;
  uint8_t x;
};


class Menu {
 public:
  Menu (MenuEntry *root, ssd1306 *disp, uint8_t lineStart, uint8_t lineEnd);

  // Update the display with current menu
  void DrawMenu(void);
  void KeyHandler(uint8_t k);
  void SetLineStartEnd(uint8_t line_start, uint8_t line_end);

 private:
  void DrawVisible(MenuEntry *top, uint8_t sel, bool above, bool below);

  // current menu
  menu_save_t *cur;

  // Handle to display
  ssd1306 *display;

  // Stack to track nested menus
  menu_save_t stack[MAX_MENU_DEPTH];
  uint8_t stack_idx;

  // Store start/end line for on screen menu
  uint8_t line_start;
  uint8_t line_end;
  uint8_t visible_lines;
};

#endif /* _MENU_H_ */
