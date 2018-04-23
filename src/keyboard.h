/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "device.h"

struct device* keyboard_get();
char keyboard_read();
void keyboard_init();
int keyboard_device_read(struct device* d, void* dest, int size, int offset);

#endif
