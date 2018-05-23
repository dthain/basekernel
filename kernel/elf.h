/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef ELF_H
#define ELF_H

#include "process.h"

int elf_load( struct process *p, const char *path );

#endif
