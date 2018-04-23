/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "kerneltypes.h"
#include "graphics.h"
#include "device.h"
#include "buffer.h"

struct device {
    int (*read) (struct device *d, void *buffer, int size, int offset);
    int (*write) (struct device *d, void *buffer, int size, int offset);
    struct device *(*subset) (struct device* d, int dx0, int dy0, int dx1, int dy1);

    int unit;
    int block_size;
    int sx0;
    int sy0;
    int sx1;
    int sy1;
    int alloced;
    struct buffer *buffer;
};

void device_init();
struct device *device_open(char *type, int unit);
struct device *device_subset(struct device *d, int dx0, int dy0, int dx1, int dy1);

int device_read(struct device *d, void *buffer, int size, int offset);
int device_write(struct device *d, void *buffer, int size, int offset);

#endif
