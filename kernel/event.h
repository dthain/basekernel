#ifndef EVENT_H
#define EVENT_H

#include <kernel/types.h>

struct event {
	uint16_t type;
	uint16_t code;
	int16_t  x;
	int16_t  y;
};

#define EVENT_MOUSE_MOVE     2
#define EVENT_BUTTON_UP      4
#define EVENT_BUTTON_DOWN    8
#define EVENT_KEY_UP         16
#define EVENT_KEY_DOWN       32
#define EVENT_RESIZE         64
#define EVENT_REVEAL         128
#define EVENT_HIDE           256

void event_post( uint16_t type, uint16_t code, int16_t x, int16_t y );
int  event_read( struct event *e, int timeout );

#endif
