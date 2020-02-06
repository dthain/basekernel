#include "event.h"
#include "interrupt.h"

#define EVENT_BUFFER_SIZE 32

static struct event buffer[EVENT_BUFFER_SIZE];

static int head=0;
static int tail=0;

/* INTERRUPT CONTEXT */

void event_post( uint16_t type, uint16_t code, int16_t x, int16_t y )
{
	if(((head+1)%EVENT_BUFFER_SIZE)==tail) return;
	head = (head+1) % EVENT_BUFFER_SIZE;
	struct event *e = &buffer[head];
	e->type = type;
	e->code = code;
	e->x = x;
	e->y = y;
	return;
}

int event_read( struct event *e, int timeout )
{
	interrupt_block();

	if(head==tail) {
		interrupt_unblock();
		return 0;
	}

	*e = buffer[tail];
	tail = (tail+1) % EVENT_BUFFER_SIZE;

	interrupt_unblock();
	return 1;
}




