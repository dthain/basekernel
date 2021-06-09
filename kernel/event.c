
#include "event.h"
#include "device.h"
#include "keyboard.h"
#include "mouse.h"
#include "interrupt.h"
#include "string.h"
#include "process.h"

#define EVENT_BUFFER_SIZE 32

static struct event buffer[EVENT_BUFFER_SIZE];

static int head=0;
static int tail=0;
static int overflow_count=0;

static struct list queue = LIST_INIT;

/* INTERRUPT CONTEXT */

void event_post( uint16_t type, uint16_t code, int16_t x, int16_t y )
{
	/* If ring buffer is full, return immediately. */
	int next = (head+1) % EVENT_BUFFER_SIZE;
	if(next==tail) {
		overflow_count++;
		return;
	}

	/* Copy event to current buffer position */
	struct event *e = &buffer[head];
	e->type = type;
	e->code = code;
	e->x = x;
	e->y = y;

	/* Advance head pointer and wake up waiting process (if any) */
	head = next;
	process_wakeup(&queue);
}

int event_read_raw( struct event *e, int size, int blocking )
{
	int total=0;

	if(size<sizeof(struct event)) return KERROR_INVALID_REQUEST;

	interrupt_block();

	while(size>=sizeof(struct event)) {

		if(head==tail) {
			if(blocking && total==0) {
				process_wait(&queue);
				continue;
			} else {
				break;
			}
		}

		*e = buffer[tail];
		tail = (tail+1) % EVENT_BUFFER_SIZE;
		total++;
		size -= sizeof(struct event);
	}

	interrupt_unblock();

	return total;
}

int event_read( struct event *e, int size )
{
	return event_read_raw(e,size,1);
}

int event_read_nonblock( struct event *e, int size )
{
	return event_read_raw(e,size,0);
}

int event_read_keyboard()
{
	struct event e;
	while(event_read(&e,sizeof(e))) {
		if(e.type==EVENT_KEY_DOWN) {
			return e.code;
		}
	}
	return 'x';
}

void event_init()
{
	mouse_init();
	keyboard_init();
	printf("event: ready\n");
}





