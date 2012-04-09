#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "Menu.h"
#include "evt_handler.h"
#include "context.h"

static context_t sys;

Menu::Menu(MenuEntry *root, ssd1306 *disp, uint8_t lineStart, uint8_t lineEnd)
{
  this->display = disp;
  this->line_start = lineStart;
  this->line_end = lineEnd;
  cur = &stack[0];
  cur->menu = root;
  cur->flag.selected = 0;
  cur->flag.evt_hdl_set = 0;
  cur->num_entries = 0;
  visible_lines = lineEnd - lineStart + 1;
  stack_idx = 0;

  sys.disp = disp;
  sys.menu = this;
}

void Menu::SetLineStartEnd(uint8_t line_start, uint8_t line_end)
{
  this->line_start = line_start;
  this->line_end = line_end;
  visible_lines = line_end - line_start + 1;
  // Draw screen
  DrawMenu();
}

void Menu::Enable (uint8_t enable)
{
  if (enable) {
    visible_lines = line_end - line_start + 1;
  }
  else {
    visible_lines = 0;
  }
}

void Menu::DrawVisible(MenuEntry *top, uint8_t selected, bool items_above,
		       bool items_below)
{
  uint8_t i;
  const char *header;

  // Draw header
  if (stack_idx == 0)
    header = ROOT_HEADER;

  // Else it is the parent entry
  else
    header = stack[stack_idx - 1].menu[stack[stack_idx - 1].flag.selected].text;

  // Draw header
  display->clearline(0);
  display->drawstring(0, 0, "[", 0);
  display->drawstring(0, 1, header, 0);
  display->drawstring(0, strlen(header) + 1, "]", 0);  
    
  // Display all visible lines
  for (i = 0; i < visible_lines; i++) {
    uint8_t x = 0;

    // Clear the line before we display it.
    display->clearline(i + line_start);
    
    // If we are out of elements continue
    if (i >= cur->num_entries)
      continue;

    // indent selected entry
    if (i == selected)
      display->drawstring(i + line_start, x++, " ", 1);
    
    // Check if it's a dropdown and selected
    if (top[i].flag.type == TYPE_DROPDOWN &&
	top[i].flag.selected == 1) {
      display->drawstring(i + line_start, x, top[i].get_text(), 1);
    }
    // Display normal entry
    else {
      display->drawstring(i + line_start, x, top[i].text, (selected == i));
    }

    // If its a directory draw icon
    if ((top[i].flag.type == TYPE_BRANCH) || (top[i].flag.type == TYPE_EVT_HDL)) {
      x += strlen(top[i].text);
      display->drawstring(i + line_start, x, DIR_ICON, 0);
    }
  }

  // Draw icons if necessary
  if (items_above)
    display->drawstring(line_start, 20, UP_ICON, 0);
  
  // Draw icons if necessary
  if (items_below)
    display->drawstring(line_end, 20, DOWN_ICON, 0);

#if 0
  // Check if any user icons need to be displayed
  for (i = 0; i < LCD_MAX_ICON; i++) {
    // Generate event for each valid icon
    if (display->validicon(i)) {
      icon_data_t icon;
      icon.index = i;
      // Cast is ok bc we know pointers are 16bit
      evt_handler_syncevent (EVENT_ICON_UPDT, (uint16_t)&icon);
      if (icon.index & ICON_DRAW_BIT)
	display->drawicon(i, icon.line, icon.x);
    }
  }
#endif
}


void Menu::DrawMenu (void)
{
  uint8_t top;
  bool items_below;

  // Calc the number of menu entries, first time this is called we won't know
  if (cur->num_entries == 0) {
    // Calculate everytime at the a time cost
    uint8_t cnt;
    for (cnt = 0; cur->menu[cnt].text != NULL; cnt++);
    cur->num_entries = cnt;
  }

  // Determine top  visible element in current menu
  top = (cur->flag.selected > (visible_lines - 1)) ? 
    cur->flag.selected - visible_lines + 1 : 0;

  // Calculate whether there are items below
  items_below = (cur->num_entries < visible_lines) ? false :
    (top + visible_lines < cur->num_entries);

  // Display these lines
  DrawVisible(&cur->menu[top], (cur->flag.selected - top), 
	      (top > 0), items_below);

  // Update display
  display->display();
}

void Menu::KeyHandler(uint8_t k)
{
  switch (k) {
    case KEY_UP:
      if (cur->menu[cur->flag.selected].flag.type == TYPE_DROPDOWN &&
	  cur->menu[cur->flag.selected].flag.selected == 1) {
	// scroll through menu entries
	cur->menu[cur->flag.selected].increment();
      }
      else {
	cur->flag.selected = (cur->flag.selected > 0) ? 
	  cur->flag.selected - 1 : 0;
      }
    break;

    case KEY_DOWN:
      if (cur->menu[cur->flag.selected].flag.type == TYPE_DROPDOWN &&
	  cur->menu[cur->flag.selected].flag.selected == 1) {
	// scroll through menu entries
	cur->menu[cur->flag.selected].decrement();	
      }
      else {
	cur->flag.selected = (cur->flag.selected >= cur->num_entries - 1) ?
	  cur->flag.selected : cur->flag.selected + 1;
      }
    break;

    case KEY_LEFT:
      // Are we deselecting a dropdown?
      if (cur->menu[cur->flag.selected].flag.type == TYPE_DROPDOWN &&
	  cur->menu[cur->flag.selected].flag.selected == 1) {
	cur->menu[cur->flag.selected].flag.selected = 0;
      }

      // Pop prev menu off stack
      else if (stack_idx > 0) {
	cur = &stack[--stack_idx];
	if (cur->flag.evt_hdl_set) {  
	  // Set event handler
	  cur->flag.evt_hdl_set = 0;
	  // Send stop event
	  evt_handler_syncevent(EVENT_APP_STOP, 0);
	  evt_handler_pophandler();
	}
      }
    break;

    case KEY_RIGHT:
      menu_save_t *stk;

      // Have we selected a dropdown option?
      if (cur->menu[cur->flag.selected].flag.type == TYPE_DROPDOWN &&
	  cur->menu[cur->flag.selected].flag.selected == 0) {
	cur->menu[cur->flag.selected].flag.selected = 1;
	
	// Initialize value
	cur->menu[cur->flag.selected].initialize();
      }

      // Go into submenu
      else if ((cur->menu[cur->flag.selected].flag.type == TYPE_BRANCH) ||
	       (cur->menu[cur->flag.selected].flag.type == TYPE_EVT_HDL)) {

	// Push event handler onto stack if exists
	if (cur->menu[cur->flag.selected].flag.type == TYPE_EVT_HDL) {
	  // Set event handler
	  cur->flag.evt_hdl_set = 1;
	  // Add event handler to list
	  evt_handler_addhandler(cur->menu[cur->flag.selected].evt_handler);
	  // Send start event
	  evt_handler_syncevent(EVENT_APP_START, (uint16_t)&sys);
	}

	// Save ptr to cur menu state
	stk = &stack[stack_idx++];
	stk->menu = cur->menu;
	stk->flag.selected = cur->flag.selected;

	// Point to new menu
	cur = &stack[stack_idx];
	cur->menu = stk->menu[stk->flag.selected].d.next;
	cur->flag.selected = 0;
	cur->num_entries = 0;	
      }
    break;

    case KEY_SELECT:
      if (cur->menu[cur->flag.selected].flag.type == TYPE_EXEC)
	cur->menu[cur->flag.selected].d.exec();	
      break;
  }

  // Update display
  DrawMenu();
}
