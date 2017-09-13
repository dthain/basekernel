/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CDROMFS_H
#define CDROMFS_H

#include "kerneltypes.h"

#define CDROM_BLOCK_SIZE 2048

struct cdrom_volume * cdrom_volume_open( int unit );
void                  cdrom_volume_close( struct cdrom_volume *v );
struct cdrom_dirent * cdrom_volume_root( struct cdrom_volume *v );
int                   cdrom_volume_block_size( struct cdrom_volume *v );

struct cdrom_dirent * cdrom_dirent_namei( struct cdrom_dirent *d, const char *path );
struct cdrom_dirent * cdrom_dirent_lookup( struct cdrom_dirent *d, const char *name );

int  cdrom_dirent_length( struct cdrom_dirent *d );
int  cdrom_dirent_read_block( struct cdrom_dirent *d, char *buffer, int nblock );
int  cdrom_dirent_read_dir( struct cdrom_dirent *d, char *buffer, int buffer_length );
void cdrom_dirent_close( struct cdrom_dirent *d );

#endif
