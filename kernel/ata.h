/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef ATA_H
#define ATA_H

#define ATA_BLOCKSIZE 512
#define ATAPI_BLOCKSIZE 2048

#include "device.h"

struct ata_count {
	int blocks_written[4];
	int blocks_read[4];
};

void ata_init();

struct ata_count ata_stats();
void ata_reset(int unit);

int ata_probe(int unit, int *nblocks, int *blocksize, char *name);
int ata_read(int unit, void *buffer, int nblocks, int offset);
int ata_write(int unit, const void *buffer, int nblocks, int offset);

int atapi_probe(int unit, int *nblocks, int *blocksize, char *name);
int atapi_read(int unit, void *buffer, int nblocks, int offset);

#endif
