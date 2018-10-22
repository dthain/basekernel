/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "kernel/types.h"
#include "graphics.h"
#include "device.h"
#include "buffer.h"

struct device {
	int (*read) (struct device * d, void *buffer, int size, int offset);
	int (*read_nonblock) (struct device * d, void *buffer, int size, int offset);
	int (*write) (struct device * d, const void *buffer, int size, int offset);
	int unit;
	int block_size;
	int alloced;
	struct buffer *buffer;
};

void device_init();
struct device *device_open(char *type, int unit);
int device_read(struct device *d, void *buffer, int size, int offset);
int device_read_nonblock(struct device *d, void *buffer, int size, int offset);
int device_write(struct device *d, const void *buffer, int size, int offset);

#endif
