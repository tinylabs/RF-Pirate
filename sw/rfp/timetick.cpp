/*
 * Timetick driver - Ticks once every 10 ms.
 * Callbacks are synchronous. Do not do any heavy lifting!! Instead post
 * an event to the event queue.
 *
 * Elliot Buller 2012
 */
#include <string.h>

#include "timetick.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// How many clients can register
#define CLIENT_CNT  5

// internal struct to track clients
struct client_t {
  uint16_t           ticks;
  timetick_cb_t      cb;
};

// Private variables
static uint16_t count;
static struct client_t client[CLIENT_CNT];

// Compare value for ~10ms
#define CMP_VAL  78

// Interrupt
ISR(TIMER0_COMPA_vect) {
  uint8_t i;

  for (i = 0; i < CLIENT_CNT; i++) {
    if (client[i].cb && (count % client[i].ticks == 0)) {
      client[i].cb(count);
    }
  }
  count++;
}

void timetick_init(void)
{
  count = 0;
  
  // Clear on clients
  memset (client, 0, sizeof(client));

  // Setup TIMER0
  TCCR0A = 2;             // cnt up to OCR0A
  TCCR0B = 5;             // div 1024
  OCR0A = CMP_VAL;        // ~10ms
  TIMSK0 = 2;             // interrupt on OCR0A match
  TCNT0 = 0;
  sei();
}

static struct client_t *find_client (timetick_cb_t cb)
{
  uint8_t i;

  for (i = 0; i < CLIENT_CNT; i++) {
    if (client[i].cb == cb)
      return &client[i];
  }
  return NULL;
}


void timetick_register(timetick_cb_t cb, uint16_t ticks)
{
  struct client_t *c = find_client(NULL);
  if (c == NULL)
    return;
  
  c->cb = cb;
  c->ticks = ticks;
}

void timetick_deregister(timetick_cb_t cb)
{
  struct client_t *c = find_client(cb);
  if (c == NULL)
    return;
  
  c->cb = NULL;
  c->ticks = 0;
}

uint16_t timetick_getcount(void)
{
  return count;
}
