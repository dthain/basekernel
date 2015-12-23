/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file COPYING for details.
*/

#ifndef ATA_H
#define ATA_H

void ata_init();

void ata_reset( int unit );
int ata_probe( int unit, int *nblocks, int *blocksize, char *name );

int ata_read( int unit, void *buffer, int nblocks, int offset );
int ata_write( int unit, void *buffer, int nblocks, int offset );
int atapi_read( int unit, void *buffer, int nblocks, int offset );

#endif
