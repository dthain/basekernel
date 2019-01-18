/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "bitmap.h"
#include "kernelcore.h"
#include "kmalloc.h"

static struct bitmap root_bitmap;

struct bitmap *bitmap_create_root()
{
	root_bitmap.width = video_xres;
	root_bitmap.height = video_yres;
	root_bitmap.format = BITMAP_FORMAT_RGB;
	root_bitmap.data = video_buffer;
	return &root_bitmap;
}

struct bitmap *bitmap_create(int width, int height, int format)
{
	struct bitmap *b = kmalloc(sizeof(*b));
	if(!b)
		return 0;

	b->data = kmalloc(width * height * 3);
	if(!b->data) {
		kfree(b);
		return 0;
	}

	b->width = width;
	b->height = height;
	b->format = format;

	return b;
}

void bitmap_delete(struct bitmap *b)
{
	kfree(b->data);
	kfree(b);
}
