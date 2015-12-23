/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "kerneltypes.h"

struct graphics_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

int graphics_width();
int graphics_height();

void graphics_rect( int x, int y, int w, int h, struct graphics_color c );
void graphics_clear( struct graphics_color c );
void graphics_char( int x, int y, char ch, struct graphics_color fgcolor, struct graphics_color bgcolor );

#endif

