/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef BITMAP_H
#define BITMAP_H

#include "kernel/types.h"

struct bitmap *bitmap_create_root();

struct bitmap *bitmap_create(int width, int height, int format);
void bitmap_delete(struct bitmap *b);

struct bitmap {
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint8_t *data;
};

#define BITMAP_FORMAT_RGB      0
#define BITMAP_FORMAT_RGBA     1

#endif
