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
    return d;
}

int device_close(struct device *toclose)
{
    kfree(toclose);
    return 1;
}

int device_read(struct device *toread, void *buffer, int size)
{
    if (toread->read) {
	return toread->read(toread, buffer, size);
    } else {
	return -1;
    }
}

int device_write(struct device *towrite, void *buffer, int size)
{
    if (towrite->read) {
	return towrite->read(towrite, buffer, size);
    } else {
	return -1;
    }
}

int device_ignore(struct device *towrite, void *buffer, int size)
{
    return -1;
}
