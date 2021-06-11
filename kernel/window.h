#ifndef WINDOW_H
#define WINDOW_H

#include "event.h"
#include "graphics.h"

struct window {
	struct window *parent;
	struct graphics *graphics;
	//struct event_queue *queue;
	int refcount;
};

extern struct window window_root;

struct window * window_create_root();

struct window * window_create( struct window *parent, int x, int y, int w, int h );
struct window * window_addref( struct window *w );
void window_delete( struct window *w );

int  window_width( struct window *w );
int  window_height( struct window *w );
void window_event_post( struct window *w, struct event *e );
int  window_read_events( struct window *w, struct event *e, int size );
int  window_write_graphics( struct window *w, struct graphics_command *command );

#endif
