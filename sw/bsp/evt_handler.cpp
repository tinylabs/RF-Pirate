#include "evt_handler.h"
#include <stdlib.h>

#define MAX_HANDLERS  3

/* Create a singly linked list to track event handlers */
struct handler_node {
  event_notify_cb    cb;
  struct handler_node *next;
};

// Static pool of nodes, no dynamic alloc here
static struct handler_node pool[MAX_HANDLERS];

// Head of linked list of handlers
static struct handler_node *head;

// Queue for events
struct event_t {
  uint8_t event;
  uint16_t data;
};

// Buffer size for system events
#define EVENT_Q_SIZE  5
struct event_t event_q[EVENT_Q_SIZE];
uint8_t q_head = 0;
uint8_t q_tail = 0;

#define NEXT(x) ((x==EVENT_Q_SIZE-1) ? 0: x+1)

// Method to grab a new node from the pool
static struct handler_node *get_node (void)
{
  uint8_t i;

  for (i = 0; i < MAX_HANDLERS; i++) {
    if (pool[i].cb == NULL)
      return &pool[i];
  }
  return NULL;
}

void evt_handler_event(uint8_t event, uint16_t data)
{
  if (NEXT(q_head) == q_tail)
    return;

  //debug("Evt=%02x", event);
  // Store event
  event_q[q_head].event = event;
  event_q[q_head].data = data;
  q_head = NEXT(q_head);
}

void evt_handler_init(void)
{
  uint8_t i;
  head = NULL;

  // Make sure all nodes are NULL
  for (i = 0; i < MAX_HANDLERS; i++) {
    pool[i].cb = NULL;
    pool[i].next = NULL;
  }
}

// Add handler to the list of handlers
void evt_handler_addhandler (event_notify_cb cb)
{
  if (cb == NULL)
    return;

  // Get node
  struct handler_node *n = get_node();
  if (n == NULL)
    return;
  n->cb = cb;

  // Push to the head of the list
  n->next = head;
  head = n;
}

void evt_handler_pophandler (void)
{
  // Pop the head off the list
  if (head != NULL)
    head->cb = NULL;
  else
    return;

  head = head->next;
}

void evt_handler_syncevent (uint8_t event, uint16_t data)
{
  struct handler_node *tmp;

  for (tmp = head; tmp != NULL; tmp = tmp->next) {
    if (tmp->cb(event, data))
      return;
  }
}

void evt_handler_dispatch (void)
{
  struct handler_node *tmp;

  // Process all events in q
  while (q_tail != q_head) {
    //debug("q_tail=[%u]", q_tail);
    //debug("q_head=[%u]", q_head);
    // Loop through all EventHandlers until event is handled
    for (tmp = head; tmp != NULL; tmp = tmp->next) {
      if (tmp->cb(event_q[q_tail].event, event_q[q_tail].data))
	break;
    }
    q_tail = NEXT(q_tail);
  }
}
