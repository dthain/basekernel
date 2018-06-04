/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CDROMFS_H
#define CDROMFS_H

#include "kernel/types.h"
#include "fs.h"

#define CDROM_BLOCK_SIZE 2048

int cdrom_init();
#endif
