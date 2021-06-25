#ifndef NANOWIN_H
#define NANOWIN_H

#include <kernel/events.h>
#include <kernel/gfxstream.h>

struct nwindow * nw_create_default();
struct nwindow * nw_create_child( struct nwindow *parent, int x, int y, int width, int height );
struct nwindow * nw_create_from_fd( int fd );

int nw_width( struct nwindow *w );
int nw_height( struct nwindow *w );

char nw_getchar( struct nwindow *w, int blocking );

int nw_next_event( struct nwindow *w, struct event *e );
int nw_read_events( struct nwindow *w, struct event *e, int count, int timeout );
int nw_post_events( struct nwindow *w, const struct event *e, int count );

int nw_move( struct nwindow *w, int x, int y );
int nw_resize( struct nwindow *w, int width, int height );
int nw_fd( struct nwindow *w );

void nw_fgcolor( struct nwindow *w, int r, int g, int b );
void nw_bgcolor( struct nwindow *w, int r, int g, int b );
void nw_clear  ( struct nwindow *w, int x, int y, int width, int height );
void nw_line   ( struct nwindow *w, int x, int y, int width, int height );
void nw_rect   ( struct nwindow *w, int x, int y, int width, int height );
void nw_char   ( struct nwindow *w, int x, int y, char c );
void nw_string ( struct nwindow *w, int x, int y, const char *s );
void nw_flush  ( struct nwindow *w );



#endif
