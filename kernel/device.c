/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#include "device.h"
#include "string.h"
#include "memory.h"
#include "kmalloc.h"

#include "kernel/types.h"
#include "kernel/error.h"

static struct device_driver *driver_list = 0;

struct device {
	struct device_driver *driver;
	int unit;
	int block_size;
	int nblocks;
	int multiplier;
};

void device_driver_register( struct device_driver *d )
{
	d->next = driver_list;
	driver_list = d;
}

static struct device *device_create( struct device_driver *dd, int unit, int nblocks, int block_size )
{
	struct device *d = kmalloc(sizeof(*d));
	d->driver = dd;
	d->unit = unit;
	d->block_size = block_size;
	d->nblocks = nblocks;

/*
If the device driver specifies a non-zero default multiplier,
then save it in this device instance.  It gives the effect of
multiplying the block size, typically from the 512 ATA sector
size, up to the usual 4KB page size.  See the effect below
in read/write.
*/

	if(dd->multiplier>0) {
		d->multiplier = dd->multiplier;
	} else {
		d->multiplier = 1;
	}
	return d;
}

struct device *device_open( const char *name, int unit )
{
	int nblocks, block_size;
	char info[64];

	struct device_driver *dd = driver_list;

	for(dd=driver_list;dd;dd=dd->next) {
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
		return d->driver->read(d->unit,data,size*d->multiplier,offset*d->multiplier);
	} else {
		return KERROR_NOT_IMPLEMENTED;
	}
}

int device_read_nonblock(struct device *d, void *data, int size, int offset)
{
	if(d->driver->read_nonblock) {
		return d->driver->read_nonblock(d->unit,data,size*d->multiplier,offset*d->multiplier);
	} else {
		return KERROR_NOT_IMPLEMENTED;
	}
}

int device_write(struct device *d, const void *data, int size, int offset)
{
	if(d->driver->write) {
		return d->driver->write(d->unit,data,size*d->multiplier,offset*d->multiplier);
	} else {
		return KERROR_NOT_IMPLEMENTED;
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
