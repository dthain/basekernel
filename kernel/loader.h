/*
Copyright (C) 2015-2025 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef LOADER_H
#define LOADER_H

#include "process.h"
#include "fs.h"

/*
loader_load_process reads the given file object,
and if it contains a valid ELF executable, loads
the segments of the executable into the process
address space, returning the executable entry point
in the argument "entry".
*/

int loader_load_process(struct process *p, struct fs_dirent *d, addr_t * entry);

#endif
