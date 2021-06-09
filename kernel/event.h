#ifndef EVENT_H
#define EVENT_H

#include <kernel/events.h>

void event_post( uint16_t type, uint16_t code, int16_t x, int16_t y );
int  event_read( struct event *e, int size );
int  event_read_nonblock( struct event *e, int size );
int event_read_keyboard();

void event_init();

#endif
