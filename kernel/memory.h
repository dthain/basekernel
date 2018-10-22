/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "kernel/types.h"

void memory_init();
void *memory_alloc_page(bool zeroit);
void memory_free_page(void *addr);

uint32_t memory_pages_free();
uint32_t memory_pages_total();

#endif
