/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef DEVICE_H
#define DEVICE_H

#include "kerneltypes.h"

struct device{
    int (*read)(void* buffer, int size);
    int (*write)(void* buffer, int size);
};

struct device*    device_open();
int               device_close(struct device* toclose);
int               device_read(struct device* toread, void* buffer, int size);
int               device_write(struct device* towrite, void* buffer, int size);

#endif
