#ifndef _MENUENTRY_H_
#define _MENUENTRY_H_

#include <stdint.h>
#include "evt_handler.h"

typedef void (*exec_fn)(void);

/* Helper to export menus */
#define EXPORT_MENU(x) extern MenuEntry x[]

/* Types of menu entries */
#define TYPE_DUMMY     0
#define TYPE_BRANCH    1
#define TYPE_EXEC      2
#define TYPE_DROPDOWN  3
#define TYPE_EVT_HDL   4

/* Types of dropdown menus */
#define TYPE_DROP_ENUM 1  /* Enumeration */
#define TYPE_DROP_NUM  2  /* numeric */
#define TYPE_DROP_ALPH 3  /* alphanumeric (string) */
#define TYPE_DROP_8_3  4  /* 8.3 filename (string) */

/* Define dropdown menu type */
typedef struct  {
  void    *data;    /* Pointer to data */
  uint16_t min;     /* Min for numeric */
  uint16_t max;     /* Max for numeric */
  uint8_t  len;     /* Length for strings */
  uint8_t  idx;     /* index of string/enum */
} dropdown_t;

class MenuEntry { 
 private:

 public:
  /* Text displayed in menu */
  const char *text;

  /* Event handler */
  event_notify_cb evt_handler;
  
  /* Share data element to save on space */
  union data_t {
    MenuEntry       *next;
    exec_fn         exec;
    dropdown_t      *drop;
  } d;
  
  /* flags */
  struct flag_t {
    unsigned char type : 3;
    // Add dropdown flags here
    unsigned char drop_type : 3;
    unsigned char selected : 1;
  } flag;

  // Different menu types
  MenuEntry (const char *text);
  MenuEntry (const char *text, MenuEntry *next);
  MenuEntry (const char *text, exec_fn exec);
  MenuEntry (const char *text, dropdown_t *d, uint8_t type);
  MenuEntry (const char *text, MenuEntry *next, event_notify_cb cb);

  // Dropdown operations
  void initialize ();  // initialize dropdown on selection
  void increment ();   // increment dropdown
  void decrement ();   // decrement dropdown
  char *get_text ();   // get text to display
};


#endif /* _MENUENTRY_H_ */
