/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#include "device.h"
#include "string.h"
#include "ata.h"
#include "cdromfs.h"
#include "keyboard.h"
#include "memory.h"
#include "serial.h"
#include "kmalloc.h"

#include "kernel/types.h"
#include "kernel/error.h"

struct device_driver {
	const char *name;
	int (*probe) ( int unit, int *nblocks, int *blocksize, char *info );
	int (*read) ( int unit, void *buffer, int nblocks, int block_offset);
	int (*read_nonblock) ( int unit, void *buffer, int nblocks, int block_offset);
	int (*write) ( int unit, const void *buffer, int nblocks, int block_offset);
};

struct device {
	struct device_driver *driver;
	int unit;
	int block_size;
	int nblocks;
	int multiplier;
};

struct device_driver drivers[] = {
{
	"ata",
	ata_probe,
	ata_read,
	ata_read,
	ata_write
},
{
	"atapi",
	ata_probe,
	atapi_read,
	atapi_read,
	0,
},
{
	"serial",
	serial_device_probe,
	serial_device_read,
	serial_device_read,
	serial_device_write
},
{
	"keyboard",
	keyboard_device_probe,
	keyboard_device_read,
	keyboard_device_read_nonblock,
	0,
},
	{0,0,0,0}
};


void device_init()
{
}

static struct device *device_create( struct device_driver *dd, int unit, int nblocks, int block_size )
{
	struct device *d = kmalloc(sizeof(*d));
	d->driver = dd;
	d->unit = unit;
	d->block_size = block_size;
	d->nblocks = nblocks;
	d->multiplier = 1;
	return d;
}


struct device *device_open( const char *name, int unit )
{
	int i;
	int nblocks, block_size;
	char info[64];

	for(i=0;drivers[i].name;i++) {
		struct device_driver *dd = &drivers[i];
		if(!strcmp(dd->name,name)) {
			if(dd->probe(unit,&nblocks,&block_size,info)) {
				return device_create(dd,unit,nblocks,block_size);
			} else {
				return 0;
			}
		}
	}

	return 0;
}

int device_set_multiplier( struct device *d, int multiplier )
{
	if(multiplier<1 || multiplier*d->block_size>PAGE_SIZE ) {
		return KERROR_INVALID_REQUEST;
	}

	d->multiplier = multiplier;

	return 0;
}

void device_close( struct device *d )
{
	kfree(d);
}

int device_read(struct device *d, void *data, int size, int offset)
{
	if(d->driver->read) {
		return d->driver->read(d->unit,data,size*d->multiplier,offset/d->multiplier);
	} else {
		return KERROR_NOT_SUPPORTED;
	}
}

int device_read_nonblock(struct device *d, void *data, int size, int offset)
{
	if(d->driver->read_nonblock) {
		return d->driver->read_nonblock(d->unit,data,size*d->multiplier,offset/d->multiplier);
	} else {
		return KERROR_NOT_SUPPORTED;
	}
}

int device_write(struct device *d, const void *data, int size, int offset)
{
	if(d->driver->write) {
		return d->driver->write(d->unit,data,size*d->multiplier,offset/d->multiplier);
	} else {
		return KERROR_NOT_SUPPORTED;
	}
}

int device_block_size( struct device *d )
{
	return d->block_size*d->multiplier;
}

int device_nblocks( struct device *d )
{
	return d->nblocks/d->multiplier;
}

int device_unit( struct device *d )
{
	return d->unit;
}

const char * device_name( struct device *d )
{
	return d->driver->name;
}
