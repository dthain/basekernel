/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CDROMFS_H
#define CDROMFS_H

#include "kerneltypes.h"
#include "fs.h"

#define CDROM_BLOCK_SIZE 2048

struct cdrom_volume;
struct cdrom_dirent;

struct volume * cdrom_volume_open( uint32_t unit );
void                  cdrom_volume_close( struct volume *v );
struct dirent * cdrom_volume_root( struct volume *v );

struct dirent * cdrom_dirent_namei( struct dirent *d, const char *path );
struct dirent * cdrom_dirent_lookup( struct dirent *d, const char *name );

int  cdrom_dirent_length( struct dirent *d );
int  cdrom_dirent_read_dir( struct dirent *d, char *buffer, int buffer_length );
void cdrom_dirent_close( struct dirent *d );

int cdrom_file_read(struct file *f, char *buffer, uint32_t n);
struct file *cdrom_file_open(struct dirent *d, int8_t mode);
int cdrom_file_close(struct file *f);
#endif
