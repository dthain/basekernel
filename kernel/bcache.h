/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef BCACHE_H
#define BCACHE_H

#include "device.h"
#include "kernel/stats.h"

int  bcache_read( struct device *d, char *data, int blocks, int offset );
int  bcache_write( struct device *d, const char *data, int blocks, int offset );

int  bcache_read_block( struct device *d, char *data, int block );
int  bcache_write_block( struct device *d, const char *data, int block );

void bcache_flush_block( struct device *d, int block );
void bcache_flush_device( struct device *d  );
void bcache_flush_all();

void bcache_get_stats( struct bcache_stats *s );

#endif
