/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "kerneltypes.h"

#define SYSCALL_exit     1
#define SYSCALL_testcall 2
#define SYSCALL_yield    3

uint32_t syscall( uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e );

static inline int32_t exit( uint32_t status )
	{ return syscall( SYSCALL_exit, status, 0, 0, 0, 0 ); }

static inline int32_t testcall( int x ) 
	{ return syscall( SYSCALL_testcall, x, 0, 0, 0, 0 ); }

static inline int32_t yield()
	{ return syscall( SYSCALL_yield, 0, 0, 0, 0, 0 ); }

#endif
