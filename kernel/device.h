/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "kernel/types.h"

struct device_driver {
	const char *name;
	int (*probe) ( int unit, int *nblocks, int *blocksize, char *info );
	int (*read) ( int unit, void *buffer, int nblocks, int block_offset);
	int (*read_nonblock) ( int unit, void *buffer, int nblocks, int block_offset);
	int (*write) ( int unit, const void *buffer, int nblocks, int block_offset);
	int multiplier;
	struct device_driver *next;
};

void device_driver_register( struct device_driver *d );

struct device *device_open(const char *name, int unit);
void device_close( struct device *d );

int device_read(struct device *d, void *buffer, int size, int offset);
int device_read_nonblock(struct device *d, void *buffer, int size, int offset);
int device_write(struct device *d, const void *buffer, int size, int offset);
int device_block_size( struct device *d );
int device_nblocks( struct device *d );
int device_unit( struct device *d );
const char * device_name( struct device *d );

#endif
