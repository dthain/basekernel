/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "serial.h"
#include "graphics.h"
#include "kmalloc.h"
#include "string.h"

struct console {
	struct graphics *gx;
	int xsize;
	int ysize;
	int xpos;
	int ypos;
	int onoff;
	int refcount;
};

struct console console_root = {0};

static struct graphics_color bgcolor = { 0, 0, 0 };
static struct graphics_color fgcolor = { 255, 255, 255 };

static void console_reset( struct console *d )
{
	if(!d || !d->gx) return;
	d->xpos = d->ypos = 0;
	d->xsize = graphics_width(d->gx) / 8;
	d->ysize = graphics_height(d->gx) / 8;
	d->onoff = 0;
	graphics_fgcolor(d->gx, fgcolor);
	graphics_bgcolor(d->gx, bgcolor);
	graphics_clear(d->gx, 0, 0, graphics_width(d->gx), graphics_height(d->gx));
}

void console_heartbeat( struct console *d )
{
	char c = d->onoff ? ' ' : '_';
	graphics_char(d->gx, d->xpos * 8, d->ypos * 8, c );
	d->onoff = !d->onoff;
}

int console_write( struct console *d, const char *data, int size )
{
	graphics_char(d->gx, d->xpos * 8, d->ypos * 8, ' ');

	int i;
	for(i = 0; i < size; i++) {
		char c = data[i];
		switch (c) {
		case 13:
		case 10:
			d->xpos = 0;
			d->ypos++;
			break;
		case '\f':
			d->xpos = d->ypos = 0;
			d->xsize = graphics_width(d->gx) / 8;
			d->ysize = graphics_height(d->gx) / 8;
			graphics_fgcolor(d->gx, fgcolor);
			graphics_bgcolor(d->gx, bgcolor);
			graphics_clear(d->gx, 0, 0, graphics_width(d->gx), graphics_height(d->gx));
			break;
		case '\b':
			d->xpos--;
			break;
		default:
			graphics_char(d->gx, d->xpos * 8, d->ypos * 8, c);
			d->xpos++;
			break;
		}

		if(d->xpos < 0) {
			d->xpos = d->xsize - 1;
			d->ypos--;
		}

		if(d->xpos >= d->xsize) {
			d->xpos = 0;
			d->ypos++;
		}

		if(d->ypos >= d->ysize) {
			d->xpos = d->ypos = 0;
			d->xsize = graphics_width(d->gx) / 8;
			d->ysize = graphics_height(d->gx) / 8;
			graphics_fgcolor(d->gx, fgcolor);
			graphics_bgcolor(d->gx, bgcolor);
			graphics_clear(d->gx, 0, 0, graphics_width(d->gx), graphics_height(d->gx));
		}

	}
	graphics_char(d->gx, d->xpos * 8, d->ypos * 8, '_');
	return i;
}

void console_putchar( struct console *c, char ch )
{
	console_write(c,&ch,1);
}

void console_putstring( struct console *c, const char *str)
{
	console_write(c,str,strlen(str));
}

struct console *console_create(struct graphics *g)
{
	struct console *c = kmalloc(sizeof(*c));
	c->gx = graphics_addref(g);
	c->refcount = 1;
	console_reset(c);
	return c;
}

struct console *console_addref( struct console *c )
{
	c->refcount++;
	return c;
}

void console_delete( struct console *c )
{
	c->refcount--;
	if(c->refcount==0) {
		graphics_delete(c->gx);
		if (c != &console_root) kfree(c);
	}
}

void console_size( struct console *c, int *xsize, int *ysize )
{
	*xsize = c->xsize;
	*ysize = c->ysize;
}

struct console * console_init(struct graphics *g)
{
	console_root.gx = g;
	console_reset(&console_root);
	console_putstring(&console_root,"\nconsole: initialized\n");
	return &console_root;
}
