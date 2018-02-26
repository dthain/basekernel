/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "graphics.h"
#include "device.h"
#include "kmalloc.h"

struct console_state {
    int xsize;
    int ysize;
    int xpos;
    int ypos;
    struct graphics *gx;
};

struct graphics_color bgcolor = {0,0,0};
struct graphics_color fgcolor = {255,0,0};

static struct device console = {0};

static void console_reset()
{
    struct console_state *cs = console.data;
    cs->xpos = cs->ypos = 0;
	cs->xsize = graphics_width(cs->gx)/8;
	cs->ysize = graphics_height(cs->gx)/8;
	graphics_fgcolor(cs->gx,fgcolor);
	graphics_bgcolor(cs->gx,bgcolor);
	graphics_clear(cs->gx,0,0,graphics_width(cs->gx),graphics_height(cs->gx));
}

void console_heartbeat()
{
	static int onoff=0;
    struct console_state *cs = console.data;
	if(onoff) {
		graphics_char(cs->gx,cs->xpos*8,cs->ypos*8,' ');
	} else {
		graphics_char(cs->gx,cs->xpos*8,cs->ypos*8,'_');
	}
	onoff = !onoff;
}

int console_device_write( struct device *d, void *buffer, int size )
{
    struct console_state *cs = d->data;
	graphics_char(cs->gx,cs->xpos*8,cs->ypos*8,' ');

    int i;
    for (i = 0; i < size; i++) {
        char c = ((char*)buffer)[i];
        switch(c) {
            case 13:
            case 10:
                cs->xpos=0;
                cs->ypos++;
                break;
            case '\f':
                cs->xpos = cs->ypos = 0;
                cs->xsize = graphics_width(cs->gx)/8;
                cs->ysize = graphics_height(cs->gx)/8;
                graphics_fgcolor(cs->gx,fgcolor);
                graphics_bgcolor(cs->gx,bgcolor);
                graphics_clear(cs->gx,0,0,graphics_width(cs->gx),graphics_height(cs->gx));
                break;
            case '\b':
                cs->xpos--;
                break;
            default:
                graphics_char(cs->gx,cs->xpos*8,cs->ypos*8,c);
                cs->xpos++;
                break;
        }

        if(cs->xpos<0) {
            cs->xpos=cs->xsize-1;
            cs->ypos--;
        }

        if(cs->xpos>=cs->xsize) {
            cs->xpos=0;
            cs->ypos++;
        }

        if(cs->ypos>=cs->ysize) {
            cs->xpos = cs->ypos = 0;
            cs->xsize = graphics_width(cs->gx)/8;
            cs->ysize = graphics_height(cs->gx)/8;
            graphics_fgcolor(cs->gx,fgcolor);
            graphics_bgcolor(cs->gx,bgcolor);
            graphics_clear(cs->gx,0,0,graphics_width(cs->gx),graphics_height(cs->gx));
        }

        graphics_char(cs->gx,cs->xpos*8,cs->ypos*8,'_');
    }
    return i;
}

void console_putchar( char c )
{
    device_write(&console, &c, 1);
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
    device_write(&console, (void*)buffer, length);
	return 1;
}

void console_init( struct graphics *g )
{
    console.write = console_device_write;
    console.data = kmalloc(sizeof(struct console_state));
    struct console_state *cs = console.data;
	cs->gx = g;
    console_reset();
	console_putstring("\nconsole: initialized\n");
}

void printf_putchar( char c )
{
	console_write(0,&c,1,0);
}

void printf_putstring( char *s )
{
	console_write(0,s,strlen(s),0);
}

