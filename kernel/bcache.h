#ifndef BCACHE_H
#define BCACHE_H

#include "device.h"

void bcache_init();

int  bcache_read( struct device *d, char *data, int blocks, int offset );
int  bcache_write( struct device *d, const char *data, int blocks, int offset );

int  bcache_read_block( struct device *d, char *data, int block );
int  bcache_write_block( struct device *d, const char *data, int block );

void bcache_flush_block( struct device *d, int block );
void bcache_flush_device( struct device *d  );

#endif
