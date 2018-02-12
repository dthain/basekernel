/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "kerneltypes.h"

struct device {
    int (*read) (struct device *d, void *buffer, int size);
    int (*write) (struct device *d, void *buffer, int size);
    struct device *(*subset) (struct device* d, void *args);

    void* data;
};

struct device *device_open();
struct device *device_subset(struct device *d, void *args);

int device_close(struct device *d);
int device_read(struct device *d, void *buffer, int size);
int device_write(struct device *d, void *buffer, int size);

#endif
