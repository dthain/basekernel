#include "event_queue.h"
#include "interrupt.h"
#include "process.h"
#include "list.h"
#include "kmalloc.h"

#define EVENT_BUFFER_SIZE 32

struct event_queue {
	struct event buffer[EVENT_BUFFER_SIZE];
	struct list process_queue;
	int head;
	int tail;
	int overflow_count;
};

struct event_queue event_queue_root;

struct event_queue * event_queue_create_root()
{
	memset(&event_queue_root,0,sizeof(event_queue_root));
	return &event_queue_root;
}

struct event_queue * event_queue_create()
{
	struct event_queue *q = kmalloc(sizeof(*q));
	memset(q,0,sizeof(*q));
	return q;
}

void event_queue_delete( struct event_queue *q )
{
	kfree(q);
}

/* INTERRUPT CONTEXT */

void event_queue_post( struct event_queue *q, struct event *e )
{
	/* If ring buffer is full, return immediately. */
	int next = (q->head+1) % EVENT_BUFFER_SIZE;
	if(next==q->tail) {
		q->overflow_count++;
		return;
	}

	/* Copy event to current buffer position */
	q->buffer[q->head] = *e;

	/* Advance head pointer and wake up waiting process (if any) */
	q->head = next;
	process_wakeup(&q->process_queue);
}

/* INTERRUPT CONTEXT */

void event_queue_post_root( uint16_t type, uint16_t code, int16_t x, int16_t y )
{
	struct event e;
	e.type = type;
	e.code = code;
	e.x = x;
	e.y = y;
	event_queue_post(&event_queue_root,&e);
}

static int event_queue_read_raw( struct event_queue *q, struct event *e, int size, int blocking )
{
	int total=0;

	if(size<sizeof(struct event)) return KERROR_INVALID_REQUEST;

	interrupt_block();

	while(size>=sizeof(struct event)) {

		if(q->head==q->tail) {
			if(blocking && total==0) {
				process_wait(&q->process_queue);
				continue;
			} else {
				break;
			}
		}

		*e = q->buffer[q->tail];
		q->tail = (q->tail+1) % EVENT_BUFFER_SIZE;
		e++;
		total++;
		size -= sizeof(struct event);
	}

	interrupt_unblock();

	return total;
}

int event_queue_read( struct event_queue *q, struct event *e, int size )
{
	return event_queue_read_raw(q,e,size,1);
}

int event_queue_read_nonblock( struct event_queue *q, struct event *e, int size )
{
	return event_queue_read_raw(q,e,size,0);
}

