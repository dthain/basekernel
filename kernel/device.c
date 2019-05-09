/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/
#include "device.h"
#include "string.h"
#include "page.h"
#include "kmalloc.h"

#include "kernel/stats.h"
#include "kernel/types.h"
#include "kernel/error.h"

static struct device_driver *driver_list = 0;

struct device {
	struct device_driver *driver;
	int refcount;
	int unit;
	int block_size;
	int nblocks;
	int multiplier;
};

void device_driver_register( struct device_driver *d )
{
	d->next = driver_list;
	d->stats = (struct device_driver_stats){0};
	driver_list = d;
}

static struct device *device_create( struct device_driver *dd, int unit, int nblocks, int block_size )
{
	struct device *d = kmalloc(sizeof(*d));
	d->refcount = 1;
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

	struct device_driver *dd = device_driver_lookup(name);
	if (dd && dd->probe(unit,&nblocks,&block_size,info)) {
		return device_create(dd,unit,nblocks,block_size);
	}

	return 0;
}

struct device *device_addref( struct device *d )
{
	d->refcount++;
	return d;
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
	d->refcount--;
	if(d->refcount<1) kfree(d);
}

int device_read(struct device *d, void *data, int size, int offset)
{
	int status;
	if(d->driver->read) {
		status = d->driver->read(d->unit,data,size*d->multiplier,offset*d->multiplier);
		if (status) {
			d->driver->stats.blocks_read += size*d->multiplier; // number of blocks
		}
		return status;
	} else {
		return KERROR_NOT_IMPLEMENTED;
	}
}

int device_read_nonblock(struct device *d, void *data, int size, int offset)
{
	int status;
	if(d->driver->read_nonblock) {
		status = d->driver->read_nonblock(d->unit,data,size*d->multiplier,offset*d->multiplier);
		if (status) {
			d->driver->stats.blocks_read += size*d->multiplier; // number of blocks
		}
		return status;
	} else {
		return KERROR_NOT_IMPLEMENTED;
	}
}

int device_write(struct device *d, const void *data, int size, int offset)
{
	int status;
	if(d->driver->write) {
		status = d->driver->write(d->unit,data,size*d->multiplier,offset*d->multiplier);
		if (!status) {
			d->driver->stats.blocks_written += size*d->multiplier;
		}
		return status;
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

struct device_driver * device_driver_lookup(const char *name)
{
	struct device_driver *dd = driver_list;
	for(dd=driver_list; dd; dd=dd->next) {
		if(!strcmp(dd->name, name)) {
			break;
		}
	}
	return dd;
}

void device_driver_get_stats(const char * name, struct device_driver_stats * s)
{
	/* Get the device driver */
	struct device_driver *dd = device_driver_lookup(name);

	/* Copy stats into struct */
	if (dd) {
		memcpy(s, &(dd->stats), sizeof(*s));
	}
}
