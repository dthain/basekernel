/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef KMALLOC_H
#define KMALLOC_H

#define KMALLOC_STATE_FREE 0xa1a1a1a1
#define KMALLOC_STATE_USED 0xbfbfbfbf

void * kmalloc( int length );
void   kfree( void *ptr );

void kmalloc_init( char *start, int length );
void kmalloc_debug();

struct kmalloc_chunk {
	int state;
	int length;
	struct kmalloc_chunk *next;
	struct kmalloc_chunk *prev;
};

#endif
