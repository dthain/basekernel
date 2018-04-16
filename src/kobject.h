/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#ifndef KOBJECT_H
#define KOBJECT_H

#include "fs.h"
#include "device.h"

struct kobject {
    union {
        struct device *device;
        struct fs_file *file;
    } data;
    enum {
        FILE,
        DEVICE
    } type;
    int rc;
};

struct kobject *kobject_create_file(struct fs_file *f);
struct kobject *kobject_create_device(struct device *d);

int kobject_read(struct kobject *kobject, void *buffer, int size);
int kobject_write(struct kobject *kobject, void *buffer, int size);
int kobject_close(struct kobject *kobject);

#endif
