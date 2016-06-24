/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "kerneltypes.h"
#include "bitmap.h"

struct graphics_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct clip {
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
};

struct graphics {
	struct bitmap *bitmap;
	struct graphics_color fgcolor;
	struct graphics_color bgcolor;
	struct clip clip;
};

struct graphics * graphics_create_root();

struct graphics * graphics_create( struct graphics *parent );
void  graphics_delete( struct graphics *g );
int32_t graphics_width( struct graphics *g );
int32_t graphics_height( struct graphics *g );
void  graphics_fgcolor( struct graphics *g, struct graphics_color c );
void  graphics_bgcolor( struct graphics *g, struct graphics_color c );
void  graphics_clip( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h );

void graphics_scrollup( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h, int32_t dy );
void graphics_rect( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h );
void graphics_clear( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h );
void graphics_line( struct graphics *g, int32_t x, int32_t y, int32_t w, int32_t h );
void graphics_char( struct graphics *g, int32_t x, int32_t y, char c );

#endif

