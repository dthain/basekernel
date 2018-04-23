/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "serial.h"
#include "graphics.h"
#include "device.h"
#include "kmalloc.h"

struct console_device{
    struct device device;
    int xsize;
    int ysize;
    int xpos;
    int ypos;
    struct graphics *gx;
};


struct graphics_color bgcolor = {0,0,0};
struct graphics_color fgcolor = {255,0,0};
struct console_device console = {0};

static void console_reset(struct console_device *d)
{
	d->xpos = d->ypos = 0;
	d->xsize = graphics_width(d->gx)/8;
	d->ysize = graphics_height(d->gx)/8;
	graphics_fgcolor(d->gx,fgcolor);
	graphics_bgcolor(d->gx,bgcolor);
	graphics_clear(d->gx,0,0,graphics_width(d->gx),graphics_height(d->gx));
}

void console_heartbeat()
{
	static int onoff=0;
	if(onoff) {
		graphics_char(console.gx,console.xpos*8,console.ypos*8,' ');
	} else {
		graphics_char(console.gx,console.xpos*8,console.ypos*8,'_');
	}
	onoff = !onoff;
}

int console_device_write( struct device *device, void *buffer, int size, int offset )
{
	struct console_device *d = (struct console_device*)device;
	graphics_char(d->gx,d->xpos*8,d->ypos*8,' ');

	int i;
	for (i = 0; i < size; i++) {
		char c = ((char*)buffer)[i];
#ifdef TEST
			serial_write(0, c);
#endif
		switch(c) {
			case 13:
			case 10:
				d->xpos=0;
				d->ypos++;
				break;
			case '\f':
				d->xpos = d->ypos = 0;
				d->xsize = graphics_width(d->gx)/8;
				d->ysize = graphics_height(d->gx)/8;
				graphics_fgcolor(d->gx,fgcolor);
				graphics_bgcolor(d->gx,bgcolor);
				graphics_clear(d->gx,0,0,graphics_width(d->gx),graphics_height(d->gx));
				break;
			case '\b':
				d->xpos--;
				break;
			default:
				graphics_char(d->gx,d->xpos*8,d->ypos*8,c);
				d->xpos++;
				break;
		}

		if(d->xpos<0) {
			d->xpos=d->xsize-1;
			d->ypos--;
		}

		if(d->xpos>=d->xsize) {
			d->xpos=0;
			d->ypos++;
		}

		if(d->ypos>=d->ysize) {
			d->xpos = d->ypos = 0;
			d->xsize = graphics_width(d->gx)/8;
			d->ysize = graphics_height(d->gx)/8;
			graphics_fgcolor(d->gx,fgcolor);
			graphics_bgcolor(d->gx,bgcolor);
			graphics_clear(d->gx,0,0,graphics_width(d->gx),graphics_height(d->gx));
		}

	}
	graphics_char(d->gx,d->xpos*8,d->ypos*8,'_');
	return i;
}

void console_putchar( char c )
{
	device_write((struct device*)&console, &c, 1, 0);
}

void console_putstring( const char *s )
{
	while(*s) {
		console_putchar(*s);
		s++;
	}
}

int console_write( int unit, const void *buffer, int length, int offset )
{
	device_write((struct device*)&console, (void*)buffer, length, offset);
	return 1;
}

struct device * console_get()
{
    return (struct device*)&console;
}

struct device * console_init( struct graphics *g )
{
	console.gx = g;
    console.device.block_size = 1;
	console.device.write = console_device_write;
	console_reset(&console);
	console.device.buffer = 0;
	console_putstring("\nconsole: initialized\n");
    return (struct device*)&console;
}

int console_device_write_debug( struct device *device, void *buffer, int size, int offset )
{
    return console_device_write(device, buffer, size, offset);
}
struct device * console_create( struct graphics *g )
{
    struct console_device *c = kmalloc(sizeof(*c));
	c->gx = g;
    c->device.block_size = 1;
    c->device.buffer = 0;
	c->device.write = console_device_write_debug;
	console_reset(c);
    return (struct device*)c;
}

void printf_putchar( char c )
{
	console_write(0,&c,1,0);
}

void printf_putstring( char *s )
{
	console_write(0,s,strlen(s),0);
}

