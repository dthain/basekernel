/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "graphics.h"

static int xsize=80;
static int ysize=25;

static int xpos=0;
static int ypos=0;

struct graphics_color bgcolor = {0,0,0};
struct graphics_color fgcolor = {255,0,0};

static struct graphics *gx = 0;

static void console_reset()
{
	xpos = ypos = 0;
	xsize = graphics_width(gx)/8;
	ysize = graphics_height(gx)/8;
	graphics_fgcolor(gx,fgcolor);
	graphics_bgcolor(gx,bgcolor);
	graphics_clear(gx,0,0,graphics_width(gx),graphics_height(gx));
}

static void console_writechar( int x, int y, char ch )
{
	graphics_char(gx,x*8,y*8,ch);
}

void console_heartbeat()
{
	static int onoff=0;
	if(onoff) {
		graphics_char(gx,xpos*8,ypos*8,'_');
	} else {
		graphics_char(gx,xpos*8,ypos*8,'_');
	}
	onoff = !onoff;
}

void console_putchar( char c )
{
	console_writechar(xpos,ypos,' ');

	switch(c) {
		case 13:
		case 10:
			xpos=0;
			ypos++;
			break;
		case '\f':
			console_reset();
			break;
		case '\b':
			xpos--;
			break;
		default:
			console_writechar(xpos,ypos,c);
			xpos++;
			break;
	}

	if(xpos<0) {
		xpos=xsize-1;
		ypos--;
	}

	if(xpos>=xsize) {
		xpos=0;
		ypos++;
	}

	if(ypos>=ysize) {
		console_reset();
	}

	console_writechar(xpos,ypos,'_');
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
	char *cbuffer = (char*)buffer;
	while(length>0) {
		console_putchar(*cbuffer);
		cbuffer++;
		length--;
	}
	return 1;
}

void console_init( struct graphics *g )
{
	gx = g;
       	console_reset();
	console_putstring("\nconsole: initialized\n");
}
