
#include "kernel/gfxstream.h"
#include "kernel/types.h"
#include "kernel/events.h"
#include "library/nwindow.h"
#include "library/syscalls.h"

#include "library/string.h"
#include "library/malloc.h"
#include "library/stdio.h"

struct nwindow {
	int fd;
	int x, y;
	int width, height;
	struct {
		int *buffer;
		int length;
		int index;
	} graphics;
};

struct nwindow * nw_create_fd( int fd )
{
	struct nwindow *w = malloc(sizeof(*w));
	w->fd = fd;
	w->x = 0;
	w->y = 0;
	w->graphics.buffer = malloc(PAGE_SIZE);
	w->graphics.length = PAGE_SIZE;
	w->graphics.index = 0;

	int dims[2];
	syscall_object_size(fd,dims,2);
	w->width = dims[0];
	w->height = dims[1];

	return w;
}

struct nwindow * nw_create_default()
{
	return nw_create_fd(KNO_STDWIN);
}

struct nwindow * nw_create_child( struct nwindow *parent, int x, int y, int width, int height )
{
	int fd = syscall_open_window(parent->fd,x,y,width,height);
	struct nwindow *nw = nw_create_fd(fd);
	nw->x = x;
	nw->y = y;
	nw->width = width;
	nw->height = height;
	return nw;
}

int nw_width( struct nwindow *nw )
{
	return nw->width;
}

int nw_height( struct nwindow *nw )
{
	return nw->height;
}

int nw_next_event( struct nwindow *nw, struct event *e )
{
	int r = syscall_object_read(nw->fd,e,sizeof(*e),0);
	return r>0;
}

int nw_read_events( struct nwindow *nw, struct event *e, int count, int blocking )
{
	int r = syscall_object_read(nw->fd,e,sizeof(*e)*count,blocking==0 ? KERNEL_IO_NONBLOCK : 0);
	return r>0 ? r : 0;
}

int nw_post_events( struct nwindow *nw, const struct event *e, int count )
{
	return syscall_object_write(nw->fd,e,sizeof(*e)*count,KERNEL_IO_POST);
}


char nw_getchar( struct nwindow *nw, int blocking )
{
	struct event e;
	while(1) {
		int r = nw_read_events(nw,&e,sizeof(e),blocking);
		if(r<=0) return 0;
		if(e.type==EVENT_KEY_DOWN) {
			return e.code;
       		}
	}
}

int nw_fd( struct nwindow *w )
{
	return w->fd;
}

static void nw_draw3( struct nwindow *nw, int t, int a0, int a1, int a2 )
{
	if(nw->graphics.length-nw->graphics.index<4) {
		nw_flush(nw);
	}
	int *p = &nw->graphics.buffer[nw->graphics.index];
	*p++ = t;
	*p++ = a0;
	*p++ = a1;
	*p++ = a2;
	nw->graphics.index += 4;
}

static void nw_draw4( struct nwindow *nw, int t, int a0, int a1, int a2, int a3 )
{
	if(nw->graphics.length-nw->graphics.index<5) {
		nw_flush(nw);
	}
	int *p = &nw->graphics.buffer[nw->graphics.index];
	*p++ = t;
	*p++ = a0;
	*p++ = a1;
	*p++ = a2;
	*p++ = a3;
	nw->graphics.index += 5;
}

void nw_flush( struct nwindow *nw )
{
	syscall_object_write(nw->fd, nw->graphics.buffer, nw->graphics.index, 0);
	nw->graphics.index = 0;
}

void nw_fgcolor( struct nwindow *nw, int r, int g, int b)
{
	nw_draw3(nw,GRAPHICS_FGCOLOR, r, g, b);
}

void nw_bgcolor( struct nwindow *nw, int r, int g, int b)
{
	nw_draw3(nw,GRAPHICS_BGCOLOR, r, g, b);
}

void nw_rect( struct nwindow *nw, int x, int y, int w, int h)
{
	nw_draw4(nw,GRAPHICS_RECT, x, y, w, h);
}

void nw_clear( struct nwindow *nw, int x, int y, int w, int h)
{
	nw_draw4(nw,GRAPHICS_CLEAR, x, y, w, h);
}

void nw_line( struct nwindow *nw, int x, int y, int w, int h)
{
	nw_draw4(nw,GRAPHICS_LINE, x, y, w, h);
}

void nw_string( struct nwindow *nw, int x, int y, const char *s )
{
	int length = strlen(s);

	if(nw->graphics.length-nw->graphics.index < (length+4) ) {
		nw_flush(nw);
	}

	int *p = &nw->graphics.buffer[nw->graphics.index];
	*p++ = GRAPHICS_TEXT;
	*p++ = x;
	*p++ = y;
	*p++ = length;

	int i;
	for(i=0;i<length;i++) {
		*p++ = s[i];
	}

	nw->graphics.index += 4 + length;
}
