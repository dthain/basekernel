/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "kernel/types.h"
#include "kernel/gfxstream.h"

struct graphics_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

extern struct graphics graphics_root;

struct graphics *graphics_create_root();
struct graphics *graphics_create(struct graphics *parent );
struct graphics *graphics_addref(struct graphics *g );
void graphics_delete(struct graphics *g);

uint32_t graphics_width(struct graphics *g);
uint32_t graphics_height(struct graphics *g);
void graphics_fgcolor(struct graphics *g, struct graphics_color c);
void graphics_bgcolor(struct graphics *g, struct graphics_color c);
int  graphics_clip(struct graphics *g, int x, int y, int w, int h);

void graphics_scrollup(struct graphics *g, int x, int y, int w, int h, int dy);
void graphics_rect(struct graphics *g, int x, int y, int w, int h);
void graphics_clear(struct graphics *g, int x, int y, int w, int h);
void graphics_line(struct graphics *g, int x, int y, int w, int h);
void graphics_char(struct graphics *g, int x, int y, unsigned char c);

int graphics_write(struct graphics *g, struct graphics_command *command);

#endif
