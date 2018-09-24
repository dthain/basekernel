/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "device.h"

struct device *keyboard_get();
char keyboard_read(int non_blocking);	// 1 for non_blocking, 0 for blocking
void keyboard_init();
int keyboard_device_read_block(struct device *d, void *dest, int size, int offset);
int keyboard_device_read_nonblock(struct device *d, void *dest, int size, int offset);

#endif
