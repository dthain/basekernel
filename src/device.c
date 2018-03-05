/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */


#include "kerneltypes.h"
#include "device.h"
#include "kmalloc.h"

struct device *device_create()
{
    struct device *d = kmalloc(sizeof(*d));
    d->read = 0;
    d->write = 0;
    d->unit = 0;
    d->subset = 0;
    d->block_size = 0;
    d->sx0 = 0;
    d->sy0 = 0;
    d->sx1 = 0;
    d->sy1 = 0;
    return d;
}

int device_close(struct device *d)
{
    kfree(d);
    return 1;
}

int device_read(struct device *d, void *buffer, int size, int offset)
{
    if (d->read) {
        return d->read(d, buffer, size, offset);
    } else {
        return -1;
    }
}

int device_write(struct device *d, void *buffer, int size, int offset)
{
    if (d->write) {
        return d->write(d, buffer, size, offset);
    } else {
        return -1;
    }
}

struct device *device_subset(struct device *d, int dx0, int dy0, int dx1, int dy1)
{
    if (d->subset) {
        return d->subset(d, dx0, dy0, dx1, dy1);
    } else {
        return 0;
    }
}

