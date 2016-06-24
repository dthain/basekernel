/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CDROMFS_H
#define CDROMFS_H

#include "kerneltypes.h"

struct cdrom_volume * cdrom_volume_open( int unit );
void                  cdrom_volume_close( struct cdrom_volume *v );
struct cdrom_dirent * cdrom_volume_root( struct cdrom_volume *v );

int  cdrom_dirent_read( struct cdrom_dirent *d, char *data, int length );
int  cdrom_dirent_readdir( struct cdrom_dirent *d, char *buffer, int buffer_length );

void cdrom_dirent_close( struct cdrom_dirent *d );

#endif
