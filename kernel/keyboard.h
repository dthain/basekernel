/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "device.h"

void keyboard_init();
char keyboard_read(int non_blocking);	// 1 for non_blocking, 0 for blocking

int keyboard_device_probe( int unit, int *nblocks, int *blocksize, char *info );
int keyboard_device_read( int unit, void *data, int size, int offset);
int keyboard_device_read_nonblock( int unit, void *data, int size, int offset);

#endif
