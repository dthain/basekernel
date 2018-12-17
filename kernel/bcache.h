#ifndef BCACHE_H
#define BCACHE_H

#include "device.h"

int  bcache_read( struct device *d, char *data, int blocks, int offset );
int  bcache_write( struct device *d, const char *data, int blocks, int offset );

int  bcache_read_block( struct device *d, char *data, int block );
int  bcache_write_block( struct device *d, const char *data, int block );

void bcache_flush_block( struct device *d, int block );
void bcache_flush_device( struct device *d  );

struct bcache_stats {
	unsigned read_hits;
	unsigned read_misses;
	unsigned write_hits;
	unsigned write_misses;
	unsigned writebacks;
};

void bcache_get_stats( struct bcache_stats *s );

#endif
