/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */


#include "kerneltypes.h"
#include "device.h"
#include "kmalloc.h"

struct device *device_open()
{
    struct device *d = kmalloc(sizeof(*d));
    d->read = 0;
    d->write = 0;
    d->subset = 0;
    d->data = 0;
    return d;
}

int device_close(struct device *d)
{
    kfree(d);
    return 1;
}

int device_read(struct device *d, void *buffer, int size)
{
    if (d->read) {
	return d->read(d, buffer, size);
    } else {
	return -1;
    }
}

int device_write(struct device *d, void *buffer, int size)
{
    if (d->write) {
	return d->write(d, buffer, size);
    } else {
	return -1;
    }
}

struct device *device_subset(struct device *d, void *args)
{
    if (d->subset) {
	return d->subset(d, args);
    } else {
	return 0;
    }
}

