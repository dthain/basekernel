/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "graphics.h"
#include "kerneltypes.h"
#include "font.h"
#include "kernelcore.h"

int graphics_width()
{
	return video_xres;
}

int graphics_height()
{
	return video_yres;
}

static inline void plot_pixel( int x, int y, struct graphics_color c )
{
	uint8_t *v = video_buffer + video_xbytes*y+x*3;
	v[2] = c.r;
	v[1] = c.g;
	v[0] = c.b;
}

void graphics_rect( int x, int y, int w, int h, struct graphics_color c )
{
	int i, j;

	for(j=0;j<h;j++) {
		for(i=0;i<w;i++) {
			plot_pixel(x+i,y+j,c);
		}
	}
}

void graphics_clear( struct graphics_color c )
{
	graphics_rect(0,0,video_xres,video_yres,c);
}

void graphics_bitmap( int x, int y, int width, int height, uint8_t *data, struct graphics_color fgcolor, struct graphics_color bgcolor )
{
	int i,j,b;
	int value;

	b=0;

	for(j=0;j<height;j++) {
		for(i=0;i<width;i++) {
			value = ((*data)<<b)&0x80;
			if(value) {
				plot_pixel(x+i,y+j,fgcolor);
			} else {
				plot_pixel(x+i,y+j,bgcolor);
			}
			b++;
			if(b==8) {
				data++;
				b=0;
			}
		}
	}
}

void graphics_char( int x, int y, char ch, struct graphics_color fgcolor, struct graphics_color bgcolor )
{
	int u = ((int)ch)*FONT_WIDTH*FONT_HEIGHT/8;
	return graphics_bitmap(x,y,FONT_WIDTH,FONT_HEIGHT,&fontdata[u],fgcolor,bgcolor);
}
