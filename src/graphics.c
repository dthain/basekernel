/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "graphics.h"
#include "kerneltypes.h"
#include "ioports.h"
#include "font.h"
#include "string.h"
#include "kmalloc.h"
#include "bitmap.h"

#ifndef MIN
#define MIN(x,y) (((x)<(y)) ? (x) : (y) )
#endif

#define FACTOR 256

static struct graphics_color color_black = {0,0,0,0};
static struct graphics_color color_white = {255,255,255,0};

static struct graphics graphics_root;

struct graphics * graphics_create_root()
{
	struct graphics *g = &graphics_root;
	g->bitmap = bitmap_create_root();
	g->fgcolor = color_white;
	g->fgcolor = color_black;
	g->clip.x = 0;
	g->clip.y = 0;
	g->clip.w = g->bitmap->width;
	g->clip.h = g->bitmap->height;
	return g;
}

struct graphics * graphics_create( struct graphics *parent )
{
	struct graphics *g = kmalloc(sizeof(*g));
	if(!g) return 0;

	*g = *parent;
	return g;
}

int32_t graphics_width( struct graphics *g )
{
	return g->clip.w;
}

int32_t graphics_height( struct graphics *g )
{
	return g->clip.h;
}

void  graphics_fgcolor( struct graphics *g, struct graphics_color c )
{
	g->fgcolor = c;
}

void  graphics_bgcolor( struct graphics *g, struct graphics_color c )
{
	g->bgcolor = c;
}

void  graphics_clip( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	if(x<0) x=0;
	if(y<0) y=0;
	if((x+w)>g->bitmap->width)  w = g->bitmap->width-x;
	if((y+h)>g->bitmap->height) h = g->bitmap->height-y;
	g->clip.x = x;
	g->clip.y = y;
	g->clip.w = w;
	g->clip.h = h;
}

static inline void plot_pixel( struct bitmap *b, int32_t x, int32_t y, struct graphics_color c )
{
	uint8_t *v = b->data + (b->width*y+x)*3;
	if(c.a==0) {
		v[2] = c.r;
		v[1] = c.g;
		v[0] = c.b;
	} else {
		uint16_t a = c.a;
		uint16_t b = 256-a;
		v[0] = (c.r*b+v[0]*a)>>8;
		v[1] = (c.g*b+v[1]*a)>>8;
		v[2] = (c.b*b+v[2]*a)>>8;
	}
}

void graphics_rect( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	int i, j;

	w = MIN(g->clip.w-x,w);
	h = MIN(g->clip.h-y,h);
	x += g->clip.x;
	y += g->clip.y;

	for(j=0;j<h;j++) {
		for(i=0;i<w;i++) {
			plot_pixel(g->bitmap,x+i,y+j,g->fgcolor);
		}
	}
}

void graphics_clear( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	int i, j;

	w = MIN(g->clip.w-x,w);
	h = MIN(g->clip.h-y,h);
	x += g->clip.x;
	y += g->clip.y;

	for(j=0;j<h;j++) {
		for(i=0;i<w;i++) {
			plot_pixel(g->bitmap,x+i,y+j,g->bgcolor);
		}
	}
}

static inline void graphics_line_vert( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	do {
		plot_pixel(g->bitmap,x,y,g->fgcolor);
		y++;
		h--;
	} while(h>0);
}

static inline void graphics_line_q1( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	int32_t slope = FACTOR*w/h;
	int32_t counter = 0;

	do {
		plot_pixel(g->bitmap,x,y,g->fgcolor);
		y++;
		h--;
		counter += slope;
		if(counter>FACTOR) {
			counter = counter-FACTOR;
			x++;
			w--;
		}
	} while(h>0);
}

static inline void graphics_line_q2( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	int32_t slope = FACTOR*h/w;
	int32_t counter = 0;

	do {
		plot_pixel(g->bitmap,x,y,g->fgcolor);
		x++;
		w--;
		counter += slope;
		if(counter>FACTOR) {
			counter = counter-FACTOR;
			y++;
			h--;
		}
	} while(w>0);
}

static inline void graphics_line_q3( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	int32_t slope = -FACTOR*h/w;
	int32_t counter = 0;

	do {
		plot_pixel(g->bitmap,x,y,g->fgcolor);
		x++;
		w--;
		counter += slope;
		if(counter>FACTOR) {
			counter = counter-FACTOR;
			y--;
			h--;
		}
	} while(w>0);
}

static inline void graphics_line_q4( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	int32_t slope = -FACTOR*w/h;
	int32_t counter = 0;

	do {
		plot_pixel(g->bitmap,x,y,g->fgcolor);
		y--;
		h--;
		counter += slope;
		if(counter>FACTOR) {
			counter = counter-FACTOR;
			x++;
			w--;
		}
	} while(h>0);
}

static inline void graphics_line_hozo( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	do {
		plot_pixel(g->bitmap,x,y,g->fgcolor);
		x++;
		w--;
	} while(w>0);
}

void graphics_line( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h )
{
	if(w<0) {
		x = x+w;
		y = y+h;
		w = -w;
		h = -h;
	}

	x += g->clip.x;
	y += g->clip.y;

	if(h>0) {
		if(w==0) {
			graphics_line_vert(g,x,y,w,h);
		} else if(h>w) {
			graphics_line_q1(g,x,y,w,h);
		} else {
			graphics_line_q2(g,x,y,w,h);
		}
	} else {
		if(h==0) {
			graphics_line_hozo(g,x,y,w,h);
		} else if(h>-w) {
			graphics_line_q3(g,x,y,w,h);
		} else {
			graphics_line_q4(g,x,y,w,h);
		}
	}
}

void graphics_bitmap( struct graphics *g, int32_t x, int32_t y, int32_t width, int32_t height, uint8_t *data )
{
	int i,j,b;
	int value;

	width = MIN(g->clip.w-x,width);
	height = MIN(g->clip.h-y,height);
	x += g->clip.x;
	y += g->clip.y;

	b=0;

	for(j=0;j<height;j++) {
		for(i=0;i<width;i++) {
			value = ((*data)<<b)&0x80;
			if(value) {
				plot_pixel(g->bitmap,x+i,y+j,g->fgcolor);
			} else {
				plot_pixel(g->bitmap,x+i,y+j,g->bgcolor);
			}
			b++;
			if(b==8) {
				data++;
				b=0;
			}
		}
	}
}

void graphics_char( struct graphics *g, int32_t x, int32_t y, char c )
{
	uint32_t u = ((uint32_t)c)*FONT_WIDTH*FONT_HEIGHT/8;
	return graphics_bitmap(g,x,y,FONT_WIDTH,FONT_HEIGHT,&fontdata[u]);
}

void graphics_scrollup( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h, int32_t dy )
{
	int j;

	w = MIN(g->clip.w-x,w);
	h = MIN(g->clip.h-y,h);
	x += g->clip.x;
	y += g->clip.y;

	if(dy>h) dy=h;

	for(j=0;j<(h-dy);j++) {
		memcpy(
			&g->bitmap->data[((y+j)*g->bitmap->width+x)*3],
			&g->bitmap->data[((y+j+dy)*g->bitmap->width+x)*3],
			w*3
		);
	}

	graphics_clear(g,x,y+h-dy,w,dy);
}


