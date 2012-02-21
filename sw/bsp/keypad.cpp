#include <avr/io.h>
#include <avr/interrupt.h>

#include "hw.h"
#include "system.h"
#include "keypad.h"
#include "evt_handler.h"

// Thresholds for keycodes
#define KEY_DOWN_TH   20
#define KEY_RIGHT_TH  140
#define KEY_SELECT_TH 20
#define KEY_UP_TH     140
#define KEY_LEFT_TH   240

// states
#define READ_ADC5     1
#define READ_ADC6     2
#define READ_VBATT    3

// private variables
static uint8_t state;
static uint8_t last_keycode = 0;

// Forward declarations
static void vbatt_read_cb(uint16_t ticks);

// ADC complete ISR
ISR(ADC_vect)
{
  uint8_t val, keycode = 0;

  // Clear interrupt enable bit
  ADCSRA |= (1<<ADIE);

  // Read ADC val
  val = ADCH;
  
  if (state == READ_ADC5) {
    if (val < KEY_DOWN_TH)
      keycode = KEY_DOWN;
    else if (val < KEY_RIGHT_TH)
      keycode = KEY_RIGHT;

    // Switch states
    state = READ_ADC6;
  }
  else if (state == READ_ADC6) {
    if (val < KEY_SELECT_TH)
      keycode = KEY_SELECT;
    else if (val < KEY_UP_TH)
      keycode = KEY_UP;
    else if (val < KEY_LEFT_TH)
      keycode = KEY_LEFT;

    // Switch states
    state = READ_ADC5;
  }
  else if (state == READ_VBATT) {
    if (val != 0xff) {
      // Produce event with battery voltage
      evt_handler_event(EVENT_VBATT, val);
      state = READ_ADC5;
    }
    // Try again
    else {
      vbatt_read_cb(0);
    }
  }

  // Produce keycodes to EventHandler
  if (keycode != 0)
    evt_handler_event(EVENT_KEYPRESS, (uint16_t)keycode);

  last_keycode = keycode;
}

uint8_t keypad_lastkeycode(void)
{
  return last_keycode;
}

static void keypad_timetick_cb(uint16_t ticks)
{
  // Select ADC
  ADMUX &= ~0x1f;
  ADMUX |= (state == READ_ADC5) ? 5 : 6;

  // Set interrupt conversion bit
  ADCSRA |= (1<<ADIE);

  // start conversion
  ADCSRA |= 0x40;
}

static void vbatt_read_cb(uint16_t ticks)
{
  // Select ADC 7
  ADMUX &= ~0x1f;
  ADMUX |= 7;

  // Set interrupt conversion bit
  ADCSRA |= (1<<ADIE);

  // start conversion
  ADCSRA |= 0x40;

  // Set state
  state = READ_VBATT;
}

void keypad_init(uint16_t ticks)
{
  state = READ_ADC5;

  // Initialize ADCs
  // 8-bit left just. ext ref
  ADMUX = 0x60;
  // Turn on ADC unit
  // Setup prescalar = 64 (8MHz / 64 = 125kHz)
  ADCSRA = 0x86;
  // Low speed, no trigger src
  ADCSRB = 0;
  // Disable digital inputs on ADC{5,6,7}
  DIDR0 = 0xE0;

  // Register with Timetick
  timetick_register(&keypad_timetick_cb, ticks);

  // Do initial battery read and register with timetick
  // Hacked this in here because this is where we define ADC
  // isr. This really probably belongs somewhere else.
  // Maybe make ADC a service?
  vbatt_read_cb(0);
  timetick_register(&vbatt_read_cb, 6000);  // Once a minute
}
