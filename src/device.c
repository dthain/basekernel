/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "kerneltypes.h"
#include "device.h"
#include "kmalloc.h"

struct device{
    int (*read)(void* buffer, int size);
    int (*write)(void* buffer, int size);
};

struct device*    device_open() {
    struct device* d = kmalloc(sizeof(*d));
    d->read = 0;
    d->write = 0;
}

int               device_close(struct device* toclose) {
    kfree(toclose);
}

int               device_read(struct device* toread, void* buffer, int size) {
    if (toread->read) {
        return toread->read(buffer, size);
    } else {
        return -1;
    }
}

int               device_write(struct device* towrite, void* buffer, int size) {
    if (towrite->read) {
        return towrite->read(buffer, size);
    } else {
        return -1;
    }
}

#endif
