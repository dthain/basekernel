
#include "window.h"
#include "graphics.h"
#include "kmalloc.h"
#include "string.h"

struct window {
	struct window *parent;
	struct graphics *graphics;
	struct event_queue *queue;
	int refcount;
};

struct window window_root = {0};

struct window * window_create_root()
{
	struct window *w = &window_root;
	w->parent = w;
	w->graphics = graphics_create_root();
	w->queue = event_queue_create_root();
	w->refcount = 1;
	return w;
}

struct window * window_create( struct window *parent, int x, int y, int width, int height )
{
	struct window *w = kmalloc(sizeof(*w));
	w->parent = parent;
	w->graphics = graphics_create(parent->graphics);
	graphics_clip(w->graphics,x,y,width,height);
	w->queue = event_queue_create();
	w->refcount = 1;
	w->parent->refcount++;
	return w;
}

struct window * window_addref( struct window *w )
{
	w->refcount++;
	return w;
}

void window_delete( struct window *w )
{
	if(!w) return;

	if(w==&window_root) return;

	w->refcount--;
	if(w->refcount==0) {
		graphics_delete(w->graphics);
		event_queue_delete(w->queue);
		window_delete(w->parent);
		kfree(w);
	}
}

int  window_width( struct window *w )
{
	return graphics_width(w->graphics);
}

int  window_height( struct window *w )
{
	return graphics_height(w->graphics);
}

struct graphics * window_graphics( struct window *w )
{
	return w->graphics;
}

int window_post_events( struct window *w, struct event *e, int size )
{
	int total = 0;

	while(size>=sizeof(struct event)) {
		event_queue_post(w->queue,e);
		size -= sizeof(*e);
		e++;
		total += sizeof(*e);
	}

	return total;
}

int  window_read_events( struct window *w, struct event *e, int size )
{
	return event_queue_read(w->queue,e,size);
}

int  window_read_events_nonblock( struct window *w, struct event *e, int size )
{
	return event_queue_read_nonblock(w->queue,e,size);
}

int  window_write_graphics( struct window *w, int *cmd, int size )
{
	return graphics_write(w->graphics,cmd,size);	
}


