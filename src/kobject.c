/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#include "kobject.h"
#include "kmalloc.h"
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

struct kobject *kobject_create_file(struct fs_file *f) {
    struct kobject *k = kmalloc(sizeof(*k));
    k->type = FILE;
    k->rc = 0;
    k->data.file = f;
}

struct kobject *kobject_create_device(struct device *d){ 
    struct kobject *k = kmalloc(sizeof(*k));
    k->type = DEVICE;
    k->rc = 0;
    k->data.device = d;
}

int kobject_read(struct kobject *kobject, void *buffer, int size) {
    switch (kobject->type) {
        case FILE:
            return fs_file_read(kobject->data.file, (char*)buffer, (uint32_t)size);
        case DEVICE:
            return device_read(kobject->data.device, buffer, size/kobject->data.device->block_size, 0);
    }
}

int kobject_write(struct kobject *kobject, void *buffer, int size) {
    switch (kobject->type) {
        case FILE:
            return fs_file_write(kobject->data.file, (char*)buffer, (uint32_t)size);
        case DEVICE:
            return device_write(kobject->data.device, buffer, size/kobject->data.device->block_size, 0);
    }
}

int kobject_close(struct kobject *kobject) {
    if (--kobject->rc) {
        switch (kobject->type) {
            case FILE:
                return fs_file_close(kobject->data.file);
            case DEVICE:
                return 0;
        }
    }
}

