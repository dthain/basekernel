/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */


#include "kerneltypes.h"
#include "device.h"
#include "kmalloc.h"
#include "string.h"
#include "ata.h"
#include "cdromfs.h"
#include "keyboard.h"

#define ATA_DEVICE_COUNT   4
#define ATAPI_DEVICE_COUNT 4

static struct device ata_devices[ATA_DEVICE_COUNT] = {0};
static struct device atapi_devices[ATAPI_DEVICE_COUNT] = {0};

int ata_device_read( struct device *d, void *buffer, int nblocks, int offset )
{
    return ata_read(d->unit, buffer, nblocks, offset);
}

int ata_device_write( struct device *d, void *buffer, int nblocks, int offset )
{
    return ata_write(d->unit, buffer, nblocks, offset);
}

int atapi_device_read( struct device *d, void *buffer, int nblocks, int offset )
{
    return atapi_read(d->unit, buffer, nblocks, offset);
}

void device_init()
{
    int i;
    for (i = 0; i < ATAPI_DEVICE_COUNT; i++)  {
        atapi_devices[i].read = atapi_device_read;
        atapi_devices[i].unit = i;
        atapi_devices[i].block_size = CDROM_BLOCK_SIZE;
    }
    for (i = 0; i < ATA_DEVICE_COUNT; i++)  {
        ata_devices[i].read = ata_device_read;
        ata_devices[i].write = ata_device_write;
        ata_devices[i].unit = i;
        ata_devices[i].block_size = CDROM_BLOCK_SIZE;
    }
}

struct device *device_open(char *name, int unit)
{
    if (!strcmp("ATA", name)) {
        if (unit >= 0 && unit < ATA_DEVICE_COUNT) {
            return &ata_devices[unit];
        } else {
            return 0;
        }
    } else if (!strcmp("ATAPI", name)) {
        if (unit >= 0 && unit < ATAPI_DEVICE_COUNT) {
            return &atapi_devices[unit];
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

int device_read(struct device *d, void *buffer, int size, int offset)
{
    if (d->read) {
        return d->read(d, buffer, size, offset);
    } else {
        return -1;
    }
}

int device_write(struct device *d, void *buffer, int size, int offset)
{
    if (d->write) {
        return d->write(d, buffer, size, offset);
    } else {
        return -1;
    }
}

struct device *device_subset(struct device *d, int dx0, int dy0, int dx1, int dy1)
{
    if (d->subset) {
        return d->subset(d, dx0, dy0, dx1, dy1);
    } else {
        return 0;
    }
}

