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
#include "kernel/types.h"
#include "kernel/error.h"

#define ATA_DEVICE_COUNT   4
#define ATAPI_DEVICE_COUNT 4

static struct device ata_devices[ATA_DEVICE_COUNT] = { {0} };
static struct device atapi_devices[ATAPI_DEVICE_COUNT] = { {0} };

int ata_device_read(struct device *d, void *data, int nblocks, int offset)
{
	return ata_read(d->unit, data, nblocks, offset);
}

int ata_device_write(struct device *d, const void *data, int nblocks, int offset)
{
	return ata_write(d->unit, data, nblocks, offset);
}

int atapi_device_read(struct device *d, void *data, int nblocks, int offset)
{
	return atapi_read(d->unit, data, nblocks, offset);
}

void device_init()
{
	int i;
	for(i = 0; i < ATAPI_DEVICE_COUNT; i++) {
		atapi_devices[i].read = atapi_device_read;
		atapi_devices[i].write = 0;
		atapi_devices[i].unit = i;
		atapi_devices[i].block_size = CDROM_BLOCK_SIZE;
	}
	for(i = 0; i < ATA_DEVICE_COUNT; i++) {
		ata_devices[i].read = ata_device_read;
		ata_devices[i].write = ata_device_write;
		ata_devices[i].unit = i;
		ata_devices[i].block_size = ATA_BLOCKSIZE;
	}
}

struct device *device_open(char *name, int unit)
{
	if(!strcmp("ATA", name)) {
		if(unit >= 0 && unit < ATA_DEVICE_COUNT) {
			return &ata_devices[unit];
		}
		return 0;
	} else if(!strcmp("ATAPI", name)) {
		if(unit >= 0 && unit < ATAPI_DEVICE_COUNT) {
			return &atapi_devices[unit];
		}
		return 0;
	}
	return 0;
}

int device_read(struct device *d, void *data, int size, int offset)
{
	if(d->read) {
		return d->read(d,data,size,offset);
	} else {
		return KERROR_NOT_SUPPORTED;
	}
}

int device_read_nonblock(struct device *d, void *data, int size, int offset)
{
	if(d->read_nonblock) {
		return d->read_nonblock(d,data,size,offset);
	} else {
		return KERROR_NOT_SUPPORTED;
	}
}

int device_write(struct device *d, const void *data, int size, int offset)
{
	if(d->write) {
		return d->write(d,data,size,offset);
	} else {
		return KERROR_NOT_SUPPORTED;
	}
}

int device_block_size( struct device *d )
{
	return d->block_size;
}
