#include <stdio.h>
#include "MenuEntry.h"

/* scratch buffer to assemble dropdown text, only one instance!! */
static char buf[20];

// Constructor for each menu types
MenuEntry::MenuEntry (const char *text)
{
  this->flag.type = TYPE_DUMMY;
  this->text = text;
}

MenuEntry::MenuEntry (const char *text, MenuEntry *next)
{
  this->flag.type = TYPE_BRANCH;
  this->text = text;
  this->d.next = next;
}

MenuEntry::MenuEntry (const char *text, exec_fn exec)
{
  this->flag.type = TYPE_EXEC;
  this->text = text;
  this->d.exec = exec;
}

MenuEntry::MenuEntry (const char *text, MenuEntry *next, event_notify_cb cb)
{
  this->flag.type = TYPE_EVT_HDL;
  this->text = text;
  this->d.next = next;
  this->evt_handler = cb;
}

MenuEntry::MenuEntry (const char *text, dropdown_t *d, uint8_t type)
{
  this->flag.type = TYPE_DROPDOWN;
  this->flag.drop_type = type;
  this->text = text;
  this->d.drop = d;
}

void MenuEntry::initialize ()
{
  // Return if not dropdown type
  if (this->flag.type != TYPE_DROPDOWN)
    return;

  // Initialize based on type
  switch (this->flag.drop_type) {
  case TYPE_DROP_ENUM:
  {
    uint8_t *idx = &this->d.drop->idx;
    
    // if data is out of bounds init to smallest value
    if (*idx > this->d.drop->max) 
    *idx = this->d.drop->min;
  }
  break;
    
  case TYPE_DROP_NUM:
  {
    uint16_t *data = (uint16_t *)this->d.drop->data;
    
    // if data is out of bounds init to smallest value
    if ((*data < this->d.drop->min) ||
	(*data > this->d.drop->max)) 
    *data = this->d.drop->min;
  }
  break;
    case TYPE_DROP_ALPH:
      break;

    case TYPE_DROP_8_3:
      break;

    default:
      break;
  }
}

void MenuEntry::increment ()
{
  switch (this->flag.drop_type) {
    case TYPE_DROP_ENUM:
    {
      uint8_t *idx = &this->d.drop->idx;
    
      // if data is out of bounds init to smallest value
      if (*idx >= this->d.drop->max)
	return;
      (*idx)++;
    }
    break;

    case TYPE_DROP_NUM:
    {
      uint16_t *data = (uint16_t *)this->d.drop->data;
      
      if (*data >= this->d.drop->max)
	return;
      (*data)++;
    }
    break;

    case TYPE_DROP_ALPH:
    break;

    case TYPE_DROP_8_3:
    break;
  }

}
void MenuEntry::decrement ()
{
  switch (this->flag.drop_type) {
    case TYPE_DROP_ENUM:
    {
      uint8_t *idx = &this->d.drop->idx;
    
      // catch underflow
      if (*idx == 0)
	return;
      (*idx)--;
    }
    break;

    case TYPE_DROP_NUM:
    {
      uint16_t *data = (uint16_t *)this->d.drop->data;
      
      /* Catch underflow also */
      if (*data <= this->d.drop->min || *data >= this->d.drop->max)
	return;
      (*data)--;
    }
    break;

    case TYPE_DROP_ALPH:
    break;

    case TYPE_DROP_8_3:
    break;
  }

}
char *MenuEntry::get_text ()
{
  switch (this->flag.drop_type) {
    case TYPE_DROP_ENUM:
      sprintf(buf, "%s", ((char **)this->d.drop->data)[this->d.drop->idx]);
    break;

    case TYPE_DROP_NUM:
    {
      uint16_t *data = (uint16_t *)this->d.drop->data;
      sprintf(buf, "%u", *data);
    }
    break;

    case TYPE_DROP_ALPH:
    break;

    case TYPE_DROP_8_3:
    break;
  }
  
  return buf;
}
