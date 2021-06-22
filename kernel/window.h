#ifndef WINDOW_H
#define WINDOW_H

#include "event_queue.h"
#include "graphics.h"

extern struct window window_root;

struct window * window_create_root();

struct window * window_create( struct window *parent, int x, int y, int w, int h );
struct window * window_addref( struct window *w );
void window_delete( struct window *w );

int  window_width( struct window *w );
int  window_height( struct window *w );

struct graphics * window_graphics( struct window *w );
int  window_post_events( struct window *w, struct event *e, int size );
int  window_read_events( struct window *w, struct event *e, int size );
int  window_read_events_nonblock( struct window *w, struct event *e, int size );
int  window_write_graphics( struct window *w, int *cmd, int size );

void window_event_post_root( uint16_t type, uint16_t code, int16_t x, int16_t y );

#endif
