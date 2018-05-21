/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef ELF_H
#define ELF_H

#include "process.h"

struct process* elf_load(const char* path, int pid);

#endif
