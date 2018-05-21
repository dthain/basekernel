/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "kerneltypes.h"

void	memory_init();
void *	memory_alloc_page( bool zeroit );
void	memory_free_page( void *addr );

#endif
