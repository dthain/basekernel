/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#ifndef KOBJECT_H
#define KOBJECT_H

#include "fs.h"
#include "device.h"
#include "graphics.h"

struct kobject {
    union {
        struct device *device;
        struct fs_file *file;
        struct graphics *graphics;
    } data;
    enum {
        KOBJECT_INVALID=0,
        KOBJECT_FILE,
        KOBJECT_DEVICE,
        KOBJECT_GRAPHICS
    } type;
    int rc;
};

struct kobject *kobject_create_file(struct fs_file *f);
struct kobject *kobject_create_device(struct device *d);
struct kobject *kobject_create_graphics(struct graphics *g);

int kobject_read(struct kobject *kobject, void *buffer, int size);
int kobject_write(struct kobject *kobject, void *buffer, int size);
int kobject_close(struct kobject *kobject);

#endif
