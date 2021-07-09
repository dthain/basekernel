#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <kernel/events.h>

extern struct event_queue event_queue_root;

struct event_queue * event_queue_create_root();

struct event_queue * event_queue_create();
void event_queue_delete( struct event_queue *e);

void event_queue_post( struct event_queue *q, struct event *e );
int  event_queue_read( struct event_queue *q, struct event *e, int size );
int  event_queue_read_nonblock( struct event_queue *q, struct event *e, int size );

void event_queue_post_root( uint16_t type, uint16_t code, int16_t x, int16_t y );

#endif
